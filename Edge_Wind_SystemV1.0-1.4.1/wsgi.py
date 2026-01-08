"""
生产环境 WSGI 入口文件
用于 Gunicorn 启动服务器

使用方式：
    gunicorn --worker-class eventlet -w 4 --bind 0.0.0.0:5000 wsgi:application

或使用配置文件：
    gunicorn -c gunicorn_config.py wsgi:application
"""
import os
from app import app, socketio, init_db

# 初始化数据库
# 注意：在 Gunicorn 多 worker 模式下，每个 worker 都会执行此代码
# 这是正常的，因为每个 worker 需要自己的数据库连接
init_db()

# Gunicorn 需要这个变量来启动服务器
# application 是 Gunicorn 的标准入口点
# 对于 Flask-SocketIO，直接使用 socketio 对象
application = socketio

