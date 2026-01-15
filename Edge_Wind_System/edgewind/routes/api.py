"""
API路由蓝图
处理所有RESTful API请求
"""
from flask import Blueprint, request, jsonify
from flask_login import login_required
from datetime import datetime, timedelta
from edgewind.models import db, Device, DataPoint, WorkOrder, SystemConfig, FaultSnapshot
from edgewind.knowledge_graph import FAULT_KNOWLEDGE_GRAPH, FAULT_CODE_MAP, generate_ai_report, get_fault_knowledge_graph
from edgewind.utils import (
    save_to_buffer, get_latest_normal_data, get_latest_fault_data,
    node_fault_states, node_snapshot_saved, save_fault_snapshot, create_work_order_from_fault
)
import time
import json
import logging
from urllib.parse import unquote
from collections import defaultdict
import os
import sys
from pathlib import Path
from io import BytesIO
import base64
from urllib.parse import quote
import re

from flask import send_file
from edgewind.time_utils import fmt_beijing, iso_beijing, to_beijing

api_bp = Blueprint('api', __name__, url_prefix='/api')
logger = logging.getLogger(__name__)
_device_api_key_warned = False

# 设备上报调试：限制心跳日志频率，避免刷屏
_last_hb_log_ts = {}

# 全局变量（将从app传入）
active_nodes = {}  # 将在app.py中初始化并传入
node_commands = {}  # 将在app.py中初始化并传入

# 节点超时时间（秒）
# 说明：此前为 10s，网络/设备偶发抖动（或一次心跳解析失败）就会导致节点被清空，前端表现为“运行一段时间后停机/无节点”。
# 这里改为环境变量可配置，默认 60s，更贴合真实链路。
NODE_TIMEOUT = max(10, int(os.environ.get("EDGEWIND_NODE_TIMEOUT_SEC", "60") or "60"))
db_executor = None  # 后台线程池，将在注册蓝图时设置
socketio_instance = None  # SocketIO实例，将在注册蓝图时设置
app_instance = None  # Flask应用实例

# ==================== 实时推送性能参数（可通过环境变量调节）====================
# 说明：多节点接入时，心跳/数据上报频率往往很高（例如 50Hz）。
# 如果后端对每次心跳都广播/推送，会导致事件风暴：CPU/网络/浏览器主线程都会被压垮，表现为“卡、慢、延迟大”。
# 因此这里默认启用“按节点节流 + 波形/频谱降采样”（仍然足够实时，且更平滑）。

def _env_int(key, default):
    try:
        return int(os.environ.get(key, default))
    except Exception:
        return int(default)

def _env_float(key, default):
    try:
        return float(os.environ.get(key, default))
    except Exception:
        return float(default)

# 每个节点的状态推送频率（Hz）：影响 node_status_update（概览/列表/指标）
STATUS_EMIT_HZ = max(1.0, _env_float("EDGEWIND_STATUS_EMIT_HZ", 5))
# 每个节点的监控推送频率（Hz）：影响 monitor_update（波形/频谱）
MONITOR_EMIT_HZ = max(1.0, _env_float("EDGEWIND_MONITOR_EMIT_HZ", 20))

# 波形/频谱降采样点数（0 表示不降采样）
MAX_WAVEFORM_POINTS = max(0, _env_int("EDGEWIND_WAVEFORM_POINTS", 256))
MAX_SPECTRUM_POINTS = max(0, _env_int("EDGEWIND_SPECTRUM_POINTS", 128))

# active_nodes 是否仅保存“轻量数据”（不保留 1024 点全量波形）
LIGHT_ACTIVE_NODES = str(os.environ.get("EDGEWIND_LIGHT_ACTIVE_NODES", "true")).strip().lower() == "true"

# 记录每个节点的上次推送时间（按节点节流）
_last_emit_status_ts = {}   # {node_id: ts}
_last_emit_monitor_ts = {}  # {node_id: ts}
# 节流：按节点减少数据库写入频率（避免高频心跳导致频繁落库）
_last_db_heartbeat_ts = {}  # {node_id: ts}

# 默认每个节点最多每 N 秒写一次 Device.last_heartbeat（可通过环境变量调节）
DEVICE_DB_UPDATE_INTERVAL_SEC = max(1.0, _env_float("EDGEWIND_DEVICE_DB_UPDATE_SEC", 5))


def _get_json_payload() -> dict:
    """
    更稳健的 JSON 解析：
    - 硬件端若漏发 Content-Type: application/json，Flask 的 request.get_json() 可能返回 None。
    - 这里优先 silent=True，然后回退到解析原始 body（UTF-8）。
    """
    data = request.get_json(silent=True)
    if isinstance(data, dict):
        return data
    # 兼容：如果 body 是 JSON 字符串但 header 不对，尝试手动解析
    try:
        raw = request.get_data(as_text=True)  # type: ignore[arg-type]
        if raw:
            obj = json.loads(raw)
            if isinstance(obj, dict):
                return obj
    except Exception:
        pass
    return {}


def _submit_update_device_heartbeat(node_id: str, payload: dict, fault_code: str, current_ts: float) -> None:
    """后台更新 Device 表：last_heartbeat/status/fault_code/location/hw_version（节流后调用）。"""
    if not db_executor or not app_instance:
        return

    def _job():
        with app_instance.app_context():
            try:
                device = Device.query.filter_by(device_id=node_id).first()
                now_utc = datetime.utcnow()
                location = payload.get('location') or node_id
                hw_version = payload.get('hw_version') or payload.get('hardware_version') or payload.get('fw_version')
                status_in = (payload.get('status') or 'online').strip().lower()
                status = 'faulty' if (fault_code and fault_code != 'E00') else ('online' if status_in != 'offline' else 'offline')

                if not device:
                    device = Device(
                        device_id=node_id,
                        location=location,
                        hw_version=hw_version or 'v1.0',
                        status=status,
                        fault_code=fault_code or 'E00',
                        last_heartbeat=now_utc
                    )
                    db.session.add(device)
                else:
                    device.location = location
                    if hw_version:
                        device.hw_version = hw_version
                    device.status = status
                    device.fault_code = fault_code or 'E00'
                    device.last_heartbeat = now_utc

                db.session.commit()
            except Exception:
                db.session.rollback()

    try:
        db_executor.submit(_job)
    except Exception:
        # 后台线程池异常不影响接口返回
        pass

def _should_emit(node_id, now_ts, hz, last_map):
    """按 node_id 节流：达到间隔才允许 emit。"""
    interval = 1.0 / float(hz)
    last = last_map.get(node_id, 0)
    if now_ts - last >= interval:
        last_map[node_id] = now_ts
        return True
    return False

