# EdgeWind - 基于边缘计算的风电场直流系统故障监测平台

<div align="center">

![Version](https://img.shields.io/badge/version-1.2.0-blue)
![Python](https://img.shields.io/badge/python-3.8+-green)
![Flask](https://img.shields.io/badge/flask-3.0-orange)
![License](https://img.shields.io/badge/license-MIT-lightgrey)
![Issues](https://img.shields.io/github/issues/yourusername/EdgeWind)
![Stars](https://img.shields.io/github/stars/yourusername/EdgeWind)

**一个高性能、实时的工业物联网监测平台**

[English](README_EN.md) | 简体中文

[在线演示](http://demo.edgewind.example.com) · [报告问题](https://github.com/yourusername/EdgeWind/issues) · [功能建议](https://github.com/yourusername/EdgeWind/discussions)

</div>

---

## 📖 项目简介

EdgeWind 是一个专为风电场直流母线系统设计的智能故障监测与诊断平台。系统采用**边缘计算架构**，实现了从数据采集、信号处理、故障识别到工单管理的全流程自动化。

### 🎯 适用场景

- ⚡ 风力发电场直流系统监测
- 🏭 工业直流电源系统故障诊断  
- 📊 实时波形与频谱分析
- 🔧 设备预防性维护管理

### ✨ 核心特性

- 🚀 **实时监测**: 50Hz高频率数据刷新，WebSocket延迟 < 50ms
- 📊 **数据可视化**: ECharts 5实时波形与FFT频谱分析，支持交互式缩放拖拽
- 🤖 **智能诊断**: 基于知识图谱的故障诊断引擎，支持5种故障类型自动识别
- 📱 **响应式设计**: Bootstrap 5 UI，完美适配PC/平板/手机
- 🔒 **安全可靠**: 用户认证、Session管理、CORS保护、数据加密
- 📈 **高性能**: CPU占用 < 25%，内存占用 < 200MB，支持10+节点并发
- 📝 **工单管理**: 自动生成故障工单，支持Word文档导出
- 🎨 **边缘计算**: 数据在边缘节点预处理，减轻服务器压力

---

## 🖼️ 系统预览

<details>
<summary>点击查看界面截图</summary>

### 系统概览页面
![系统概览](docs/images/overview.png)

### 实时监测页面
![实时监测](docs/images/monitor.png)

### 故障管理页面
![故障管理](docs/images/faults.png)

</details>

> 📌 **提示**: 截图文件请放置在 `docs/images/` 目录下

---

## 🎬 快速开始

### 📋 系统要求

| 组件 | 最低要求 | 推荐配置 |
|-----|---------|---------|
| **操作系统** | Windows 7 / Ubuntu 18.04 / macOS 10.14 | Windows 10+ / Ubuntu 20.04+ / macOS 11+ |
| **Python** | 3.8+ | 3.10+ |
| **内存** | 2GB | 4GB+ |
| **硬盘** | 500MB | 2GB+ |
| **浏览器** | Chrome 80+ / Firefox 75+ / Edge 80+ | 最新版本 |

### 📦 依赖说明

- **Flask 3.0**: Web框架
- **Flask-SocketIO**: WebSocket实时通信
- **SQLAlchemy**: ORM数据库
- **NumPy**: 数值计算（FFT分析）
- **ECharts 5**: 前端可视化（CDN加载）

### ⚡ 快速安装（推荐新手）

```bash
# 1. 克隆项目
git clone https://github.com/yourusername/EdgeWind.git
cd EdgeWind

# 2. 创建虚拟环境（推荐）
python -m venv venv

# 激活虚拟环境
# Windows PowerShell:
.\venv\Scripts\Activate.ps1
# Linux/macOS:
source venv/bin/activate

# 3. 安装依赖
pip install -r requirements.txt

# 4. 启动服务器（首次运行会自动初始化数据库）
python app.py
```

### 🪟 Windows 一键启动（推荐）

如果你在 Windows 上使用本项目，推荐直接双击**根目录**下的中文入口脚本（最直观）：

- `服务器开关.bat`：启动/停止 EdgeWind 服务（彩色控制台）
- `模拟器开关.bat`：启动/停止 sim.py 模拟器（彩色控制台）
- `添加防火墙规则.bat`：一键放行 5000 端口（需管理员权限）
- `提交到GitHub.bat`：提交并推送（简化流程）

> 说明：脚本实现统一收纳在 `scripts/` 目录中；根目录中文 `.bat` 作为“可双击入口”，便于使用。

### 🌐 访问系统

启动成功后，在浏览器中打开：

- **本地访问**: http://localhost:5000
- **局域网访问**: http://你的IP地址:5000

**管理员登录凭据（首次启动初始化）**:
- 用户名：默认 `Edge_Wind`（可通过环境变量 `EDGEWIND_ADMIN_USERNAME` 修改）
- 密码：建议在首次启动前通过环境变量 `EDGEWIND_ADMIN_INIT_PASSWORD` 指定

> ⚠️ **安全提示**：为避免硬编码弱口令泄露，项目不再内置固定默认密码。  
> 如果你未设置 `EDGEWIND_ADMIN_INIT_PASSWORD`，系统会在首次启动时**自动生成强密码**并在日志中**仅提示一次**，请立即保存并登录后修改。

---

## ⚡ 关于 eventlet / threading 异步模式（重要）

EdgeWind 使用 Flask-SocketIO 支持实时通信。异步模式会影响并发能力与兼容性：

- **eventlet（推荐用于高并发/生产）**：I/O 并发更强，WebSocket 支持更稳定。
- **threading（推荐用于 Windows 开发/兼容优先）**：最省心，但并发上来后线程开销更大。

### 为什么你可能会看到 threading

如果你使用 **Python 3.12+（例如 3.14）**，`eventlet==0.33.3` 在该版本上可能不兼容（例如 `ssl.wrap_socket` 相关报错），系统会自动回退到 `threading`。

### 如何强制指定模式

系统支持环境变量 `FORCE_ASYNC_MODE`：

- `FORCE_ASYNC_MODE=auto`（默认）：按 `eventlet -> gevent -> threading` 顺序自动探测
- `FORCE_ASYNC_MODE=eventlet|gevent|threading`：强制模式；若不可用会直接报错，避免误判

### 在 Windows 上启用 eventlet（推荐做法）

建议安装 **Python 3.11**，并运行项目根目录脚本：

- `创建Eventlet环境(Python3.11).bat`

该脚本会创建 `venv311` 并强制以 eventlet 模式启动服务。

### 🎮 启动硬件模拟器（演示功能）

EdgeWind 包含一个功能完整的硬件数字孪生模拟器，用于演示和测试：

```bash
# 在新的终端窗口中运行
python sim.py
```

**模拟器命令**:

```bash
# 注册边缘节点
add STM32_Node_001 风机#1直流母线
add STM32_Node_002 风机#2直流母线

# 查看节点状态
status

# 注入故障（演示）
fault 1 E01   # 节点1 - 交流窜入故障
fault 2 E03   # 节点2 - 电容老化故障

# 清除故障
clear 1
clear 2

# 查看帮助
help
```

**支持的故障类型**:
- `E01`: 交流窜入（50Hz/150Hz谐波）
- `E02`: 绝缘故障（漏电流异常）
- `E03`: 电容老化（高频纹波）
- `E04`: IGBT开路（半波缺失）
- `E05`: 接地故障（电压偏移）

---

## 📊 系统架构

```
┌─────────────────────────────────────────────────────────┐
│                   前端 (Browser)                         │
│  系统概览 | 实时监测 | 故障管理 | 系统设置                │
└─────────────────────────────────────────────────────────┘
                     ↕ WebSocket/HTTP
┌─────────────────────────────────────────────────────────┐
│                 Flask 后端服务器                          │
│  API层 (REST + WebSocket)                                │
│  业务逻辑层 (故障诊断 / 工单管理)                         │
│  数据层 (SQLAlchemy ORM + SQLite)                        │
└─────────────────────────────────────────────────────────┘
                     ↕ HTTP POST (50Hz)
┌─────────────────────────────────────────────────────────┐
│            边缘节点 (sim.py)                              │
│  ADC采集 → DFT计算 → 故障特征生成 → 数据上传             │
└─────────────────────────────────────────────────────────┘
```

---

## 🎯 主要功能

### 1. 系统概览
- 📈 实时统计信息（在线节点、故障数量）
- 📊 故障类型分布饼图
- 📉 30天故障趋势图
- 🔍 边缘节点状态监控

### 2. 实时监测
- 📡 4通道同步波形显示（直流母线+/-、负载电流、漏电流）
- 📊 实时频谱分析（FFT）
- 🎚️ 交互式缩放和拖拽
- ⚡ 50Hz高频刷新

### 3. 故障管理
- 📋 故障日志列表
- 🔍 智能诊断引擎（知识图谱）
- 📄 工单自动创建
- 📤 Word文档导出

### 4. 系统设置
- ⚙️ 报警阈值配置
- 🔧 系统参数调整
- 🗑️ 数据管理（清理/导出）
- 📊 系统统计信息

---

## 🛠️ 技术栈

### 后端
- **Web框架**: Flask 3.0
- **实时通信**: Flask-SocketIO + eventlet
- **数据库**: SQLite (WAL模式) + SQLAlchemy
- **认证**: Flask-Login
- **文档生成**: python-docx

### 前端
- **UI框架**: Bootstrap 5
- **可视化**: ECharts 5
- **实时通信**: Socket.IO Client
- **语言**: Vanilla JavaScript (ES6+)

### 硬件仿真
- **采样率**: 5.12kHz
- **采样点数**: 1024点/帧
- **频谱分析**: 实时DFT (115频率bin)
- **更新频率**: 50Hz

---

## 📁 项目结构

```
EdgeWind/
├── 📄 app.py                     # Flask后端主程序 (3650行核心代码)
├── 📄 sim.py                     # 硬件数字孪生模拟器
├── 📄 wsgi.py                    # WSGI入口（生产部署）
├── 📄 requirements.txt           # Python依赖清单
├── 📄 gunicorn_config.py         # Gunicorn配置
│
├── 📁 templates/                 # Jinja2模板（前端页面）
│   ├── base.html                 # 基础布局模板
│   ├── login.html                # 登录页面
│   ├── overview.html             # 系统概览仪表板
│   ├── monitor.html              # 实时监测（波形/频谱）
│   ├── faults.html               # 故障管理与诊断
│   ├── settings.html             # 系统配置
│   └── snapshots.html            # 波形快照
│
├── 📁 static/                    # 静态资源（JS/CSS/Images）
│   ├── app.js                    # 全局JavaScript
│   └── js/
│       └── dashboard.js          # 仪表板逻辑
│
├── 📁 instance/                  # 运行时数据（SQLite数据库）
│   └── wind_farm.db              # 主数据库（自动创建）
│
├── 📁 logs/                      # 应用日志
│   └── edgewind.log              # 主日志文件
│
├── 📁 venv/                      # Python虚拟环境（建议）
│
└── 📁 docs/                      # 文档目录
    ├── README.md                 # 项目说明
    ├── QUICK_START.md            # 快速开始指南
    ├── PRODUCTION_DEPLOYMENT.md  # 生产部署指南
    ├── CHANGELOG.md              # 版本更新日志
    ├── CONTRIBUTING.md           # 贡献指南
    ├── FAQ.md                    # 常见问题
    └── images/                   # 文档图片
```

**代码统计**:
- 后端代码: ~3,650 行（Python）
- 模拟器代码: ~1,800 行（Python）
- 前端代码: ~2,500 行（HTML + JavaScript）
- 总计: ~8,000 行代码

---

## 🔧 配置说明

### 环境变量（推荐）

本项目启动时会优先加载 `edgewind.env`（项目根目录下），也支持 `.env`。
推荐做法：复制 `env.example` 为 `edgewind.env`，再按需修改。

示例（`edgewind.env`）：
```bash
SECRET_KEY=your-secret-key-here
DATABASE_URL=sqlite:///instance/wind_farm.db
HOST=0.0.0.0
PORT=5000
```

### 数据库配置

默认使用SQLite，生产环境推荐PostgreSQL：

```python
# app.py
app.config['SQLALCHEMY_DATABASE_URI'] = 'postgresql://user:pass@localhost/edgewind'
```

---

## 📚 API文档

### 主要接口

| 接口 | 方法 | 说明 |
|-----|------|------|
| `/api/node/heartbeat` | POST | 边缘节点心跳 |
| `/api/devices` | GET | 获取设备列表 |
| `/api/workorders` | GET | 获取工单列表 |
| `/api/workorder/export` | POST | 导出工单文档 |
| `/api/admin/system_info` | GET | 系统统计信息 |
| `/api/admin/reset_data` | POST | 清空历史数据 |

详细API文档请参考 [API.md](API.md)（待补充）

---

## 🔍 故障类型

| 代码 | 名称 | 特征 | 检测方法 |
|-----|------|------|---------|
| **E00** | 系统正常 | 稳定的DC波形 | 基线检测 |
| **E01** | 交流窜入 | 50Hz/150Hz谐波 | FFT频谱分析 |
| **E02** | 绝缘故障 | 漏电流异常 | 阈值检测 |
| **E03** | 电容老化 | 100Hz/200Hz纹波 | 频域特征 |
| **E04** | IGBT开路 | 半波缺失 | 波形畸变检测 |
| **E05** | 接地故障 | 电压大幅偏移 | 电压范围检测 |

---

## 🚀 性能指标

- **WebSocket延迟**: 30-50ms
- **波形刷新率**: 50Hz
- **CPU使用率**: 15-25%
- **内存占用**: 150-200MB
- **数据库响应**: 5-15ms
- **支持节点数**: 10+ (理论上限受硬件限制)

---

## 🔒 安全注意事项

⚠️ **生产环境部署前必读**：

1. **修改SECRET_KEY**: 不要使用默认值
2. **启用HTTPS**: 使用SSL证书
3. **管理员密码**: 首次启动建议设置 `EDGEWIND_ADMIN_INIT_PASSWORD`；若不设置会自动生成强密码并仅在日志提示一次
4. **限制CORS**: 不要使用`*`允许所有来源
5. **定期更新**: 及时更新依赖包

---

## 🔌 STM32H7 + ESP8266 固件（与后端对接）

### 工程位置

- STM32H750（CubeMX）：`STM32H750XBH6/.../MOBANCX.ioc`
- Keil MDK 工程：`STM32H750XBH6/.../MDK-ARM/MOBANCX.uvprojx`

### ESP8266 透传 HTTP 上报

ESP8266 相关代码在：

- `STM32H750XBH6/.../MDK-ARM/HARDWORK/ESP8266/esp8266.c`
- `STM32H750XBH6/.../MDK-ARM/HARDWORK/ESP8266/esp8266.h`

为避免把 WiFi/服务器地址等敏感信息上传 GitHub，已将配置拆分为本地文件：

- 复制 `esp8266_config.h.example` → `esp8266_config.h`（该文件已加入 `.gitignore`）

```bash
# 设备侧接口（HTTP over TCP 透传）
POST /api/register
POST /api/node/heartbeat
```

---

## 📖 使用指南

### 用户操作流程

1. **登录系统** → 输入用户名密码
2. **查看概览** → 了解整体运行状态
3. **实时监测** → 选择节点查看波形
4. **故障诊断** → 查看故障详情和建议
5. **导出工单** → 生成维修文档

### 管理员操作

1. **系统设置** → 配置报警阈值
2. **数据管理** → 清理历史数据
3. **用户管理** → 添加/删除用户（待实现）

---

## 🤝 贡献指南

欢迎贡献代码！请遵循以下步骤：

1. Fork 本项目
2. 创建特性分支 (`git checkout -b feature/AmazingFeature`)
3. 提交更改 (`git commit -m 'Add some AmazingFeature'`)
4. 推送到分支 (`git push origin feature/AmazingFeature`)
5. 开启 Pull Request

### 代码规范

- Python: PEP 8
- JavaScript: ES6+ 
- 注释: 中文（业务逻辑）+ 英文（技术细节）
- 提交信息: 中文说明

---

## 🐛 问题反馈

遇到问题或有建议？我们非常欢迎您的反馈！

### 报告Bug
- 🐛 [提交Issue](https://github.com/yourusername/EdgeWind/issues/new?template=bug_report.md)
- 📋 请包含：系统信息、错误日志、复现步骤

### 功能建议
- 💡 [功能请求](https://github.com/yourusername/EdgeWind/issues/new?template=feature_request.md)
- 💬 [讨论区](https://github.com/yourusername/EdgeWind/discussions)

### 安全问题
- 🔒 如果发现安全漏洞，请私下联系：security@example.com
- ⚠️ 请勿公开披露安全问题

### 常见问题
- 📖 查看 [FAQ.md](FAQ.md) 获取常见问题解答
- 📚 查看 [文档](docs/) 获取详细指南

---

## 📜 更新日志

### v1.4.0 (2026-01-07) - 最新版本 🎉

**新增功能**:
- ✨ 系统配置管理API（报警阈值、系统参数）
- ✨ 数据自动清理功能（支持定时/手动清理）
- ✨ 系统统计信息接口（设备数量、工单统计）
- ✨ 波形快照功能（保存关键时刻数据）

**优化改进**:
- ⚡ WebSocket性能优化（降低ping间隔至25秒）
- ⚡ 数据库查询性能提升（添加索引）
- 🎨 前端术语统一（边缘节点）
- 📝 完善日志记录系统

**Bug修复**:
- 🐛 修复工单导出文件名重复问题
- 🐛 修复长时间运行内存泄漏
- 🐛 修复多节点并发数据混乱

### v1.1.0 (2025-12-15)

- ✨ 新增知识图谱故障诊断引擎
- ✨ 新增工单Word文档导出功能
- ⚡ 数据库性能优化（启用WAL模式）
- 🎨 UI界面全面美化（Bootstrap 5）
- 📊 新增故障趋势分析图表

### v1.0.0 (2025-12-01) - 首次发布

- 🎉 EdgeWind 正式发布
- ✨ 实时监测功能（50Hz刷新）
- ✨ 5种故障类型自动识别
- ✨ 工单管理系统
- 🔒 用户认证与权限管理

查看完整更新日志：[CHANGELOG.md](CHANGELOG.md)

---

## 📄 许可证

本项目采用 MIT 许可证 - 详见 [LICENSE](LICENSE) 文件

---

## 🙏 致谢

- [Flask](https://flask.palletsprojects.com/) - Web框架
- [Bootstrap](https://getbootstrap.com/) - UI框架
- [ECharts](https://echarts.apache.org/) - 数据可视化
- [Socket.IO](https://socket.io/) - 实时通信

---

## 📞 联系方式

- **项目主页**: [GitHub](https://github.com/yourusername/EdgeWind)
- **在线文档**: [文档中心](https://edgewind.readthedocs.io)（建设中）
- **邮箱**: edgewind@example.com
- **微信群**: 扫码加入（添加后获取二维码）

---

## 💖 支持项目

如果 EdgeWind 对您有帮助，欢迎通过以下方式支持：

- ⭐ 给项目点个Star
- 🔀 Fork项目并提交PR
- 📢 分享给更多需要的人
- 🐛 报告Bug和提出建议
- 📝 完善文档和翻译

---

## 📈 项目状态

[![Build Status](https://img.shields.io/github/workflow/status/yourusername/EdgeWind/CI)](https://github.com/yourusername/EdgeWind/actions)
[![Code Coverage](https://codecov.io/gh/yourusername/EdgeWind/branch/main/graph/badge.svg)](https://codecov.io/gh/yourusername/EdgeWind)
[![Documentation Status](https://readthedocs.org/projects/edgewind/badge/?version=latest)](https://edgewind.readthedocs.io/zh_CN/latest/?badge=latest)

---

<div align="center">

### ⭐ 如果这个项目对你有帮助，请给一个Star！⭐

**Made with ❤️ by EdgeWind Team**

[⬆ 回到顶部](#edgewind---基于边缘计算的风电场直流系统故障监测平台)

</div>

