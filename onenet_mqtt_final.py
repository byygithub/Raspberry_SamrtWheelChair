import paho.mqtt.client as mqtt
import time
import json
import base64
import hmac
from urllib.parse import quote

"""
OneNet MQTT 连接脚本

本脚本用于在树莓派上连接OneNet平台，作为设备发送数据。

使用说明：
1. 确保已安装 paho-mqtt 库：pip3 install paho-mqtt
2. 填写以下OneNet平台参数
3. 运行脚本：python3 onenet_mqtt_final.py

连接参数说明：
- PRODUCT_ID: 产品ID
- DEVICE_NAME: 设备名称
- ACCESS_KEY: 产品的access-key

MQTT服务器信息：
- MQTT_SERVER: OneNet MQTT服务器地址
- MQTT_PORT: OneNet MQTT服务器端口
"""

# OneNet平台参数
PRODUCT_ID = "H9At1TTBP4"
DEVICE_NAME = "Respberry_Pi"
ACCESS_KEY = "THNpd1BHczdnclFSd3d0Zkg3RnZRamFwZkhFcFM3S2k="

# OneNet MQTT服务器地址和端口
MQTT_SERVER = "183.230.40.96"
MQTT_PORT = 1883

# 客户端ID：设备名称
CLIENT_ID = DEVICE_NAME

# 计算token函数
def calculate_token(product_id, access_key):
    """计算OneNet平台的认证token"""
    version = '2018-10-31'
    res = 'products/%s' % product_id
    # 设置token过期时间为1小时
    et = str(int(time.time()) + 3600)
    method = 'sha1'
    # 对access_key进行decode
    key = base64.b64decode(access_key)
    # 计算sign
    org = et + '\n' + method + '\n' + res + '\n' + version
    sign_b = hmac.new(key=key, msg=org.encode(), digestmod=method)
    sign = base64.b64encode(sign_b.digest()).decode()
    # value 部分进行url编码
    sign = quote(sign, safe='')
    res = quote(res, safe='')
    # token参数拼接
    token = 'version=%s&res=%s&et=%s&method=%s&sign=%s' % (version, res, et, method, sign)
    return token

# 连接回调函数
def on_connect(client, userdata, flags, rc):
    """连接回调函数"""
    print(f"Connect result: {rc}")
    # MQTT连接代码含义
    rc_codes = {
        0: "Connection successful",
        1: "Connection refused - incorrect protocol version",
        2: "Connection refused - invalid client identifier",
        3: "Connection refused - server unavailable",
        4: "Connection refused - bad username or password",
        5: "Connection refused - not authorized"
    }
    if rc in rc_codes:
        print(f"Connection result: {rc_codes[rc]}")
    else:
        print(f"Connection failed with unknown code {rc}")
    
    if rc == 0:
        print("Connection successful!")
        # 订阅命令主题
        client.subscribe(f"$sys/{PRODUCT_ID}/{DEVICE_NAME}/cmd/#")
    else:
        print(f"Connection failed with code {rc}")
        print("请检查以下内容：")
        print("1. 产品ID和设备名称是否正确")
        print("2. token是否正确生成")
        print("3. 设备是否在OneNet平台上正确注册")
        print("4. 网络连接是否正常")

# 消息接收回调函数
def on_message(client, userdata, msg):
    """消息接收回调函数"""
    print(f"Topic: {msg.topic}")
    print(f"Message: {msg.payload.decode()}")

# 发布数据函数
def publish_data(client, data):
    """发布数据到OneNet平台"""
    # 构建数据格式
    payload = {
        "datastreams": [
            {
                "id": key,
                "datapoints": [
                    {
                        "value": value
                    }
                ]
            }
            for key, value in data.items()
        ]
    }
    # 发布数据到OneNet
    result = client.publish(f"$sys/{PRODUCT_ID}/{DEVICE_NAME}/dp", json.dumps(payload))
    print(f"Publish result: {result}")
    print(f"Published data: {json.dumps(payload)}")

# 主函数
def main():
    """主函数"""
    # 创建MQTT客户端
    client = mqtt.Client(client_id=CLIENT_ID)
    
    # 设置回调函数
    client.on_connect = on_connect
    client.on_message = on_message
    
    # 计算token
    token = calculate_token(PRODUCT_ID, ACCESS_KEY)
    print(f"Calculated token: {token}")
    
    # 设置用户名和密码（用户名使用产品ID，密码使用计算得到的token）
    client.username_pw_set(PRODUCT_ID, token)
    
    print("=" * 60)
    print("OneNet MQTT 连接测试")
    print("=" * 60)
    print(f"MQTT Server: {MQTT_SERVER}:{MQTT_PORT}")
    print(f"Client ID: {CLIENT_ID}")
    print(f"Username: {PRODUCT_ID}")
    print(f"Password: {token}")
    print("=" * 60)
    
    try:
        # 连接到OneNet MQTT服务器
        print("Connecting to OneNet MQTT server...")
        client.connect(MQTT_SERVER, MQTT_PORT, 60)
        
        # 启动客户端循环
        client.loop_start()
        
        # 等待连接成功
        time.sleep(2)
        
        # 模拟发送数据
        print("\nSending test data...")
        sensor_data = {
            "temperature": 25.5,
            "humidity": 60.0,
            "pressure": 1013.25
        }
        publish_data(client, sensor_data)
        
        # 保持连接5秒
        time.sleep(5)
        
        # 停止客户端循环
        client.loop_stop()
        client.disconnect()
        
        print("\nTest completed!")
        print("\n如果连接失败，请检查以下内容：")
        print("1. 产品ID和设备名称是否正确")
        print("2. token是否正确生成")
        print("3. 设备是否在OneNet平台上正确注册")
        print("4. 网络连接是否正常")
        print("5. OneNet平台是否支持MQTT连接")
        
    except Exception as e:
        print(f"Error: {e}")
        print("\n请检查网络连接或OneNet平台配置")

if __name__ == "__main__":
    main()