def _downsample_list(arr, max_points):
    """
    简单降采样（抽取），用于减少 JSON 体积/前端渲染压力。
    - max_points=0：不处理
    """
    if max_points <= 0:
        return arr
    if not isinstance(arr, list):
        return []
    n = len(arr)
    if n <= max_points:
        return arr
    step = max(1, n // max_points)
    sampled = arr[::step]
    # 可能多一点，最终裁剪到 max_points
    return sampled[:max_points]

def _lighten_channels(channels):
    """将 channels 转为轻量版：保留展示必须字段，剔除大数组。"""
    if not isinstance(channels, list):
        return []
    out = []
    for ch in channels:
        if not isinstance(ch, dict):
            continue
        out.append({
            'id': ch.get('id', 0),
            'label': ch.get('label', ''),
            'unit': ch.get('unit', ''),
            'type': ch.get('type', ''),
            'range': ch.get('range', []),
            'color': ch.get('color', ''),
            'value': ch.get('value', ch.get('current_value', 0)),
        })
    return out


def init_api_blueprint(app, socketio, executor, nodes, commands):
    """初始化API蓝图的全局变量"""
    global active_nodes, node_commands, db_executor, socketio_instance, app_instance
    active_nodes = nodes
    node_commands = commands
    db_executor = executor
    socketio_instance = socketio
    app_instance = app


def _device_auth_or_401():
    """
    设备侧接口鉴权（可选）：
    - 若设置了环境变量 EDGEWIND_DEVICE_API_KEY，则要求请求头携带 X-EdgeWind-ApiKey 且匹配。
    - 若未设置，则默认不鉴权（兼容开发/演示环境），并仅在首次请求时打印一次安全提示。

    注意：这里用于设备上报接口（register/upload/heartbeat），不影响管理员登录与页面鉴权。
    """
    expected = (os.environ.get('EDGEWIND_DEVICE_API_KEY') or '').strip()
    if not expected:
        global _device_api_key_warned
        if not _device_api_key_warned:
            logger.warning("安全提示：未设置 EDGEWIND_DEVICE_API_KEY，设备上报接口将不鉴权（仅建议开发环境）。")
            _device_api_key_warned = True
        return None

    provided = (request.headers.get('X-EdgeWind-ApiKey')
                or request.headers.get('X-Device-ApiKey')
                or request.headers.get('X-Device-Key')
                or '').strip()

    if not provided or provided != expected:
        return jsonify({'error': 'Unauthorized'}), 401

    return None


# ==================== 设备注册API ====================

@api_bp.route('/register', methods=['POST'])
def register_device():
    """设备注册API"""
    try:
        auth_resp = _device_auth_or_401()
        if auth_resp:
            return auth_resp

        data = _get_json_payload()
        # 兼容：部分固件可能用 node_id
        device_id = data.get('device_id') or data.get('node_id')
        # 兼容：location 允许缺省（回退为 device_id），避免注册失败导致后续全链路不可用
        location = data.get('location') or device_id
        hw_version = data.get('hw_version') or data.get('hardware_version') or data.get('fw_version') or 'v1.0'
        
        if not device_id:
            return jsonify({'error': 'Missing device_id'}), 400
        logger.info(f"[/api/register] device_id={device_id}, location={location}, hw_version={hw_version}")
        
        # 检查设备是否已存在
        device = Device.query.filter_by(device_id=device_id).first()
        
        if device:
            # 更新现有设备信息
            device.location = location
            device.hw_version = hw_version
            device.status = 'online'
            device.last_heartbeat = datetime.utcnow()
            db.session.commit()
            return jsonify({
                'message': 'Device updated',
                'device_id': device_id
            }), 200
        else:
            # 创建新设备
            device = Device(
                device_id=device_id,
                location=location,
                hw_version=hw_version,
                status='online',
                last_heartbeat=datetime.utcnow()
            )
            db.session.add(device)
            db.session.commit()
            return jsonify({
                'message': 'Device registered successfully',
                'device_id': device_id
            }), 201
            
    except Exception as e:
        db.session.rollback()
        return jsonify({'error': str(e)}), 500


# ==================== 兼容接口：波形上传（旧模拟器 /api/upload）====================
@api_bp.route('/upload', methods=['POST'])
def upload_data():
    """
    兼容旧版模拟器/硬件上报接口：/api/upload

    sim.py 当前会向该接口发送：
    - device_id
    - status（normal / fault）
    - fault_code（E00-E05）
    - waveform（1024点数组）
    """
    try:
        auth_resp = _device_auth_or_401()
        if auth_resp:
            return auth_resp

        data = _get_json_payload()
        device_id = data.get('device_id')
        status = data.get('status', 'normal')
        fault_code = data.get('fault_code', 'E00')
        waveform = data.get('waveform')

        if not device_id:
            return jsonify({'error': 'Missing device_id'}), 400

        # 1) 确保设备存在
        device = Device.query.filter_by(device_id=device_id).first()
        if not device:
            device = Device(
                device_id=device_id,
                location=data.get('location', device_id),
                status='online',
                fault_code='E00',
                last_heartbeat=datetime.utcnow()
            )
            db.session.add(device)
            db.session.flush()

        # 2) 更新设备状态
        # 兼容 sim.py：status=normal/fault
        device.status = 'faulty' if status in ['fault', 'faulty'] or fault_code != 'E00' else 'online'
        device.fault_code = fault_code or 'E00'
        device.last_heartbeat = datetime.utcnow()

        # 2.1) 同步更新 active_nodes（统一“在线判定”口径）
        # 说明：
        # - 新版模拟器会调用 /api/node/heartbeat 更新 active_nodes
        # - 旧版/兼容链路会调用 /api/upload（仅更新数据库）
        # 为避免“统计显示在线=0，但卡片仍显示在线/有数据”的口径不一致，这里把 /api/upload 也视为一次心跳
        current_timestamp = time.time()
        active_nodes[device_id] = {
            'timestamp': current_timestamp,
            'status': 'faulty' if fault_code != 'E00' else 'online',
            'fault_code': fault_code or 'E00',
            'data': {
                **(data or {}),
                'node_id': device_id,
                'device_id': device_id,
                'location': device.location
            }
        }

        # 3) 保存波形数据点（用于历史趋势/后续分析）
        # 性能说明：多节点高频上报时，频繁落库会显著拖慢响应。
        # 可通过环境变量关闭：EDGEWIND_STORE_UPLOAD_DATAPOINTS=false
        store_upload_datapoints = str(os.environ.get("EDGEWIND_STORE_UPLOAD_DATAPOINTS", "true")).strip().lower() == "true"
        if store_upload_datapoints and waveform is not None:
            datapoint = DataPoint(
                device_id=device_id,
                waveform=json.dumps(waveform),
                status='fault' if fault_code != 'E00' else 'normal',
                fault_code=fault_code,
                timestamp=datetime.utcnow()
            )
            db.session.add(datapoint)

        # 4) 故障事件入库：/api/upload 也要遵循“故障事件(E00->E0X)创建工单”的口径
        # 说明：部分模拟器/旧链路仍使用 /api/upload 注入故障；如果这里只落库 datapoint 而不按事件建单，
        # 会导致“故障快照有，但故障管理/系统故障日志没有”的错觉。
        prev_fault = node_fault_states.get(device_id, 'E00')
        curr_fault = fault_code or 'E00'

        if prev_fault == 'E00' and curr_fault != 'E00':
            # 2秒内去重（防止网络重发/并发导致同秒两条）
            now_utc = datetime.utcnow()
            window_start = now_utc - timedelta(seconds=2)

            fault_info = FAULT_KNOWLEDGE_GRAPH.get(curr_fault) or FAULT_KNOWLEDGE_GRAPH.get(FAULT_CODE_MAP.get(curr_fault, '') or '')
            expected_fault_name = (fault_info or {}).get('name')

            recent = WorkOrder.query.filter(
                WorkOrder.device_id == device_id,
                WorkOrder.fault_time != None,
                WorkOrder.fault_time >= window_start
            ).order_by(WorkOrder.fault_time.desc()).first()

            if not (recent and expected_fault_name and (recent.fault_type == expected_fault_name or expected_fault_name in (recent.fault_type or ''))):
                create_work_order_from_fault(db, device_id, curr_fault, device.location, fault_time=now_utc)

        # 更新故障状态机（用于事件判定）
        node_fault_states[device_id] = curr_fault

        db.session.commit()

        # 4.1) WebSocket：/api/upload 也推送全局状态更新（保证实时监测“系统故障日志”能更新）
        try:
            if socketio_instance:
                socketio_instance.emit('node_status_update', {
                    'node_id': device_id,
                    'status': 'faulty' if curr_fault != 'E00' else 'online',
                    'fault_code': curr_fault,
                    'timestamp': current_timestamp,
                    'metrics': {
                        'voltage': float((data or {}).get('voltage', 0) or 0),
                        'voltage_neg': float((data or {}).get('voltage_neg', 0) or 0),
                        'current': float((data or {}).get('current', 0) or 0),
                        'leakage': float((data or {}).get('leakage', 0) or 0)
                    }
                }, namespace='/')
        except Exception:
            # 推送失败不影响接口返回
            pass

        # 5) 命令下发（维修完成 -> reset）
        # 重要：不要 pop！否则设备若“没及时解析响应”，命令会丢失。
        # 策略：命令会在设备上报 fault_code=E00 后自动清除（视为已执行）。
        resp = {'success': True}
        cmd = node_commands.get(device_id)
        if cmd:
            resp['command'] = cmd
            # ack：设备已恢复正常，则认为 reset 已执行
            if cmd == 'reset' and (fault_code == 'E00'):
                node_commands.pop(device_id, None)
        return jsonify(resp), 200

    except Exception as e:
        db.session.rollback()
        logger.error(f"/api/upload 处理失败: {e}")
        import traceback
        logger.error(traceback.format_exc())
        return jsonify({'error': str(e)}), 500


# ==================== 节点心跳API ====================

@api_bp.route('/node/heartbeat', methods=['POST'])
def node_heartbeat():
    """节点心跳接口 - 接收STM32节点的实时数据"""
    try:
        auth_resp = _device_auth_or_401()
        if auth_resp:
            return auth_resp

        data = _get_json_payload()
        node_id = data.get('node_id') or data.get('device_id')
        fault_code = (data.get('fault_code') or 'E00').strip() or 'E00'
        
        if not node_id:
            return jsonify({'error': 'Missing node_id'}), 400

        # 0. Update timestamp + Debug log (rate limited per node)
        current_timestamp = time.time()
        last = _last_hb_log_ts.get(node_id, 0)
        if current_timestamp - last >= 5:
            _last_hb_log_ts[node_id] = current_timestamp
            logger.info(f"[/api/node/heartbeat] node_id={node_id} fault={fault_code} ch={len(data.get('channels') or [])}")

        # 1. Update Active Node
        # 1. Update Active Node（可选：轻量化存储，避免多节点时内存/序列化成本过高）
        if LIGHT_ACTIVE_NODES:
            data_light = dict(data)
            # 剥离大数组，只保留必须元数据和值（便于概览/订阅初始数据）
            if 'channels' in data_light:
                data_light['channels'] = _lighten_channels(data_light.get('channels'))
            active_nodes[node_id] = {
                'timestamp': current_timestamp,
                'status': data.get('status', 'offline'),
                'fault_code': fault_code,
                'data': data_light
            }
        else:
            active_nodes[node_id] = {
                'timestamp': current_timestamp,
                'status': data.get('status', 'offline'),
                'fault_code': fault_code,
                'data': data
            }

        # 2. Initialize Data Structure
        processed_data = {
            'voltage': 0, 'voltage_neg': 0, 'current': 0, 'leakage': 0,
            'voltage_waveform': [], 'voltage_spectrum': [],
            'voltage_neg_waveform': [], 'voltage_neg_spectrum': [],
            'current_waveform': [], 'current_spectrum': [],
            'leakage_waveform': [], 'leakage_spectrum': []
        }

        # 3. Parse Channels（注意：这里会处理波形/频谱大数组；后续 emit 时会按需降采样）
        raw_channels = data.get('channels') or []
        if not isinstance(raw_channels, list):
            raw_channels = []
        for ch in raw_channels:
            if not isinstance(ch, dict):
                continue
            ch_id = ch.get('id')
            label = (ch.get('label') or '').strip()
            val = ch.get('value', ch.get('current_value', 0))
            wave = ch.get('waveform', [])
            # 统一字段名：优先 fft_spectrum；兼容历史设备的 fft
            spec = ch.get('fft_spectrum', ch.get('fft', []))

            if not isinstance(wave, list):
                wave = []
            if not isinstance(spec, list):
                spec = []

            # 降采样：减少 SocketIO JSON 体积（尤其多节点时效果明显）
            wave = _downsample_list(wave, MAX_WAVEFORM_POINTS)
            spec = _downsample_list(spec, MAX_SPECTRUM_POINTS)

            try:
                val_float = float(val) if val is not None else 0.0
            except Exception:
                val_float = 0.0

            # 先按 label 识别（中文优先）
            mapped = False
            if "直流" in label:
                # 兼容：label=“直流母线” 未标注正负时，默认当作正母线
                if ("-" in label) or ("负" in label):
                    processed_data['voltage_neg'] = val_float
                    processed_data['voltage_neg_waveform'] = wave
                    processed_data['voltage_neg_spectrum'] = spec
                else:
                    processed_data['voltage'] = val_float
                    processed_data['voltage_waveform'] = wave
                    processed_data['voltage_spectrum'] = spec
                mapped = True
            elif "漏" in label:
                processed_data['leakage'] = val_float
                processed_data['leakage_waveform'] = wave
                processed_data['leakage_spectrum'] = spec
                mapped = True
            elif ("负载" in label or "电流" in label) and "漏" not in label:
                processed_data['current'] = val_float
                processed_data['current_waveform'] = wave
                processed_data['current_spectrum'] = spec
                mapped = True

            # label 无法识别时，按通道 id 做兜底映射（与你给的示例结构一致）
            if (not mapped) and isinstance(ch_id, int):
                if ch_id == 0:
                    processed_data['voltage'] = val_float
                    processed_data['voltage_waveform'] = wave
                    processed_data['voltage_spectrum'] = spec
                elif ch_id == 1:
                    processed_data['current'] = val_float
                    processed_data['current_waveform'] = wave
                    processed_data['current_spectrum'] = spec
                elif ch_id == 2:
                    processed_data['leakage'] = val_float
                    processed_data['leakage_waveform'] = wave
                    processed_data['leakage_spectrum'] = spec

        # 4. Save to buffer
        if fault_code == 'E00':
            save_to_buffer(node_id, data, is_fault=False)
        else:
            save_to_buffer(node_id, data, is_fault=True)
        
        # 5. Fault snapshot logic
        previous_fault = node_fault_states.get(node_id, 'E00')
        current_fault = fault_code
        db_op_submitted = False  # 防止同一次故障事件被重复建单（同一秒出现两条记录）
        
        # Fault occurred
        if previous_fault == 'E00' and current_fault != 'E00':
            logger.info(f"🔴 检测到故障发生: {node_id} -> {current_fault}")
            
            if node_id not in node_snapshot_saved or node_snapshot_saved[node_id].get('fault_code') != current_fault:
                # Save before snapshot
                before_data = get_latest_normal_data(node_id)
                if before_data:
                    db_executor.submit(save_fault_snapshot, db, app_instance, node_id, current_fault, 'before', before_data['data'])
                
                # Save after snapshot
                db_executor.submit(save_fault_snapshot, db, app_instance, node_id, current_fault, 'after', data)
                
                node_snapshot_saved[node_id] = {
                    'fault_code': current_fault,
                    'saved_types': ['before', 'after']
                }

            # 关键修复：故障发生时必须创建/更新工单（不应被 node_commands=reset 等指令阻断）
            # 说明：故障快照保存与工单创建是两条链路，快照能保存但工单没创建会导致：
            # - 故障管理页看不到新故障（依赖 /api/faults -> work_orders）
            # - 实时监测页“系统故障日志”历史回填看不到新故障
            if db_executor:
                db_executor.submit(_handle_fault_database_operation, node_id, current_fault, data)
                db_op_submitted = True
        
        # Fault recovered
        elif previous_fault != 'E00' and current_fault == 'E00':
            logger.info(f"🟢 检测到故障恢复: {node_id} {previous_fault} -> E00")
            
            # Save recovery snapshots
            fault_data = get_latest_fault_data(node_id)
            if fault_data:
                db_executor.submit(save_fault_snapshot, db, app_instance, node_id, previous_fault, 'before_recovery', fault_data['data'])
            
            db_executor.submit(save_fault_snapshot, db, app_instance, node_id, previous_fault, 'after_recovery', data)
            
            if node_id in node_snapshot_saved:
                del node_snapshot_saved[node_id]
        
        node_fault_states[node_id] = current_fault
        
        # 6. WebSocket 推送（关键性能优化：按节点节流，避免多节点事件风暴）
        # 6.1 轻量状态推送（概览/侧边栏/指标）
        if _should_emit(node_id, current_timestamp, STATUS_EMIT_HZ, _last_emit_status_ts):
            socketio_instance.emit('node_status_update', {
                'node_id': node_id,
                'status': data.get('status', 'online'),
                'fault_code': fault_code,
                'timestamp': current_timestamp,
                'metrics': {
                    'voltage': processed_data.get('voltage', 0),
                    'voltage_neg': processed_data.get('voltage_neg', 0),
                    'current': processed_data.get('current', 0),
                    'leakage': processed_data.get('leakage', 0)
                }
            }, namespace='/')

        # 6.2 监控推送（仅订阅房间）：波形/频谱（也节流）
        if _should_emit(node_id, current_timestamp, MONITOR_EMIT_HZ, _last_emit_monitor_ts):
            socketio_instance.emit('monitor_update', {
                'node_id': node_id,
                'data': processed_data,
                'fault_code': fault_code
            }, room=f'node_{node_id}', namespace='/')

        # 7. Database operation in background（保底）
        # 说明：建单只应在“故障发生事件(E00->E0X)”触发一次。
        # 若上面已提交过任务，则不再重复提交，避免同一秒出现重复工单。
        if db_executor and (not db_op_submitted) and previous_fault == 'E00' and current_fault != 'E00':
            db_executor.submit(_handle_fault_database_operation, node_id, current_fault, data)
        
        # 8. Command response
        response_payload = {
            'success': True, 
            'node_id': node_id, 
            'timestamp': current_timestamp
        }
        # 命令下发（不要 pop，避免命令丢失；fault_code=E00 时视为已执行并清除）
        cmd = node_commands.get(node_id)
        if cmd:
            response_payload['command'] = cmd
            if cmd == 'reset' and fault_code == 'E00':
                node_commands.pop(node_id, None)
        
        # 9. 节流更新数据库设备心跳（避免 50Hz 高频心跳把 SQLite 打爆）
        last_db = _last_db_heartbeat_ts.get(node_id, 0)
        if current_timestamp - last_db >= DEVICE_DB_UPDATE_INTERVAL_SEC:
            _last_db_heartbeat_ts[node_id] = current_timestamp
            _submit_update_device_heartbeat(node_id, data, fault_code, current_timestamp)

        return jsonify(response_payload), 200

    except Exception as e:
        db.session.rollback()
        logger.error(f"❌ Heartbeat failed: {str(e)}")
        import traceback
        logger.error(traceback.format_exc())
        return jsonify({'error': str(e)}), 500


def _handle_fault_database_operation(node_id, fault_code, data):
    """后台处理故障数据库操作"""
    with app_instance.app_context():
        try:
            # Ensure device exists
            device = Device.query.filter_by(device_id=node_id).first()
            if not device:
                device = Device(device_id=node_id, location=data.get('location', 'N/A'), status='faulty')
                db.session.add(device)
                db.session.commit()
            
            # Create work order（按“故障事件”创建，而不是按“设备是否已有未关闭工单”去重）
            # 说明：
            # - 故障管理页的“故障日志”本质上是故障事件流；同一设备可能多次发生同一故障。
            # - 之前按 pending/processing 去重，会导致“快照有，但故障管理没新增记录”的错觉。
            # - 本函数只在 E00 -> E0X 事件发生时被调用（上层已做状态机判定），因此不会因高频心跳产生海量重复。
            # - 关键：同一秒内可能因为并发/重复提交导致建单两次，这里做 2 秒窗口去重。
            now_utc = datetime.utcnow()
            window_start = now_utc - timedelta(seconds=2)

            # 计算“当前故障事件”的标准故障名，用于更精确去重
            fault_info = FAULT_KNOWLEDGE_GRAPH.get(fault_code) or FAULT_KNOWLEDGE_GRAPH.get(FAULT_CODE_MAP.get(fault_code, '') or '')
            expected_fault_name = (fault_info or {}).get('name')

            recent = WorkOrder.query.filter(
                WorkOrder.device_id == node_id,
                WorkOrder.fault_time != None,
                WorkOrder.fault_time >= window_start
            ).order_by(WorkOrder.fault_time.desc()).first()

            if recent and expected_fault_name and (recent.fault_type == expected_fault_name or expected_fault_name in (recent.fault_type or '')):
                logger.info(f"⏭️ 跳过去重：{node_id} 在2秒内已创建同类故障工单 ({fault_code})")
                return

            create_work_order_from_fault(db, node_id, fault_code, device.location, fault_time=now_utc)
            db.session.commit()
            logger.info(f"✅ WorkOrder已创建: {node_id}")
        except Exception as e:
            db.session.rollback()
            logger.error(f"❌ 数据库操作失败: {node_id} - {str(e)}")


# ==================== 节点管理API ====================

@api_bp.route('/get_active_nodes', methods=['GET'])
@login_required
def get_active_nodes():
    """获取活动节点列表"""
    try:
        current_time = time.time()
        expired_nodes = []
        
        # 清理超时节点
        for node_id, node_info in list(active_nodes.items()):
            if current_time - node_info['timestamp'] > NODE_TIMEOUT:
                expired_nodes.append(node_id)
                del active_nodes[node_id]
        
        # 返回活动节点
        active_nodes_list = []
        for node_id, node_info in active_nodes.items():
            node_data = node_info['data'].copy()
            node_data['node_id'] = node_id
            active_nodes_list.append(node_data)
        
        return jsonify({
            'success': True,
            'nodes': active_nodes_list,
            'count': len(active_nodes_list),
            'expired_count': len(expired_nodes)
        }), 200
        
    except Exception as e:
        return jsonify({'error': str(e)}), 500


# ==================== 知识图谱API ====================

def _infer_fault_code_from_fault_type(fault_type: str | None) -> str:
    """根据工单里的故障中文名称推断故障代码（用于知识图谱等场景）"""
    if not fault_type:
        return 'E00'
    if '交流窜入' in fault_type:
        return 'E01'
    if '绝缘故障' in fault_type:
        return 'E02'
    if '电容老化' in fault_type or '电容' in fault_type:
        return 'E03'
    if 'IGBT' in fault_type or '开路' in fault_type:
        return 'E04'
    if '接地故障' in fault_type or '接地' in fault_type:
        return 'E05'
    return 'E00'


def _extract_actionable_bullets(detailed_report: str) -> list[str]:
    """从 AI 深度报告中提取“智能运维建议”部分的要点（以 '-' 开头的行）"""
    if not detailed_report:
        return []

    lines = [ln.strip() for ln in detailed_report.splitlines() if ln.strip()]
    bullets: list[str] = []
    in_advice = False

    for ln in lines:
        # 进入建议段落
        if ('智能运维建议' in ln) or ln.startswith('3.'):
            in_advice = True
            continue

        if in_advice:
            # 遇到下一段（如 4.）则停止
            if re.match(r'^\d+\.', ln) and (not ln.startswith('3.')):
                break
            if ln.startswith('-'):
                bullets.append(ln.lstrip('-').strip())

    return bullets


def _split_sentences(text: str) -> list[str]:
    """把中文描述按常见标点切分成短句，便于抽取“关键词标题”"""
    if not text:
        return []
    # 统一空白
    t = re.sub(r'\s+', ' ', str(text)).strip()
    # 用中文标点/分号/句号/顿号/换行切分
    parts = re.split(r'[。；;！!？?\n\r]+', t)
    out: list[str] = []
    for p in parts:
        p = p.strip(' ,，。；;:：\t')
        if p:
            out.append(p)
    return out


def _make_short_title(text: str, max_len: int = 12) -> str:
    """
    从一句话生成一个尽量“领域相关”的短标题，避免出现“说明/预防/立即”等泛词。
    规则（尽量简单稳定）：
    - 去掉前缀标签 [xx]
    - 若有 '：'，优先取冒号后的部分
    - 去掉常见动作前缀（使用/检查/更换/确认/停止...）但保留关键名词
    - 截断到 max_len
    """
    if not text:
        return ''
    s = str(text).strip()
    s = re.sub(r'^\[[^\]]+\]\s*', '', s)  # 去掉 [检测] 等
    if '：' in s:
        s = s.split('：', 1)[1].strip()
    if ':' in s:
        s = s.split(':', 1)[1].strip()

    # 去掉非常泛的前缀动词/提示词（只删开头，避免破坏内容）
    s = re.sub(r'^(请|建议|提示|立即|尽快|应|务必)\s*', '', s)
    s = re.sub(r'^(使用|检查|检测|更换|确认|排查|修复|处理|测试|测量)\s*', '', s)

    # 取到第一个逗号/顿号前，标题更像关键词
    s = re.split(r'[，,、;；]', s, 1)[0].strip()

    # 过滤掉太短/太泛的标题
    stop_titles = {'说明', '预防', '立即', '检查', '检测', '更换', '处理', '要点', '详情', '建议', '提示'}
    if (not s) or (s in stop_titles):
        return ''

    if len(s) > max_len:
        s = s[:max_len].rstrip()
    return s


@api_bp.route('/knowledge_graph/<fault_code>', methods=['GET'])
def get_knowledge_graph(fault_code):
    """获取故障诊断知识图谱"""
    try:
        graph_data = get_fault_knowledge_graph(fault_code)
        if graph_data:
            return jsonify(graph_data), 200
        else:
            return jsonify({'error': 'Fault code not found'}), 404
    except Exception as e:
        return jsonify({'error': str(e)}), 500


@api_bp.route('/graph/details/<int:log_id>/<path:node_name>', methods=['GET'])
@login_required
def get_graph_node_details(log_id: int, node_name: str):
    """
    返回“节点二级展开”数据（给 templates/faults.html 的知识图谱点击展开使用）

    前端会请求：
      /api/graph/details/<log_id>/<node_name>

    返回格式：
    {
      "children": [
        {"name": "...", "description": "..."},
        ...
      ]
    }
    """
    try:
        # 前端已 encodeURIComponent，这里做一次解码兜底（避免重复编码导致中文不匹配）
        try:
            node_name_decoded = unquote(node_name)
        except Exception:
            node_name_decoded = node_name

        order = WorkOrder.query.get(log_id)
        if not order:
            return jsonify({'error': '工单不存在', 'children': []}), 404

        # 优先使用设备当前故障码；若设备已恢复为 E00，则根据工单故障类型推断
        device = Device.query.filter_by(device_id=order.device_id).first()
        device_fault_code = getattr(device, 'fault_code', None) if device else None
        fault_code = device_fault_code if (device_fault_code and device_fault_code != 'E00') else _infer_fault_code_from_fault_type(order.fault_type)

        # 兼容：若 fault_code 是映射键（如 DC_CAPACITOR_AGING），先映射到标准 E01-E05
        mapped = FAULT_CODE_MAP.get(fault_code)
        if mapped:
            fault_code = mapped

        fault_info = FAULT_KNOWLEDGE_GRAPH.get(fault_code)
        if not fault_info:
            return jsonify({'children': []}), 200

        # 仅对“根本原因/解决方案”节点做二级展开；故障根节点默认不展开
        # 目标：二级节点直接展示“故障相关关键词”，并尽量固定 3 个
        children: list[dict] = []
        seen_desc: set[str] = set()

        def _add_child_from_text(text: str):
            """从一段文本生成 child（短标题+详细描述），过滤泛词"""
            if not text:
                return
            desc = re.sub(r'\s+', ' ', str(text)).strip()
            if not desc:
                return
            if desc.lower() in seen_desc:
                return
            seen_desc.add(desc.lower())

            title = _make_short_title(desc, max_len=12)
            if not title:
                return
            children.append({'name': title, 'description': desc})

        # 从知识库中匹配该节点（按名称）
        matched_desc = None
        for cause in fault_info.get('root_causes', []) or []:
            if cause.get('name') == node_name_decoded:
                matched_desc = cause.get('description') or ''
                break
        if matched_desc is None:
            for sol in fault_info.get('solutions', []) or []:
                if sol.get('name') == node_name_decoded:
                    matched_desc = sol.get('description') or ''
                    break

        # 1) 先用“节点自身描述”生成 1-2 个关键词（信息最相关）
        if matched_desc:
            # 尝试按短句拆分，最多取 2 条（避免全是同一句话）
            for seg in _split_sentences(matched_desc)[:3]:
                if len(children) >= 2:
                    break
                _add_child_from_text(seg)

        # 2) 再从详细报告提取“运维建议”要点，补齐到 3 条（但必须与当前节点语义相关）
        bullets = _extract_actionable_bullets(fault_info.get('detailed_report', ''))
        if bullets:
            # 用一组行业关键词做轻量匹配：只返回与当前节点真正相关的内容
            kw_pool = [
                'ESR', '纹波', '电容', '绝缘', '接地', '隔离', '变压器', '滤波', 'IGBT', '门极', '驱动', '温度', '散热', '电桥', '选线'
            ]
            node_kws = [k for k in kw_pool if k and (k in node_name_decoded)]
            # 再补充几个动作词（只在节点名包含时才启用）
            for k in ['检测', '测试', '更换', '检查', '排查', '修复']:
                if k in node_name_decoded and k not in node_kws:
                    node_kws.append(k)

            def _is_relevant(b: str) -> bool:
                if not node_kws:
                    return False
                return any(k in b for k in node_kws)

            picked = [b for b in bullets if _is_relevant(b)]
            for b in picked:
                if len(children) >= 3:
                    break
                # 取内容部分作为描述；标题由 _make_short_title 自动从描述中抽取关键词
                m = re.match(r'^\[[^\]]+\]\s*(.*)$', b)
                desc = (m.group(1) if m else b).strip()
                _add_child_from_text(desc)

        # 3) 若仍不足 3 个，用 fault_info 的 root_cause/solution 等字段补齐（只取最短关键词）
        if len(children) < 3:
            for extra in [fault_info.get('root_cause', ''), fault_info.get('solution', '')]:
                if len(children) >= 3:
                    break
                for seg in _split_sentences(extra)[:2]:
                    _add_child_from_text(seg)

        # 4) 最终保证最多 3 个（不做“说明/预防/立即”等泛词标题）
        children = children[:3]

        return jsonify({'children': children}), 200
    except Exception as e:
        logger.exception(f"获取图谱节点详情失败: {e}")
        return jsonify({'error': str(e), 'children': []}), 500


@api_bp.route('/ai_report/<fault_code>', methods=['GET'])
def get_ai_report(fault_code):
    """获取AI诊断报告"""
    try:
        report = generate_ai_report(fault_code)
        return jsonify({'report': report}), 200
    except Exception as e:
        return jsonify({'error': str(e)}), 500


# ==================== 工单管理API ====================

@api_bp.route('/work_orders', methods=['GET'])
@login_required
def get_work_orders():
    """获取工单列表"""
    try:
        orders = WorkOrder.query.order_by(WorkOrder.fault_time.desc()).all()
        result = []
        for order in orders:
            result.append({
                'id': order.id,
                'device_id': order.device_id,
                'fault_time': (order.fault_time + timedelta(hours=8)).strftime('%Y-%m-%d %H:%M:%S') if order.fault_time else None,
                'location': order.location,
                'fault_type': order.fault_type,
                'status': order.status,
                'ai_recommendation': order.ai_recommendation
            })
        return jsonify(result), 200
    except Exception as e:
        return jsonify({'error': str(e)}), 500


@api_bp.route('/work_orders/<int:order_id>', methods=['PATCH'])
@login_required
def update_work_order(order_id):
    """更新工单状态"""
    try:
        order = WorkOrder.query.get(order_id)
        if not order:
            return jsonify({'error': 'Work order not found'}), 404
        
        data = request.get_json()
        if 'status' in data:
            order.status = data['status']
            
            # 如果标记为已修复，发送重置命令给节点
            if data['status'] in ['fixed', 'resolved']:
                node_commands[order.device_id] = 'reset'
        
        db.session.commit()
        return jsonify({'message': 'Work order updated'}), 200
    except Exception as e:
        db.session.rollback()
        return jsonify({'error': str(e)}), 500


# ==================== 兼容接口：故障日志（templates/faults.html 使用）====================
@api_bp.route('/faults', methods=['GET'])
@login_required
def get_faults():
    """
    返回故障日志列表（与 faults.html 的 /api/faults 兼容）
    返回格式：
    {
      "faults": [ ... ]
    }
    """
    try:
        start_ts = time.time()
        logger.info("[/api/faults] 开始获取故障日志（work_orders -> faults）")

        def _infer_fault_code(fault_type: str | None) -> str:
            """根据故障中文名称推断故障代码（用于兼容旧前端展示/知识图谱加载）"""
            if not fault_type:
                return 'E00'
            if '交流窜入' in fault_type:
                return 'E01'
            if '绝缘故障' in fault_type:
                return 'E02'
            if '电容老化' in fault_type or '电容' in fault_type:
                return 'E03'
            if 'IGBT' in fault_type or '开路' in fault_type:
                return 'E04'
            if '接地故障' in fault_type or '接地' in fault_type:
                return 'E05'
            return 'E00'

        def _infer_severity(fault_code: str | None, fault_type: str | None = None) -> str:
            """
            推断严重程度（供 faults.html 展示）

            返回值：severe | major | general
            说明：
            - 目前 WorkOrder 表没有 severity 字段，因此在接口层按故障码推断，确保前端不再显示“未知”
            - 规则可按业务需要调整
            """
            fc = (fault_code or '').strip()
            ft = (fault_type or '').strip()

            # 最高严重：IGBT 开路 / 直流母线接地
            if fc in ('E04', 'E05'):
                return 'severe'

            # 主要：交流窜入 / 绝缘故障
            if fc in ('E01', 'E02'):
                return 'major'

            # 一般：电容老化
            if fc in ('E03',):
                return 'general'

            # 兜底：根据中文名称再判断一次（兼容历史/异常数据）
            if 'IGBT' in ft or '开路' in ft or '接地' in ft:
                return 'severe'
            if '交流窜入' in ft or '绝缘故障' in ft:
                return 'major'
            if '电容' in ft:
                return 'general'

            return 'general'

        orders = WorkOrder.query.order_by(WorkOrder.fault_time.desc()).all()
        faults = []
        for order in orders:
            device = Device.query.filter_by(device_id=order.device_id).first()
            # 前端期望 status: pending/processing/resolved
            status = order.status or 'pending'
            if status == 'fixed':
                status = 'resolved'

            # 优先使用设备当前故障码；若设备已恢复为E00，则根据工单故障类型推断
            device_fault_code = getattr(device, 'fault_code', None) if device else None
            fault_code = device_fault_code if (device_fault_code and device_fault_code != 'E00') else _infer_fault_code(order.fault_type)
            severity = _infer_severity(fault_code, order.fault_type)

            # 时间统一口径：返回“北京时间”
            # - fault_time：ISO 8601（带 +08:00 时区偏移），前端 new Date() 解析不会受本机时区影响
            # - time：北京时间展示字符串（YYYY-MM-DD HH:MM:SS）
            local_dt = (order.fault_time + timedelta(hours=8)) if order.fault_time else None
            fault_time_iso = local_dt.strftime('%Y-%m-%dT%H:%M:%S+08:00') if local_dt else None
            fault_time_display = local_dt.strftime('%Y-%m-%d %H:%M:%S') if local_dt else None

            faults.append({
                'id': order.id,
                'device_id': order.device_id,
                'location': order.location or (device.location if device else 'N/A'),
                'fault_type': order.fault_type,
                'fault_code': fault_code,
                'severity': severity,
                'status': status,
                'ai_recommendation': order.ai_recommendation,
                # 兼容：faults.html 使用 fault_time；也保留 time 字段（旧逻辑可能用）
                'fault_time': fault_time_iso,
                'time': fault_time_display
            })
        cost_ms = int((time.time() - start_ts) * 1000)
        logger.info(f"[/api/faults] 完成：数量={len(faults)} 耗时={cost_ms}ms")
        return jsonify({'faults': faults}), 200
    except Exception as e:
        logger.exception(f"[/api/faults] 失败: {e}")
        return jsonify({'faults': [], 'message': str(e)}), 500


@api_bp.route('/faults/<int:fault_id>/dispatch', methods=['POST'])
@login_required
def dispatch_fault(fault_id: int):
    """派单：pending -> processing（与 faults.html 兼容）"""
    try:
        order = WorkOrder.query.get(fault_id)
        if not order:
            return jsonify({'success': False, 'message': '工单不存在'}), 404
        order.status = 'processing'
        db.session.commit()
        return jsonify({'success': True}), 200
    except Exception as e:
        db.session.rollback()
        return jsonify({'success': False, 'message': str(e)}), 500


@api_bp.route('/faults/<int:fault_id>/resolve', methods=['POST'])
@login_required
def resolve_fault(fault_id: int):
    """维修完成：processing -> resolved，并向节点下发 reset（与 faults.html 兼容）"""
    try:
        order = WorkOrder.query.get(fault_id)
        if not order:
            return jsonify({'success': False, 'message': '工单不存在'}), 404
        order.status = 'resolved'
        # 下发复位命令（兼容 sim.py 支持 reset/reset_local_state）
        node_commands[order.device_id] = 'reset'
        db.session.commit()
        return jsonify({'success': True}), 200
    except Exception as e:
        db.session.rollback()
        return jsonify({'success': False, 'message': str(e)}), 500


# ==================== 故障快照API ====================

@api_bp.route('/snapshots', methods=['GET'])
@login_required
def get_snapshots():
    """获取故障快照列表"""
    try:
        device_id = request.args.get('device_id')
        fault_code = request.args.get('fault_code')
        
        query = FaultSnapshot.query
        if device_id:
            query = query.filter_by(device_id=device_id)
        if fault_code:
            query = query.filter_by(fault_code=fault_code)
        
        snapshots = query.order_by(FaultSnapshot.timestamp.desc()).all()
        result = [s.to_dict() for s in snapshots]
        
        return jsonify(result), 200
    except Exception as e:
        return jsonify({'error': str(e)}), 500


# ==================== 兼容接口：设备列表（overview/settings 使用）====================
@api_bp.route('/devices', methods=['GET'])
@login_required
def get_devices():
    """返回设备列表（简化版，满足前端展示/筛选）
    
    默认只返回“近期活跃设备”，避免概览页堆满历史离线节点。
    如需全部设备，调用方可显式传入 all=true。
    """
    try:
        # online_only=true：仅返回“实时在线”的设备（与 active_nodes 统计口径一致）
        # 用于系统概览页，避免“最近30分钟上报过但当前已离线”的设备仍显示为卡片
        online_only = request.args.get('online_only', 'false').lower() == 'true'

        # 默认启用活跃过滤；仅当 all=true 时返回全部
        all_devices = request.args.get('all', 'false').lower() == 'true'
        active_only = not all_devices or request.args.get('active_only', 'false').lower() == 'true'
        minutes = int(request.args.get('minutes', 30))
        devices_query = Device.query

        # 计算当前“实时在线”节点集合（NODE_TIMEOUT 秒内有心跳）
        current_time = time.time()
        realtime_online_ids = {
            node_id for node_id, info in list(active_nodes.items())
            if current_time - info.get('timestamp', 0) <= NODE_TIMEOUT
        }

        if online_only:
            # 概览页：只展示实时在线节点（可包含故障节点）
            if not realtime_online_ids:
                logger.info(f"[/api/devices] 在线过滤=是，在线节点=0，args={dict(request.args)} -> 返回0")
                return jsonify({'success': True, 'devices': []}), 200
            devices_query = devices_query.filter(Device.device_id.in_(list(realtime_online_ids)))
        elif active_only:
            cutoff = datetime.utcnow() - timedelta(minutes=minutes)
            # 仅保留在截止时间后有心跳的设备
            devices_query = devices_query.filter(Device.last_heartbeat != None, Device.last_heartbeat >= cutoff)

        devices = devices_query.order_by(Device.registered_at.desc()).all()
        result = []
        for d in devices:
            # 计算返回给前端的状态：以“实时在线(active_nodes)”为准，避免数据库状态滞后
            computed_status = 'offline'
            if d.device_id in realtime_online_ids:
                node_info = active_nodes.get(d.device_id, {}) or {}
                fc = node_info.get('fault_code') or d.fault_code or 'E00'
                computed_status = 'faulty' if fc != 'E00' else 'online'

            result.append({
                'device_id': d.device_id,
                'location': d.location,
                'status': computed_status,
                'fault_code': d.fault_code,
                'last_heartbeat': (d.last_heartbeat + timedelta(hours=8)).strftime('%Y-%m-%d %H:%M:%S') if d.last_heartbeat else None
            })

        # 打印一次关键日志，便于定位“页面仍显示卡片”的原因（缓存/旧接口/参数未生效等）
        logger.info(
            f"[/api/devices] args={dict(request.args)} online_only={'是' if online_only else '否'} "
            f"active_only={'是' if active_only else '否'} minutes={minutes} "
            f"实时在线={len(realtime_online_ids)} 返回={len(result)}"
        )
        return jsonify({'success': True, 'devices': result}), 200
    except Exception as e:
        return jsonify({'success': False, 'message': str(e), 'devices': []}), 500


# ==================== 兼容接口：仪表盘统计（base/overview 使用）====================
@api_bp.route('/dashboard/stats', methods=['GET'])
@login_required
def get_dashboard_stats():
    """
    返回仪表盘统计数据（尽量与旧前端字段兼容）
    """
    try:
        # 基于 active_nodes 统计在线/离线/故障
        current_time = time.time()
        total_nodes = 0
        online_nodes = 0
        faulty_nodes = 0
        offline_nodes = 0

        for node_id, node_info in list(active_nodes.items()):
            if current_time - node_info.get('timestamp', 0) > NODE_TIMEOUT:
                offline_nodes += 1
                continue
            total_nodes += 1
            fc = node_info.get('fault_code', 'E00')
            if fc and fc != 'E00':
                faulty_nodes += 1
            else:
                online_nodes += 1

        # 基于工单统计
        today_start = datetime.now().replace(hour=0, minute=0, second=0, microsecond=0)
        today_cumulative = WorkOrder.query.filter(WorkOrder.fault_time >= today_start).count()
        today_resolved = WorkOrder.query.filter(WorkOrder.fault_time >= today_start, WorkOrder.status.in_(['resolved', 'fixed'])).count()
        current_pending = WorkOrder.query.filter(WorkOrder.status.in_(['pending', 'processing'])).count()

        # 简单健康分
        health_score = max(0, 100 - faulty_nodes * 20 - offline_nodes * 10)

        return jsonify({
            'total_nodes': total_nodes,
            'online_nodes': online_nodes,
            'faulty_nodes': faulty_nodes,
            'offline_nodes': offline_nodes,
            'current_fault_count': faulty_nodes,
            'offline_count': offline_nodes,
            'today_cumulative': today_cumulative,
            'today_resolved': today_resolved,
            'current_pending': current_pending,
            'system_health_score': health_score
        }), 200

    except Exception as e:
        logger.error(f"获取仪表盘统计失败: {str(e)}")
        return jsonify({'error': str(e)}), 500


# ==================== 兼容接口：故障分析数据（overview 使用）====================
@api_bp.route('/dashboard/fault_analytics', methods=['GET'])
@login_required
def get_fault_analytics():
    """
    返回故障分析数据：故障类型分布 + 24小时故障趋势
    """
    try:
        from edgewind.knowledge_graph import FAULT_CODE_MAP
        
        def _normalize_fault_type_name(name: str) -> str:
            """
            统一故障名称为“纯中文标准名”，避免出现：
            - 同一故障被统计成多条（中文/英文/中英混写）
            - 前端饼图标签过长导致重叠
            """
            if not name:
                return '未知故障'
            s = str(name).strip()

            # 1) 去掉括号内英文：例如 “交流窜入 (AC Intrusion)” -> “交流窜入”
            for sep in [' (', '（']:
                if sep in s:
                    s = s.split(sep, 1)[0].strip()
                    break

            # 2) 兼容英文名称（极少数老数据）
            english_map = {
                'AC Intrusion': '交流窜入',
                'Insulation Fault': '绝缘故障',
                'Capacitor Aging': '直流母线电容老化',
                'IGBT Open Circuit': '变流器IGBT开路',
                'DC Bus Grounding Fault': '直流母线接地故障'
            }
            if s in english_map:
                return english_map[s]

            # 3) 若是短中文别名，映射到标准故障名（与项目定义保持一致）
            alias_map = {
                '电容老化': '直流母线电容老化',
                'IGBT开路': '变流器IGBT开路',
                '接地故障': '直流母线接地故障'
            }
            return alias_map.get(s, s)

        # 1. 故障类型分布（从工单统计）
        fault_type_counter = defaultdict(int)
        orders = WorkOrder.query.all()
        for order in orders:
            fault_type = _normalize_fault_type_name(order.fault_type or '未知故障')
            fault_type_counter[fault_type] += 1
        
        # 构建饼图数据（按故障类型名称）
        fault_type_labels = []
        fault_type_values = []
        for fault_type, count in sorted(fault_type_counter.items(), key=lambda x: x[1], reverse=True):
            if count > 0:
                fault_type_labels.append(fault_type)
                fault_type_values.append(count)
        
        # 如果没有数据，返回空数据
        if not fault_type_labels:
            fault_type_labels = ['正常']
            fault_type_values = [0]
        
        # 2. 24小时故障趋势（按小时分组统计）
        # 重要：数据库存储的是 UTC 时间，但前端展示/用户认知都是北京时间（UTC+8）
        # 因此这里统一转成北京时间计算，确保图表横轴显示的"小时"与用户实际时间一致
        now_utc = datetime.utcnow()
        now_beijing = now_utc + timedelta(hours=8)  # 转换为北京时间
        hour_start_beijing = now_beijing.replace(minute=0, second=0, microsecond=0) - timedelta(hours=23)
        
        # 查询窗口：转回UTC用于数据库查询（数据库内部存UTC）
        hour_start_utc = hour_start_beijing - timedelta(hours=8)
        recent_orders = WorkOrder.query.filter(
            WorkOrder.fault_time >= hour_start_utc
        ).all()
        
        # 按小时分组统计（转为北京时间后再分组）
        hourly_count = defaultdict(int)
        for order in recent_orders:
            if order.fault_time:
                # 数据库 fault_time 是UTC，转为北京时间
                order_time_beijing = order.fault_time + timedelta(hours=8)
                order_hour_beijing = order_time_beijing.replace(minute=0, second=0, microsecond=0)
                hour_key = order_hour_beijing.strftime('%H:%M')
                hourly_count[hour_key] += 1
        
        # 生成24小时时间序列（北京时间，即使某小时没有数据也要显示0）
        hours_list = []
        faults_list = []
        for i in range(24):
            hour_time_beijing = hour_start_beijing + timedelta(hours=i)
            hour_label = hour_time_beijing.strftime('%H:%M')
            hours_list.append(hour_label)
            faults_list.append(hourly_count.get(hour_label, 0))
        
        return jsonify({
            'fault_type_distribution': {
                'labels': fault_type_labels,
                'values': fault_type_values
            },
            'hourly_fault_frequency': {
                'hours': hours_list,
                'faults': faults_list
            }
        }), 200
        
    except Exception as e:
        logger.error(f"获取故障分析数据失败: {str(e)}")
        import traceback
        logger.error(traceback.format_exc())
        return jsonify({
            'fault_type_distribution': {
                'labels': [],
                'values': []
            },
            'hourly_fault_frequency': {
                'hours': [],
                'faults': []
            },
            'error': str(e)
        }), 500

# ==================== 兼容接口：故障快照（前端 snapshots.html 使用）====================
@api_bp.route('/fault_snapshots', methods=['GET'])
@login_required
def get_fault_snapshots_events():
    """
    返回快照事件列表（与 templates/snapshots.html 兼容）
    """
    try:
        device_id = request.args.get('device_id')
        fault_code = request.args.get('fault_code')
        snapshot_type = request.args.get('snapshot_type')
        limit = min(int(request.args.get('limit', 500)), 1000)

        query = FaultSnapshot.query
        if device_id:
            query = query.filter_by(device_id=device_id)
        if fault_code:
            query = query.filter_by(fault_code=fault_code)
        if snapshot_type:
            query = query.filter_by(snapshot_type=snapshot_type)

        snapshots = query.order_by(FaultSnapshot.timestamp.desc()).limit(limit).all()

        # 按“设备 + 故障代码 + 本地时间(秒)”分组
        events_dict = {}
        for snapshot in snapshots:
            local_time = snapshot.timestamp + timedelta(hours=8)
            time_key = local_time.strftime('%Y%m%d%H%M%S')
            key = f"{snapshot.device_id}_{snapshot.fault_code}_{time_key}"
            if key not in events_dict:
                events_dict[key] = {
                    'device_id': snapshot.device_id,
                    'fault_code': snapshot.fault_code,
                    'timestamp': local_time.strftime('%Y-%m-%d %H:%M:%S'),
                    'snapshot_count': 0
                }
            events_dict[key]['snapshot_count'] += 1

        return jsonify({'success': True, 'events': list(events_dict.values())})
    except Exception as e:
        logger.error(f"获取故障快照事件失败: {e}")
        return jsonify({'success': False, 'message': str(e)}), 500


@api_bp.route('/fault_snapshots/event/<device_id>/<fault_code>/<timestamp>', methods=['GET', 'DELETE'])
@login_required
def handle_fault_event_snapshots(device_id, fault_code, timestamp):
    """
    GET: 返回某个事件的所有快照（before/after/before_recovery/after_recovery）
    DELETE: 删除该事件的所有快照
    """
    try:
        timestamp = unquote(timestamp)
        local_time = datetime.strptime(timestamp, '%Y-%m-%d %H:%M:%S')
        # 前端传的是本地时间(UTC+8)，转回UTC用于查询
        utc_start = local_time - timedelta(hours=8)
        utc_end = utc_start + timedelta(seconds=1)

        query = FaultSnapshot.query.filter(
            FaultSnapshot.device_id == device_id,
            FaultSnapshot.fault_code == fault_code,
            FaultSnapshot.timestamp >= utc_start,
            FaultSnapshot.timestamp < utc_end
        )

        if request.method == 'DELETE':
            to_delete = query.all()
            deleted_count = len(to_delete)
            for s in to_delete:
                db.session.delete(s)
            db.session.commit()
            return jsonify({'success': True, 'deleted': deleted_count, 'message': '删除成功'})

        # GET
        snaps = query.order_by(FaultSnapshot.timestamp.asc()).all()
        grouped = {'before': [], 'after': [], 'before_recovery': [], 'after_recovery': []}
        for s in snaps:
            if s.snapshot_type in grouped:
                grouped[s.snapshot_type].append(s.to_dict())

        return jsonify({
            'success': True,
            'device_id': device_id,
            'fault_code': fault_code,
            'timestamp': timestamp,
            'snapshots': grouped
        })

    except Exception as e:
        logger.error(f"处理故障事件快照失败: {e}")
        db.session.rollback()
        return jsonify({'success': False, 'message': str(e)}), 500


# ==================== 系统配置API ====================

@api_bp.route('/config', methods=['GET', 'POST'])
@login_required
def manage_config():
    """获取或更新系统配置"""
    try:
        if request.method == 'GET':
            configs = SystemConfig.query.all()
            result = {}
            for config in configs:
                result[config.key] = json.loads(config.value) if config.value else None
            return jsonify(result), 200
        
        elif request.method == 'POST':
            data = request.get_json()
            for key, value in data.items():
                config = SystemConfig.query.filter_by(key=key).first()
                if config:
                    config.value = json.dumps(value)
                    config.updated_at = datetime.utcnow()
                else:
                    config = SystemConfig(key=key, value=json.dumps(value))
                    db.session.add(config)
            
            db.session.commit()
            return jsonify({'message': 'Config updated'}), 200
    except Exception as e:
        db.session.rollback()
        return jsonify({'error': str(e)}), 500



# ==================== 管理接口：设置页（templates/settings.html）====================

def _get_sqlite_db_path_from_uri(db_uri: str):
    """
    从 SQLAlchemy SQLite URI 中提取数据库文件路径。
    支持：
    - sqlite:///relative/path.db
    - sqlite:////absolute/path.db
    """
    if not isinstance(db_uri, str):
        return None
    if not db_uri.startswith("sqlite:///"):
        return None

    raw = db_uri[len("sqlite:///"):]
    if not raw:
        return None

    # 这里 raw 可能是：
    # - instance/wind_farm.db（相对路径）
    # - C:/xxx/instance/wind_farm.db（绝对路径）
    try:
        return Path(raw)
    except Exception:
        return None


@api_bp.route('/admin/system_info', methods=['GET'])
@login_required
def admin_system_info():
    """系统信息（供系统设置页展示）"""
    try:
        # 1) 版本号：优先环境变量，未配置则给一个默认值
        version = os.environ.get('EDGEWIND_VERSION', 'v1.4.0')

        # 2) 数据库大小（仅对 SQLite 计算文件大小）
        db_uri = (app_instance.config.get('SQLALCHEMY_DATABASE_URI') if app_instance else '') or ''
        db_size_mb = 0.0
        sqlite_path = _get_sqlite_db_path_from_uri(db_uri)
        if sqlite_path is not None:
            # 相对路径以项目根目录为基准（与 Config 的绝对化逻辑保持一致）
            if not sqlite_path.is_absolute():
                project_root = Path(__file__).resolve().parents[1]  # edgewind/
                project_root = project_root.parent  # 项目根
                sqlite_path = (project_root / sqlite_path).resolve()
            if sqlite_path.exists():
                db_size_mb = round(sqlite_path.stat().st_size / (1024 * 1024), 2)

        # 3) 活跃节点（NODE_TIMEOUT 秒内有心跳）
        current_time = time.time()
        active_node_ids = [
            node_id for node_id, info in list(active_nodes.items())
            if current_time - info.get('timestamp', 0) <= NODE_TIMEOUT
        ]

        # 4) 工单统计
        total_orders = WorkOrder.query.count()
        pending_orders = WorkOrder.query.filter(WorkOrder.status.in_(['pending', 'processing'])).count()
        resolved_orders = WorkOrder.query.filter(WorkOrder.status.in_(['resolved', 'fixed'])).count()

        # 5) 异步模式（eventlet/gevent/threading）
        async_mode = getattr(socketio_instance, 'async_mode', None) or os.environ.get('FORCE_ASYNC_MODE', 'auto')

        return jsonify({
            'success': True,
            'data': {
                'version': version,
                'database_size_mb': db_size_mb,
                'database_uri': 'sqlite' if str(db_uri).startswith('sqlite') else 'other',
                'active_nodes': len(active_node_ids),
                'workorders': {
                    'total': total_orders,
                    'pending': pending_orders,
                    'resolved': resolved_orders
                },
                'async_mode': async_mode,
                'python_version': sys.version.split()[0]
            }
        }), 200
    except Exception as e:
        logger.error(f"[admin_system_info] 获取系统信息失败: {e}")
        import traceback
        logger.error(traceback.format_exc())
        return jsonify({'success': False, 'error': str(e)}), 500


@api_bp.route('/admin/config', methods=['GET', 'POST'])
@login_required
def admin_config():
    """
    设置页配置读写接口（与 templates/settings.html 对齐）
    返回格式：{success: true, data: {...}}
    """
    try:
        keys = [
            'poll_interval',
            'voltage_max',
            'leakage_threshold',
            'auto_refresh',
            'fft_enabled',
            'show_debug_log',
            'log_retention'
        ]

        if request.method == 'GET':
            data = {}
            for k in keys:
                config = SystemConfig.query.filter_by(key=k).first()
                if config and config.value is not None:
                    try:
                        data[k] = json.loads(config.value)
                    except Exception:
                        data[k] = config.value
            return jsonify({'success': True, 'data': data}), 200

        # POST
        payload = request.get_json() or {}
        for k in keys:
            if k not in payload:
                continue
            v = payload.get(k)
            row = SystemConfig.query.filter_by(key=k).first()
            if row:
                row.value = json.dumps(v, ensure_ascii=False)
                row.updated_at = datetime.utcnow()
            else:
                row = SystemConfig(key=k, value=json.dumps(v, ensure_ascii=False), description='系统设置')
                db.session.add(row)

        db.session.commit()
        return jsonify({'success': True, 'message': '配置已保存'}), 200
    except Exception as e:
        db.session.rollback()
        return jsonify({'success': False, 'error': str(e)}), 500


@api_bp.route('/admin/cleanup_old_data', methods=['POST'])
@login_required
def admin_cleanup_old_data():
    """按保留天数清理历史数据（波形数据 + 已完成工单）"""
    try:
        payload = request.get_json() or {}
        retention_days = int(payload.get('retention_days', 30))
        if retention_days <= 0:
            return jsonify({'success': False, 'error': 'retention_days 必须大于 0'}), 400

        cutoff = datetime.utcnow() - timedelta(days=retention_days)

        # 1) 删除过期波形数据
        datapoints_deleted = DataPoint.query.filter(DataPoint.timestamp < cutoff).delete(synchronize_session=False)

        # 2) 删除已完成工单（resolved/fixed）
        workorders_deleted = WorkOrder.query.filter(
            WorkOrder.fault_time < cutoff,
            WorkOrder.status.in_(['resolved', 'fixed'])
        ).delete(synchronize_session=False)

        db.session.commit()
        return jsonify({
            'success': True,
            'details': {
                'datapoints_deleted': int(datapoints_deleted or 0),
                'workorders_deleted': int(workorders_deleted or 0)
            }
        }), 200
    except Exception as e:
        db.session.rollback()
        return jsonify({'success': False, 'error': str(e)}), 500


@api_bp.route('/admin/clear_all_data', methods=['POST'])
@login_required
def admin_clear_all_data():
    """清空所有历史数据（高危）：波形数据 + 工单 + 故障快照"""
    try:
        # 注意：保留用户/设备表，避免系统不可登录或设备列表丢失
        datapoints_deleted = DataPoint.query.delete(synchronize_session=False)
        workorders_deleted = WorkOrder.query.delete(synchronize_session=False)
        snapshots_deleted = FaultSnapshot.query.delete(synchronize_session=False)
        db.session.commit()
        return jsonify({
            'success': True,
            'details': {
                'datapoints_deleted': int(datapoints_deleted or 0),
                'workorders_deleted': int(workorders_deleted or 0),
                'snapshots_deleted': int(snapshots_deleted or 0)
            }
        }), 200
    except Exception as e:
        db.session.rollback()
        return jsonify({'success': False, 'error': str(e)}), 500


# ==================== 兼容接口：导出工单（templates/faults.html 使用）====================

@api_bp.route('/workorder/export', methods=['POST'])
@login_required
def export_workorder_docx():
    """
    导出工单 Word 文档（.docx）

    前端（faults.html）会提交：
    - log_id: 工单ID（WorkOrder.id）
    - graph_image: 可选，ECharts dataURL（base64 PNG）
    """
    try:
        payload = request.get_json() or {}
        log_id = payload.get('log_id')
        graph_image = payload.get('graph_image')

        if not log_id:
            return jsonify({'success': False, 'error': '缺少 log_id'}), 400

        order = WorkOrder.query.get(int(log_id))
        if not order:
            return jsonify({'success': False, 'error': f'工单不存在: {log_id}'}), 404

        device = Device.query.filter_by(device_id=order.device_id).first()
        if not device:
            # 兼容：设备可能被清理/未入库
            device = Device(device_id=order.device_id, location=order.location or 'Unassigned', status='offline')

        # 使用“专业排版版”导出模板（与旧版 app.py 效果对齐）
        from edgewind.report_generator import generate_workorder_docx
        doc = generate_workorder_docx(order, device, graph_image_dataurl=graph_image)

        # 写入内存并返回
        buf = BytesIO()
        doc.save(buf)
        buf.seek(0)

        # 文件名：避免同名覆盖
        # - 加入工单ID
        # - 加入微秒级时间戳（同一秒内多次导出也不会重名）
        ts = datetime.now().strftime('%Y%m%d_%H%M%S_%f')
        safe_device = (device.device_id or 'device').replace('/', '_').replace('\\', '_').replace(':', '_')
        filename_cn = f"工单_{safe_device}_ID{int(order.id)}_{ts}.docx"

        # 关键：HTTP 响应头必须是 ASCII/latin-1 可编码内容
        # 如果直接把中文写进 Content-Disposition: filename="..."，可能导致后端在发送响应时抛异常，
        # 浏览器侧表现为 “Failed to fetch”（连接被中断，拿不到响应）。
        # 因此这里用 ASCII 回退名作为 download_name，并通过 filename* / 自定义头传递中文文件名。
        ascii_device = re.sub(r'[^A-Za-z0-9._-]+', '_', safe_device).strip('_') or 'device'
        filename_ascii = f"workorder_{ascii_device}_ID{int(order.id)}_{ts}.docx"

        resp = send_file(
            buf,
            as_attachment=True,
            download_name=filename_ascii,
            mimetype='application/vnd.openxmlformats-officedocument.wordprocessingml.document'
        )

        # 兼容性说明：
        # - filename= 放 ASCII（避免后端编码异常）
        # - filename*=UTF-8''... 放中文（标准写法）
        # - X-EdgeWind-Filename 额外给前端用（URL 编码，ASCII 安全），确保前端下载名一定包含中文
        quoted_cn = quote(filename_cn)
        resp.headers['Content-Disposition'] = f"attachment; filename=\"{filename_ascii}\"; filename*=UTF-8''{quoted_cn}"
        resp.headers['X-EdgeWind-Filename'] = quoted_cn
        return resp

    except Exception as e:
        logger.error(f"[workorder/export] 导出失败: {e}")
        import traceback
        logger.error(traceback.format_exc())
        return jsonify({'success': False, 'error': str(e)}), 500
