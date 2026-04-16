#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
消息队列模块
"""

import queue
from enum import Enum


class MessageType(Enum):
    """消息类型"""
    CONTROL_COMMAND = "control_command"  # 控制指令
    HEARTBEAT_DATA = "heartbeat_data"    # 心跳数据
    SYSTEM_STATUS = "system_status"      # 系统状态


class Message:
    """消息类"""
    def __init__(self, msg_type, data):
        self.msg_type = msg_type
        self.data = data


class MessageQueue:
    """消息队列类"""
    def __init__(self, maxsize=100):
        self.maxsize = maxsize
        self.queues = {msg_type: queue.Queue(maxsize=maxsize) for msg_type in MessageType}

    def put(self, msg_type, data):
        """放入消息"""
        if msg_type not in self.queues:
            return False
        msg = Message(msg_type, data)
        try:
            self.queues[msg_type].put(msg, block=False)
            return True
        except queue.Full:
            return False

    def get(self, msg_type, block=True, timeout=None):
        """获取消息"""
        if msg_type not in self.queues:
            return None
        try:
            return self.queues[msg_type].get(block=block, timeout=timeout)
        except queue.Empty:
            return None

    def qsize(self):
        """获取队列大小"""
        return sum(q.qsize() for q in self.queues.values())

    def empty(self, msg_type=None):
        """检查队列是否为空"""
        if msg_type:
            if msg_type not in self.queues:
                return True
            return self.queues[msg_type].empty()
        return all(q.empty() for q in self.queues.values())


# 全局消息队列实例
msg_queue = MessageQueue()
