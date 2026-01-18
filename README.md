# Edge_Wind_System + STM32H7 + ESP8266（完整工程归档版）

本仓库是一个“上位机 Web/桌面端 + 下位机固件”的完整工程集合，用于 **风电场直流系统故障监测与智能诊断**。

- **上位机后端 + Web 前端**：`Edge_Wind_System/`（Flask + Socket.IO + SQLite + Bootstrap/ECharts）
- **下位机固件**：`STM32H750XBH6_ESP8266_FreeRTOS_LVGL9.4.0/`（STM32H750 + ESP8266 + FreeRTOS + LVGL 9.4.0，HTTP 透传上报）
- **Windows 桌面软件工程**：`EdgeWind_Desktop/`（pywebview + PyInstaller + Inno Setup）

---

## 目录导航

- [1. 项目总体结构](#1-项目总体结构)
- [2. 一键启动（Windows：服务器开关/模拟器开关）](#2-一键启动windows服务器开关模拟器开关)
- [3. 桌面软件（EXE/安装包）](#3-桌面软件exe安装包)
- [4. 配置文件与关键环境变量（edgewind.env）](#4-配置文件与关键环境变量edgewindenv)
- [5. Web 页面与功能入口](#5-web-页面与功能入口)
- [6. 设备接入协议（ESP8266 → Flask）](#6-设备接入协议esp8266--flask)
- [7. WebSocket 实时推送机制](#7-websocket-实时推送机制)
- [8. 工单/故障/快照与导出](#8-工单故障快照与导出)
- [9. 打包构建（生成 EXE 与安装包）](#9-打包构建生成-exe-与安装包)
- [10. 常见问题与排查](#10-常见问题与排查)
- [11. 运维与维护（重置密码/清理数据）](#11-运维与维护重置密码清理数据)

---

## 1. 项目总体结构

```
Edge_Wind_System+ESP8266/
├─ README.md                       #（本文件）根目录总说明
├─ Edge_Wind_System/               # 上位机：Flask + Web 前端
│  ├─ app.py                       # Flask 程序入口（初始化、SocketIO、日志、DB、后台任务）
│  ├─ edgewind.env                 # 运行配置（端口/推送频率/降采样/数据库等）
│  ├─ requirements.txt             # Python 依赖（推荐 Python 3.11）
│  ├─ templates/                   # 页面模板：overview/monitor/faults/settings/snapshots/login
│  ├─ static/                      # 静态资源 JS
│  ├─ edgewind/
│  │  ├─ routes/                   # pages/auth/api 路由
│  │  ├─ socket_events.py          # WebSocket 事件（订阅房间、推送状态/监控数据）
│  │  ├─ models.py                 # SQLAlchemy 模型（Device/WorkOrder/DataPoint 等）
│  │  ├─ report_generator.py       # Word 工单导出（python-docx）
│  │  └─ ...
│  ├─ scripts/                     # PowerShell 控制台脚本（彩色交互式：启动/停止/状态）
│  ├─ 服务器开关.bat               # 中文入口（转发到 scripts/edgewind_ctl.bat）
│  ├─ 模拟器开关.bat               # 中文入口（转发到 scripts/sim_ctl.bat）
│  ├─ 添加防火墙规则.bat           # Windows 放行 5000 端口（需管理员权限）
│  └─ tools/reset_admin_password.py# 离线重置管理员密码
│
├─ EdgeWind_Desktop/               # Windows 桌面软件：pywebview + PyInstaller + Inno Setup
│  ├─ run_desktop.py               # 桌面入口（启动后端子进程 + 启动 UI WebView）
│  ├─ build_windows.bat            # 一键打包脚本（输出到 D:\Edge_Wind\Admin\...）
│  ├─ installer.iss                # Inno Setup 安装脚本
│  ├─ make_icon.py                 # 生成真实 .ico（解决“图标白纸”）
│  ├─ EdgeWind.ico                 # 默认源图标（可被 Admin.ico 覆盖）
│  └─ Admin.ico                    # 管理员版图标（可选）
│
├─ EdgeWind_Client/                # Windows 客户端软件：仅 WebView2，连接远程服务器
│  ├─ run_client.py                # 客户端入口（只打开服务器 URL）
│  ├─ build_client.bat             # 客户端打包脚本
│  ├─ installer_client.iss         # 客户端安装脚本
│  ├─ edgewind_client.env          # 客户端默认配置（服务器地址）
│  └─ Client.ico                   # 客户端图标（可选）
│
└─ STM32H750XBH6_ESP8266_FreeRTOS_LVGL9.4.0/        # 下位机：STM32H750 + ESP8266 + FreeRTOS + LVGL 9.4.0
   └─ MDK-ARM/STM32H750XBH6.uvprojx                 # Keil 工程入口
```

---

## 2. 一键启动（Windows：服务器开关/模拟器开关）

### 2.1 启动/停止服务器（推荐日常开发/联调）

入口脚本在：`Edge_Wind_System/服务器开关.bat`  
它会转发到：`Edge_Wind_System/scripts/edgewind_ctl.ps1`（彩色菜单）

特点（脚本内已实现）：
- 自动检测 **Python 3.11**：要求 `py -3.11 -V` 可用
- 自动创建/使用虚拟环境：`Edge_Wind_System/venv311/`
- 自动安装依赖：`pip install -r requirements.txt`
- 默认端口策略：**固定 5000**（配合硬件端口一致性）

访问地址：
- 本机：`http://localhost:5000`
- 局域网：`http://你的IPv4:5000`（先放行防火墙，见下文）

### 2.2 启动/停止模拟器（sim.py）

入口脚本在：`Edge_Wind_System/模拟器开关.bat`  
它会转发到：`Edge_Wind_System/scripts/sim_ctl.ps1`

模拟器说明：
- 用途：无硬件时演示“节点注册/心跳/故障注入/波形与频谱”
- 默认连：`http://127.0.0.1:5000`
- 可通过环境变量或参数改服务器地址：
  - 环境变量：`EDGEWIND_SERVER_URL=http://192.168.1.10:5000`
  - 参数：`python sim.py --server http://192.168.1.10:5000`

### 2.3 放行 Windows 防火墙（局域网访问必做）

脚本：`Edge_Wind_System/添加防火墙规则.bat`  
做的事：
- 检查管理员权限
- 放行 TCP 5000 入站
- 输出本机 IPv4，提示局域网访问地址

也可手动 PowerShell：

```powershell
New-NetFirewallRule -DisplayName "EdgeWind-5000" -Direction Inbound -Protocol TCP -LocalPort 5000 -Action Allow
```

> 局域网访问详细说明见：`Edge_Wind_System/docs/局域网访问指南.md`

---

## 3. 桌面软件（EXE/安装包）

桌面端本质是“把 Web 系统装进一个 Windows 软件壳里”：
- 后端：仍然是 Flask（跑在 5000）
- UI：pywebview（Windows WebView2 / EdgeChromium）

### 3.1 运行方式

- 直接运行 EXE（便携版）：`D:\Edge_Wind\Admin\dist\EdgeWind_Admin.exe`
- 或安装包安装：`D:\Edge_Wind\Admin\installer\EdgeWind_Admin_Setup.exe`

### 3.2 桌面端关键设计（与你之前遇到的坑都在这里解决）

- **后端用“子进程”运行**（不是线程）：避免长时间运行后端莫名退出导致 `127.0.0.1 拒绝连接`
- **强制 eventlet + 禁用 greendns**：避免 eventlet 在缺少 dns 依赖时回退导致 WebSocket 不可用
- **允许下载**：桌面端可正常导出工单（docx）
- **持久化 WebView2 存储**：勾选“保持登录状态”后，下次打开不再反复登录

> 桌面端入口代码：`EdgeWind_Desktop/run_desktop.py`

图标说明：
- 管理员版：放置 `EdgeWind_Desktop/Admin.ico`（安装包图标可选 `Admin_setup.ico`）
- 客户端：放置 `EdgeWind_Client/Client.ico`（安装包图标可选 `Client_setup.ico`）
- 如果安装包提示图标过大，请用较小的 `.ico` 作为 `*_setup.ico`（例如 256x256）

### 3.3 客户端版（仅 UI，不含后端/数据库）

- 适用于连接远程 EdgeWind 服务器（由管理员版提供后端服务）
- 打包脚本：`EdgeWind_Client/build_client.bat`
- 服务器地址配置：
  - 优先读取 exe 同目录 `edgewind_client.env`
  - 否则读取 `%APPDATA%\EdgeWind_Client\edgewind_client.env`
- 也可通过参数启动：`EdgeWind_Client.exe --server=http://192.168.1.10:5000`

---

## 4. 配置文件与关键环境变量（edgewind.env）

配置文件位置：`Edge_Wind_System/edgewind.env`  
加载顺序由 `Edge_Wind_System/app.py` 决定：

1) 若设置了 `EDGEWIND_ENV_FILE`，优先加载该文件  
2) 否则加载当前目录的 `edgewind.env`（推荐）  
3) 再尝试 `.env`

### 4.1 最重要的参数（建议你重点理解）

| 变量 | 作用 | 默认/当前 |
|---|---|---|
| `HOST`/`PORT` | 监听地址/端口 | `0.0.0.0:5000` |
| `DATABASE_URL` | SQLite 数据库路径 | 默认：`sqlite:///instance/wind_farm.db`（桌面版会覆盖到 exe 同目录） |
| `EDGEWIND_NODE_TIMEOUT_SEC` | 节点超时时间（秒） | 默认 60 |
| `EDGEWIND_STATUS_EMIT_HZ` | 状态推送频率（每节点 Hz） | 5 |
| `EDGEWIND_MONITOR_EMIT_HZ` | 监控推送频率（每节点 Hz） | 20 |
| `EDGEWIND_WAVEFORM_POINTS` | 波形降采样点数 | 256 |
| `EDGEWIND_SPECTRUM_POINTS` | 频谱降采样点数 | 128 |
| `EDGEWIND_LIGHT_ACTIVE_NODES` | active_nodes 是否只存轻量数据 | true（桌面版会运行时覆盖为 false） |
| `EDGEWIND_DEVICE_API_KEY` | 设备接口鉴权 Key（可选） | 空=不鉴权 |
| `FORCE_ASYNC_MODE` | SocketIO 模式：auto/eventlet/gevent/threading | 桌面版会强制 eventlet |

### 4.2 日志与数据落点

- 日志：`Edge_Wind_System/logs/edgewind.log`
- 数据库（源码/服务器开关）：`Edge_Wind_System/instance/wind_farm.db`
- 数据库（桌面 EXE）：`EdgeWind_Admin.exe` 同目录的 `instance/wind_farm.db`

---

## 5. Web 页面与功能入口

页面路由由 `edgewind/routes/pages.py` 定义（均需登录）：

- `/` → 概览 `overview.html`
- `/overview`：系统概览
- `/monitor`：实时监测（波形/FFT/节点列表/性能面板）
- `/faults`：故障管理（工单、诊断、导出）
- `/settings`：系统设置（阈值/配置/清理）
- `/snapshots`：快照与事件回放
- `/login`：登录页（`edgewind/routes/auth.py`）

---

## 6. 设备接入协议（ESP8266 → Flask）

设备侧主要走两个接口：

1) `POST /api/register`（注册）
2) `POST /api/node/heartbeat`（心跳 + 数据上报）

### 6.1 注册接口：/api/register

请求（JSON）关键字段：
- `device_id` 或 `node_id`
- `location`（允许为空，后端会回退为 device_id）
- `hw_version`（固件/硬件版本，可选）

响应：
- 200：更新已有设备
- 201：新设备注册成功

### 6.2 心跳接口：/api/node/heartbeat（核心）

固件发送的数据结构（简化示例，与你固件实现一致）：

```json
{
  "node_id": "STM32_H7_智能2",
  "status": "online",
  "fault_code": "E00",
  "channels": [
    {
      "id": 0,
      "channel_id": 0,
      "label": "直流母线(+)",
      "name": "直流母线(+)",
      "value": 375.12,
      "current_value": 375.12,
      "unit": "V",
      "waveform": [ ... ],
      "fft_spectrum": [ ... ]
    }
  ]
}
```

注意点：
- 固件侧是通过 **TCP 透传**拼 HTTP 报文发送到 `POST /api/node/heartbeat`
- 后端对 `Content-Type` 不严格：即使 header 缺失也会尝试解析 body（见 `_get_json_payload()`）
- 心跳响应可能包含下发命令（例如 `reset`），固件端会解析后执行

---

## 7. WebSocket 实时推送机制

Socket.IO 服务在 `app.py` 初始化，事件在 `edgewind/socket_events.py` 注册。

核心事件：

- `node_status_list`：客户端连接成功后，后端推送“当前在线节点摘要列表”
- `node_status_update`：全局状态更新（在线/离线/故障码/关键指标）
- `monitor_update`：订阅节点的“完整监控数据”（波形/频谱/故障信息）
- `subscribe_node` / `unsubscribe_node`：前端订阅/取消订阅某节点（房间机制 `node_<node_id>`）

> 安全策略：Socket 连接必须已登录，否则会 `disconnect()`（防止局域网被扫描连接）

---

## 8. 工单/故障/快照与导出

后端 API（节选）：

- `GET /api/faults`：故障日志
- `GET /api/work_orders`：工单列表
- `PATCH /api/work_orders/<id>`：更新工单状态
- `POST /api/workorder/export`：导出 Word 工单（`.docx`）
- `GET /api/snapshots` / `GET /api/fault_snapshots`：快照相关

工单导出实现：
- 后端：`edgewind/report_generator.py` + `send_file(as_attachment=True)`
- 前端：`templates/faults.html` 中 `exportWorkOrder()` fetch + Blob 下载
- 桌面端：已开启下载权限（pywebview `ALLOW_DOWNLOADS=True`）

---

## 9. 打包构建（生成 EXE 与安装包）

脚本（管理员版）：`EdgeWind_Desktop/build_windows.bat`  
脚本（客户端）：`EdgeWind_Client/build_client.bat`

前置条件：
- Python 3.11（推荐与运行环境一致）
- 已安装依赖：`pip install -r Edge_Wind_System/requirements.txt`
- Inno Setup（你当前路径）：`D:\Inno Setup 6\ISCC.exe`
- Windows WebView2 Runtime（Win10/11 通常自带；缺失会导致 pywebview 无法使用 edgechromium）

运行：
- 双击 `EdgeWind_Desktop/build_windows.bat`
- 双击 `EdgeWind_Client/build_client.bat`

输出位置（已改为固定 D 盘，方便归档与交付）：

- 管理员版：
  - `D:\Edge_Wind\Admin\dist\EdgeWind_Admin.exe`
  - `D:\Edge_Wind\Admin\installer\EdgeWind_Admin_Setup.exe`
- 客户端：
  - `D:\Edge_Wind\Client\dist\EdgeWind_Client.exe`
  - `D:\Edge_Wind\Client\installer\EdgeWind_Client_Setup.exe`

---

## 10. 常见问题与排查

### 10.1 端口占用

- 桌面版/服务器默认使用 5000
- 若提示端口占用，先关闭占用进程或重启（脚本/桌面端已尽量自动处理）

### 10.2 WebSocket 显示未连接/卡顿

典型原因：
- eventlet 不可用导致回退（threading/gevent）→ WebSocket 退化 → 前端轮询压力变大

排查：
- 看 `Edge_Wind_System/logs/edgewind.log` 是否出现 `使用 eventlet 异步模式` 或 `异步模式(强制): eventlet`

### 10.3 桌面端无法导出工单

已在桌面端启用下载：`ALLOW_DOWNLOADS=True`  
若仍无下载，先确认使用的是最新打包版本。

### 10.4 桌面端每次打开都要登录

已在桌面端启用持久化存储：
- `private_mode=False`
- `storage_path=%LOCALAPPDATA%\EdgeWind_Admin\webview_storage`

---

## 11. 运维与维护（重置密码/清理数据）

### 11.1 离线重置管理员密码

脚本：`Edge_Wind_System/tools/reset_admin_password.py`

示例：

```powershell
cd Edge_Wind_System
.\venv311\Scripts\python.exe .\tools\reset_admin_password.py --username "Edge_Wind" --password "Gentle9532"
```

### 11.2 清理数据接口

API：
- `POST /api/admin/cleanup_old_data`
- `POST /api/admin/clear_all_data`

（具体参数与页面操作可在 `settings.html` 中查看对应按钮/调用）

---

## 备注

本仓库为“完整工程归档”，包含上位机、桌面端、固件端；建议你在 GitHub 私有仓库中按 tag 标记每次可交付版本（例如 `v1.4.0-desktop`），方便未来回溯。

