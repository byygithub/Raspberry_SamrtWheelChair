#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
小程序控制模块
"""

import paho.mqtt.client as mqtt
import time
import json
import base64
import hmac
import threading
from urllib.parse import quote
from config import PRODUCT_ID, DEVICE_NAME, ACCESS_KEY, MQTT_SERVER, MQTT_PORT
from utils.message_queue import msg_queue, MessageType
from utils.logger import get_logger

logger = get_logger(__name__)


class OneNetControl:
    """OneNet平台控制"""
    def __init__(self):
        self.running = False
        self.thread = None
        self.client = None
        self.last_heartbeat_time = 0
        self.heartbeat_interval = 10  # 心跳数据上传间隔（秒）
        self.connected = False  # 标记是否已连接

    def calculate_token(self, product_id, access_key):
        """计算token"""
        return "version=2018-10-31&res=products%2FH9At1TTBP4%2Fdevices%2Fclient&et=1900800000&method=md5&sign=CjzWJ2qR1SBv1wEqdS83dw%3D%3D"

    def on_connect(self, client, userdata, flags, rc):
        """连接回调函数"""
        logger.debug(f"Connect result: {rc}")
        rc_codes = {
            0: "Connection successful",
            1: "Connection refused - incorrect protocol version",
            2: "Connection refused - invalid client identifier",
            3: "Connection refused - server unavailable",
            4: "Connection refused - bad username or password",
            5: "Connection refused - not authorized"
        }
        if rc in rc_codes:
            logger.debug(f"Connection result: {rc_codes[rc]}")
        else:
            logger.error(f"Connection failed with unknown code {rc}")
        
        if rc == 0:
            if not self.connected:
                logger.info("设备已在线！")
                self.connected = True
            # 订阅指令主题
            client.subscribe(f"$sys/{PRODUCT_ID}/{DEVICE_NAME}/thing/property/set")
            client.subscribe(f"$sys/{PRODUCT_ID}/{DEVICE_NAME}/#")
        else:
            self.connected = False
            logger.error(f"Connection failed with code {rc}")

    def on_message(self, client, userdata, msg):
        """消息接收回调函数"""
        logger.info(f"Topic: {msg.topic}")
        logger.info(f"Message: {msg.payload.decode()}")
        
        try:
            payload = json.loads(msg.payload.decode())
            if 'params' in payload and 'Direction_Control' in payload['params']:
                desired_dir = payload['params']['Direction_Control']
                logger.info(f"收到小程序指令: Direction_Control={desired_dir}")
                # 上报属性 → 平台会变
                self.post_property(client, desired_dir)
                # 回复平台
                reply_topic = f"$sys/{PRODUCT_ID}/{DEVICE_NAME}/thing/property/set_reply"
                reply_data = {
                    "id": payload.get("id", "1"),
                    "code": 200,
                    "msg": "success"
                }
                client.publish(reply_topic, json.dumps(reply_data))
                logger.info("已回复平台 + 已上报属性")
                
                # 发送控制命令到消息队列
                if desired_dir in ['F', 'B', 'L', 'R', 'S', 'A', 'D']:
                    msg_queue.put(MessageType.CONTROL_COMMAND, desired_dir)
                    logger.info(f"发送控制命令: {desired_dir}")
        except Exception as e:
            logger.error(f"处理消息失败: {e}")

    def post_property(self, client, actual_dir):
        """上报属性到平台"""
        post_topic = f"$sys/{PRODUCT_ID}/{DEVICE_NAME}/thing/property/post"
        ts = int(time.time()*1000)  # 毫秒级时间戳
        post_data = {
            "id": str(ts),  # 满足数字字符串、长度≤13位
            "version": "1.0",
            "params": {
                "Direction_Control": {
                    "value": actual_dir,  # 包裹value字段
                    "time": ts  # 增加time时间戳
                }
            }
        }
        client.publish(post_topic, json.dumps(post_data))
        logger.info(f"上报成功，平台属性已更新: Direction_Control={actual_dir}")

    def post_heartbeat_data(self, client, heartbeat_data):
        """上报心跳数据到平台"""
        post_topic = f"$sys/{PRODUCT_ID}/{DEVICE_NAME}/thing/property/post"
        ts = int(time.time()*1000)  # 毫秒级时间戳
        post_data = {
            "id": str(ts),  # 满足数字字符串、长度≤13位
            "version": "1.0",
            "params": {
                "HeartBeat": {
                    "value": int(heartbeat_data["heart_rate"]),  # 心跳数据，转换为int32
                    "time": ts  # 增加time时间戳
                },
                "Blood_Pressure": {
                    "value": int(heartbeat_data['blood_pressure_high']),  # 血压数据，取收缩压，转换为int32
                    "time": ts  # 增加time时间戳
                },
                "temperature": {
                    "value": int(heartbeat_data["temperature"]),  # 温度数据，转换为int32
                    "time": ts  # 增加time时间戳
                }
            }
        }
        client.publish(post_topic, json.dumps(post_data))
        logger.info(f"上报心跳数据成功: HeartBeat={heartbeat_data['heart_rate']}, Blood_Pressure={heartbeat_data['blood_pressure_high']}, temperature={heartbeat_data['temperature']}")

    def run(self):
        """运行OneNet控制"""
        self.running = True
        logger.info("OneNet控制启动")

        # 初始化MQTT客户端
        client = mqtt.Client(client_id=DEVICE_NAME, protocol=mqtt.MQTTv311)
        client.on_connect = self.on_connect
        client.on_message = self.on_message
        token = self.calculate_token(PRODUCT_ID, ACCESS_KEY)
        client.username_pw_set(PRODUCT_ID, token)

        try:
            client.connect(MQTT_SERVER, MQTT_PORT, 60)
            logger.info("连接成功，保持永久在线...")
            client.loop_start()  # 启动MQTT循环

            while self.running:
                # 从消息队列获取心跳数据
                msg = msg_queue.get(MessageType.HEARTBEAT_DATA, block=False)
                if msg and msg.msg_type == MessageType.HEARTBEAT_DATA:
                    # 上报心跳数据到平台
                    self.post_heartbeat_data(client, msg.data)
                
                time.sleep(0.1)

        except Exception as e:
            logger.error(f"OneNet控制错误: {e}")
        finally:
            if client:
                client.loop_stop()
                client.disconnect()
            logger.info("OneNet控制已停止")

    def start(self):
        """启动OneNet控制线程"""
        self.thread = threading.Thread(target=self.run, daemon=True)
        self.thread.start()
        logger.info("OneNet控制线程已启动")

    def stop(self):
        """停止OneNet控制"""
        self.running = False
        if self.thread:
            self.thread.join(timeout=5)
        logger.info("OneNet控制已停止")


if __name__ == "__main__":
    onenet_control = OneNetControl()
    onenet_control.start()
    
    try:
        while True:
            time.sleep(1)
    except KeyboardInterrupt:
        onenet_control.stop()
