# 树莓派断联/卡死排障与部署（最小可执行版）

## 1) 先判定故障类型

1. **SSH 是否可用**：若 SSH 都掉线，优先排查电源/温度/存储 I/O。  
2. **网关连通性**：`ping -c 3 <网关IP>`，判断是否网络中断。  
3. **资源状态**：查看 `top`、`free -h`、`df -h`、`vcgencmd measure_temp`。  

## 2) 分阶段启动排查

`start_all.py` 现支持分阶段启动（定位哪个模块触发卡顿）：

```bash
python3 start_all.py --profile serial_mqtt
python3 start_all.py --profile plus_heartbeat
python3 start_all.py --profile full
```

也可自定义模块：

```bash
python3 start_all.py --modules serial,onenet
python3 start_all.py --modules serial,heartbeat,onenet
python3 start_all.py --modules serial,heartbeat,integrated,onenet
```

## 3) systemd 托管

模板文件：`deploy/systemd/wheelchair.service`  

部署步骤（树莓派）：

```bash
sudo mkdir -p /var/log/wheelchair
sudo cp deploy/systemd/wheelchair.service /etc/systemd/system/
sudo systemctl daemon-reload
sudo systemctl enable wheelchair.service
sudo systemctl start wheelchair.service
sudo systemctl status wheelchair.service --no-pager
```

## 4) 日志轮转

模板文件：`deploy/logrotate/wheelchair`  

```bash
sudo cp deploy/logrotate/wheelchair /etc/logrotate.d/wheelchair
sudo logrotate -f /etc/logrotate.d/wheelchair
```

## 5) 掉线自动采集与健康检查

脚本：
- `deploy/scripts/health_check.sh`
- `deploy/scripts/capture_diagnostics.sh`

先赋权：

```bash
chmod +x deploy/scripts/*.sh
```

示例：每分钟健康检查（cron）

```bash
* * * * * HEALTH_HEARTBEAT_FILE=/tmp/wheelchair_heartbeat GATEWAY_IP=192.168.1.1 SERVICE_NAME=wheelchair.service /home/pi/Raspberry_SamrtWheelChair/raspberry/project/deploy/scripts/health_check.sh
```

手动抓现场：

```bash
SERVICE_NAME=wheelchair.service /home/pi/Raspberry_SamrtWheelChair/raspberry/project/deploy/scripts/capture_diagnostics.sh
```

## 6) 本次关键代码修复

1. **消息队列并发与串扰修复**：按消息类型分队列，避免不同消费者误取并丢弃消息。  
2. **日志轮转**：代码层启用 `RotatingFileHandler`，降低长时间运行导致日志膨胀风险。  
3. **健康心跳文件**：主进程周期写心跳文件，便于 watchdog 监控进程活性。  
