#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
启动所有模块
"""

import sys
import os
import time
import argparse

# 添加当前目录到Python路径
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))

from serial_controller import SerialController
from heartbeat_sensor import HeartbeatSensor
from integrated_control import IntegratedControl
from onenet_control import OneNetControl
from utils.logger import get_logger
from config import STARTUP_PROFILE, HEALTH_HEARTBEAT_FILE, HEALTH_HEARTBEAT_INTERVAL

logger = get_logger(__name__)

MODULE_ORDER = ["serial", "heartbeat", "integrated", "onenet"]
PROFILE_MODULES = {
    "serial_mqtt": ["serial", "onenet"],
    "plus_heartbeat": ["serial", "heartbeat", "onenet"],
    "full": MODULE_ORDER,
}


def parse_args():
    """解析命令行参数"""
    parser = argparse.ArgumentParser(description="启动轮椅系统模块")
    parser.add_argument(
        "--profile",
        choices=list(PROFILE_MODULES.keys()),
        default=STARTUP_PROFILE if STARTUP_PROFILE in PROFILE_MODULES else "full",
        help="启动配置：serial_mqtt / plus_heartbeat / full",
    )
    parser.add_argument(
        "--modules",
        help="自定义模块列表（逗号分隔）：serial,heartbeat,integrated,onenet",
    )
    return parser.parse_args()


def resolve_modules(args):
    """解析最终启动模块"""
    if args.modules:
        modules = [m.strip() for m in args.modules.split(",") if m.strip()]
    else:
        modules = PROFILE_MODULES[args.profile]

    unknown = [m for m in modules if m not in MODULE_ORDER]
    if unknown:
        raise ValueError(f"未知模块: {unknown}")

    return [m for m in MODULE_ORDER if m in modules]


def touch_heartbeat():
    """写入健康心跳文件"""
    try:
        with open(HEALTH_HEARTBEAT_FILE, "w", encoding="utf-8") as f:
            f.write(str(int(time.time())))
    except Exception as e:
        logger.warning(f"心跳文件写入失败: {e}")


def start_module(name, modules):
    """启动单个模块"""
    logger.info(f"启动{name}模块")
    modules[name].start()
    time.sleep(2)


def stop_module(name, modules):
    """停止单个模块"""
    try:
        modules[name].stop()
    except Exception as e:
        logger.error(f"停止{name}模块失败: {e}")


def main():
    """主函数"""
    args = parse_args()
    selected_modules = resolve_modules(args)
    logger.info("开始启动所有模块")
    logger.info(f"启动配置: profile={args.profile}, modules={selected_modules}")
    
    # 创建模块实例
    modules = {
        "serial": SerialController(),
        "heartbeat": HeartbeatSensor(),
        "integrated": IntegratedControl(),
        "onenet": OneNetControl(),
    }
    
    # 启动模块（顺序很重要）
    started = []
    try:
        for name in selected_modules:
            start_module(name, modules)
            started.append(name)
    except Exception as e:
        logger.error(f"模块启动失败: {e}")
        for name in reversed(started):
            stop_module(name, modules)
        return
    
    logger.info("所有模块启动完成")
    logger.info("系统已就绪，按 Ctrl+C 退出")
    
    try:
        last_heartbeat_write = 0
        while True:
            time.sleep(1)
            now = time.time()
            if now - last_heartbeat_write >= HEALTH_HEARTBEAT_INTERVAL:
                touch_heartbeat()
                last_heartbeat_write = now
    except KeyboardInterrupt:
        logger.info("收到中断信号，正在停止所有模块...")
    finally:
        # 停止模块（顺序相反）
        for name in reversed(started):
            stop_module(name, modules)
        logger.info("所有模块已停止")


if __name__ == "__main__":
    main()
