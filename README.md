# Edge_Wind_System + STM32H7 + ESP8266（一键可跑版）

本仓库包含两部分：

- **上位机后端 + Web**：`Edge_Wind_System/`（Flask + SocketIO）
- **下位机固件**：`STM32H750XBH6/`（STM32H7 + ESP8266 透传 HTTP 上报）

## Windows 上位机（推荐一键启动）

1. 进入目录 `Edge_Wind_System/`
2. 双击运行：
   - `服务器开关.bat`：启动/停止后端服务（默认端口 5000）
   - `模拟器开关.bat`：启动/停止 `sim.py`（演示/联调）
3. 如需局域网访问，先运行：`添加防火墙规则.bat`（管理员权限）

> 说明：脚本会自动创建/使用 `venv311` 并安装 `requirements.txt` 依赖（需要本机已安装 Python 3.11，且 `py -3.11 -V` 可用）。

## STM32H7 + ESP8266（WiFi/服务器配置）

- 工程位置：`STM32H750XBH6/.../MDK-ARM/MOBANCX.uvprojx`
- ESP8266 配置文件：  
  `STM32H750XBH6/.../HARDWORK/ESP8266/esp8266_config.h`

其中包含：
- `WIFI_SSID` / `WIFI_PASSWORD`
- `SERVER_IP` / `SERVER_PORT`

> 注意：ESP8266 仅支持 2.4GHz WiFi。

