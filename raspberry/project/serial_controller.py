#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
串口控制模块
"""

import serial
import time
import threading
from config import SERIAL_PORT, SERIAL_BAUD
from utils.message_queue import msg_queue, MessageType
from utils.logger import get_logger

logger = get_logger(__name__)


class SerialController:
    """串口控制器"""
    def __init__(self):
        self.ser = None
        self.running = False
        self.thread = None

    def init_serial(self):
        """初始化串口"""
        try:
            self.ser = serial.Serial(
                port=SERIAL_PORT,
                baudrate=SERIAL_BAUD,
                timeout=1,
                bytesize=serial.EIGHTBITS,
                parity=serial.PARITY_NONE,
                stopbits=serial.STOPBITS_ONE
            )
            time.sleep(2)
            logger.info(f"串口 {SERIAL_PORT} 打开成功，波特率: {SERIAL_BAUD}")
            return True
        except Exception as e:
            logger.error(f"串口打开失败: {e}")
            self.ser = None
            return False

    def send_command(self, command):
        """发送控制指令"""
        if not self.ser or not self.ser.is_open:
            logger.warning("串口未打开，无法发送指令")
            return False

        try:
            self.ser.write(command.encode('ascii'))
            self.ser.flush()
            logger.info(f"串口发送: {command}")
            return True
        except Exception as e:
            logger.error(f"串口发送失败: {e}")
            return False

    def run(self):
        """运行串口控制器"""
        self.running = True
        logger.info("串口控制器启动")

        # 初始化串口
        if not self.init_serial():
            logger.error("串口初始化失败，退出")
            self.running = False
            return

        while self.running:
            try:
                # 从消息队列获取控制指令
                msg = msg_queue.get(MessageType.CONTROL_COMMAND, block=True, timeout=1)
                if msg and msg.msg_type == MessageType.CONTROL_COMMAND:
                    command = msg.data
                    if isinstance(command, str) and len(command) == 1:
                        self.send_command(command)
            except Exception as e:
                logger.error(f"串口控制器错误: {e}")
                # 尝试重新初始化串口
                if not self.ser or not self.ser.is_open:
                    logger.info("尝试重新打开串口")
                    self.init_serial()
            time.sleep(0.1)

        # 清理
        if self.ser and self.ser.is_open:
            self.ser.close()
            logger.info("串口已关闭")

    def start(self):
        """启动串口控制器线程"""
        self.thread = threading.Thread(target=self.run, daemon=True)
        self.thread.start()
        logger.info("串口控制器线程已启动")

    def stop(self):
        """停止串口控制器"""
        self.running = False
        if self.thread:
            self.thread.join(timeout=5)
        logger.info("串口控制器已停止")


if __name__ == "__main__":
    controller = SerialController()
    controller.start()
    
    try:
        while True:
            time.sleep(1)
    except KeyboardInterrupt:
        controller.stop()
