import paho.mqtt.client as mqtt 
import time 
import json 
import base64 
import hmac 
from urllib.parse import quote 

# OneNet平台参数（你自己的，正确） 
PRODUCT_ID = "H9At1TTBP4" 
DEVICE_NAME = "client" 
ACCESS_KEY = "aFBEZ2FHNUdvYlFYbTNSVzlUNWRrckpTc1ZNS21LU0g=" 

# OneNet MQTT服务器地址和端口 
MQTT_SERVER = "183.230.40.96" 
MQTT_PORT = 1883 

# 客户端ID：设备名称 
CLIENT_ID = DEVICE_NAME 

# 计算token函数（你自己的，正确） 
def calculate_token(product_id, access_key): 
    return "version=2018-10-31&res=products%2FH9At1TTBP4%2Fdevices%2Fclient&et=1900800000&method=md5&sign=CjzWJ2qR1SBv1wEqdS83dw%3D%3D" 

# 连接回调函数 
def on_connect(client, userdata, flags, rc): 
    print(f"Connect result: {rc}") 
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
        print("✅ 设备已在线！") 
        # 订阅指令主题（正确） 
        client.subscribe(f"$sys/{PRODUCT_ID}/{DEVICE_NAME}/thing/property/set") 
        client.subscribe(f"$sys/{PRODUCT_ID}/{DEVICE_NAME}/#") 
    else: 
        print(f"Connection failed with code {rc}") 

# 消息接收回调函数（你自己的，完整保留） 
def on_message(client, userdata, msg): 
    print(f"\nTopic: {msg.topic}") 
    print(f"Message: {msg.payload.decode()}") 
    
    try: 
        payload = json.loads(msg.payload.decode()) 
        if 'params' in payload and 'Direction_Control' in payload['params']: 
            desired_dir = payload['params']['Direction_Control'] 
            print(f"\n=== 收到小程序指令 ===") 
            print(f"Direction_Control: {desired_dir}") 
            # 上报属性 → 平台会变 
            post_property(client, desired_dir) 
            # 回复平台 
            reply_topic = f"$sys/{PRODUCT_ID}/{DEVICE_NAME}/thing/property/set_reply" 
            reply_data = { 
                "id": payload.get("id", "1"), 
                "code": 200, 
                "msg": "success" 
            } 
            client.publish(reply_topic, json.dumps(reply_data)) 
            print("✅ 已回复平台 + 已上报属性") 
            
            # 发送控制命令到 wheelchair/cmd 主题，与 head_node.py 逻辑一致 
            if desired_dir in ['F', 'B', 'L', 'R', 'S', 'A', 'D']: 
                client.publish("wheelchair/cmd", desired_dir) 
                print(f"✅ 发送控制命令: {desired_dir}") 
    except: 
        pass 

# ===================== 【只改了这里】 ===================== 
# 上报属性到平台 —— 严格匹配OneNET官方OneJSON格式 
def post_property(client, actual_dir): 
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
    print(f"✅ 上报成功，平台属性已更新: Direction_Control={actual_dir}") 

# ========================= 【修复：永久在线】 ========================= 
def main(): 
    client = mqtt.Client(client_id=CLIENT_ID, protocol=mqtt.MQTTv311) 
    client.on_connect = on_connect 
    client.on_message = on_message 
    token = calculate_token(PRODUCT_ID, ACCESS_KEY) 
    client.username_pw_set(PRODUCT_ID, token) 

    try: 
        client.connect(MQTT_SERVER, MQTT_PORT, 60) 
        print("🔗 连接成功，保持永久在线...") 
        client.loop_forever()  # 这里修复！永久在线，不会断开 

    except Exception as e: 
        print(f"Error: {e}") 

if __name__ == "__main__": 
    main()