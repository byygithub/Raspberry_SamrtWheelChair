#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
配置管理模块
"""

import os

# 串口配置
SERIAL_PORT = os.environ.get('SERIAL_PORT', '/dev/ttyAMA0')
SERIAL_BAUD = int(os.environ.get('SERIAL_BAUD', '115200'))

# 心跳传感器配置
HEARTBEAT_PORT = os.environ.get('HEARTBEAT_PORT', '/dev/ttyUSB0')
HEARTBEAT_BAUD = int(os.environ.get('HEARTBEAT_BAUD', '115200'))

# OneNet平台配置
PRODUCT_ID = os.environ.get('PRODUCT_ID', 'H9At1TTBP4')
DEVICE_NAME = os.environ.get('DEVICE_NAME', 'client')
ACCESS_KEY = os.environ.get('ACCESS_KEY', 'aFBEZ2FHNUdvYlFYbTNSVzlUNWRrckpTc1ZNS21LU0g=')
MQTT_SERVER = os.environ.get('MQTT_SERVER', '183.230.40.96')
MQTT_PORT = int(os.environ.get('MQTT_PORT', '1883'))

# 语音识别配置
VOICE_MODEL_PATH = os.environ.get('VOICE_MODEL_PATH', '/home/szh/vosk_models/vosk-model-small-cn-0.22')
AUDIO_DIR = os.environ.get('AUDIO_DIR', './Audio')

# 头部姿态检测配置
YOLO_MODEL_PATH = os.environ.get('YOLO_MODEL_PATH', '/usr/local/last.onnx')
CAMERA_ID = int(os.environ.get('CAMERA_ID', '0'))

# 消息队列配置
QUEUE_SIZE = int(os.environ.get('QUEUE_SIZE', '100'))

# 日志配置
LOG_LEVEL = os.environ.get('LOG_LEVEL', 'INFO')
LOG_FILE = os.environ.get('LOG_FILE', 'app.log')
LOG_MAX_BYTES = int(os.environ.get('LOG_MAX_BYTES', str(5 * 1024 * 1024)))
LOG_BACKUP_COUNT = int(os.environ.get('LOG_BACKUP_COUNT', '3'))

# 启动配置
STARTUP_PROFILE = os.environ.get('STARTUP_PROFILE', 'full')

# 健康检查配置
HEALTH_HEARTBEAT_FILE = os.environ.get('HEALTH_HEARTBEAT_FILE', '/tmp/wheelchair_heartbeat')
HEALTH_HEARTBEAT_INTERVAL = int(os.environ.get('HEALTH_HEARTBEAT_INTERVAL', '5'))
