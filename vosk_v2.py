#!/home/szh/vosk-env/bin/python3
import os
import json
import re
import time
from pypinyin import lazy_pinyin, Style
from vosk import Model, KaldiRecognizer
import pyaudio

# 尝试导入 serial
try:
    import serial

    SERIAL_AVAILABLE = True
    print("串口库加载成功，将使用真实串口通信")
except ImportError:
    SERIAL_AVAILABLE = False
    print("未找到 pyserial 库，使用模拟串口模式")
# 语音模型路径 - 需要修改为树莓派上的路径
model_path = "/home/szh/vosk_models/vosk-model-small-cn-0.22"  # 模型路径

# 串口配置 - 树莓派专用
SERIAL_PORT = "/dev/ttyAMA0"  # 树莓派默认串口设备
# 或者使用 "/dev/ttyS0" 取决于树莓派型号
BAUD_RATE = 115200  # 根据您的要求修改为115200

# 验证模型路径
if not os.path.exists(model_path):
    print(f"错误: 路径 '{model_path}' 不存在")
    exit(1)

# 尝试加载模型
try:
    model = Model(model_path)
    print("模型加载成功!")
except Exception as e:
    print(f"模型加载失败: {e}")
    exit(1)

# 初始化串口 - 树莓派专用配置
ser = None
if SERIAL_AVAILABLE:
    try:
        # 树莓派串口配置
        ser = serial.Serial(
            port=SERIAL_PORT,
            baudrate=BAUD_RATE,
            timeout=1,
            bytesize=serial.EIGHTBITS,
            parity=serial.PARITY_NONE,
            stopbits=serial.STOPBITS_ONE
        )
        time.sleep(2)  # 等待串口初始化
        print(f"串口 {SERIAL_PORT} 打开成功，波特率: {BAUD_RATE}")
    except Exception as e:
        print(f"串口打开失败: {e}")
        print("请检查:")
        print("1. 串口设备是否正确")
        print("2. 是否已启用串口: sudo raspi-config -> Interface Options -> Serial")
        print("3. 是否有权限访问串口: sudo usermod -a -G dialout $USER")
        ser = None
else:
    print("模拟串口模式：指令将在控制台显示，不会实际发送")

# 系统状态
SYSTEM_ACTIVE = False  # 系统是否被唤醒
LAST_ACTIVE_TIME = 0  # 最后活跃时间
ACTIVE_TIMEOUT = 60  # 1分钟超时

# 🚀 优化版指令映射 - 结合两个版本的优点
command_config = {
    "前进": {
        "code": 'F',
        "pinyin": ["qian jin", "xiang qian", "wang qian"],
        "pinyin_no_tone": ["qian jin", "xiang qian", "wang qian"],
        "aliases": [
            "前进", "向前", "往前", "前近", "钱进",
            "向前走", "往前走", "前进走", "钱进走",
            "想前", "象前", "向钱", "想钱", "象钱",
            "向前进", "往前进", "前前进", "钱前进",
            "往前", "网前", "望前", "王前", "忘前",
            "往前开", "往前跑", "往前行", "往前移动"
        ],
        "keywords": ["前", "进", "向", "往", "走", "冲", "开", "跑", "行"],
        "min_length": 2,  # 新增：最小识别长度
        "require_full": False  # 新增：是否要求完整匹配
    },
    "后退": {
        "code": 'B',
        "pinyin": ["hou tui", "xiang hou", "wang hou"],
        "pinyin_no_tone": ["hou tui", "xiang hou", "wang hou"],
        "aliases": [
            "后退", "向后", "往后", "后推", "厚退",
            "向后退", "往后退", "后退退", "厚退退",
            "想后", "象后", "向后走", "往后走", "后退走",
            "往后", "网后", "望后", "王后", "忘后",
            "往后倒", "往后撤", "往后移", "往后移动",
            "倒车", "后倒", "退后", "撤退", "后撤"
        ],
        "keywords": ["后", "退", "倒", "撤", "移", "向", "往", "走"],
        "min_length": 2,
        "require_full": False
    },
    "向左": {
        "code": 'L',
        "pinyin": ["xiang zuo", "wang zuo","zuo"],
        "pinyin_no_tone": ["xiang zuo", "wang zuo","zuo"],
        "aliases": [
            "向左", "往左", "左转", "想左", "象左", "像左",
            "向左转", "往左转", "左转弯",
            "网左", "望左", "往左走", "向左走", "左走",
            "往左拐", "向左拐", "左拐", "左转弯","左"
        ],
        "keywords": ["左", "向", "往", "转", "拐", "弯", "走"],
        "min_length": 1,  # 允许单个"左"字触发
        "require_full": False
    },
    "向右": {
        "code": 'R',
        "pinyin": ["xiang you", "wang you"],
        "pinyin_no_tone": ["xiang you", "wang you"],
        "aliases": [
            "向右", "往右", "右转", "想右", "象右", "像右",
            "向右转", "往右转", "右转弯",
            "网右", "望右", "往右走", "向右走", "右走",
            "往右拐", "向右拐", "右拐", "右转弯"
        ],
        "keywords": ["右", "向", "往", "转", "拐", "弯", "走"],
        "min_length": 1,
        "require_full": False
    },
    "停止": {
        "code": 'S',
        "pinyin": ["ting zhi", "ting"],
        "pinyin_no_tone": ["ting zhi", "ting","ding"],
        "aliases": [
            "停止", "停下", "停", "停至", "亭止",
            "停止运动", "停下来", "停一下", "暂停", "停车", "停住",
            "停止前进", "停止移动", "停止运行"
        ],
        "keywords": ["停", "止", "下", "住", "车", "暂"],
        "min_length": 1,
        "require_full": False
    },
    "加速": {
        "code": 'A',
        "pinyin": ["jia su"],
        "pinyin_no_tone": ["jia su"],
        "aliases": ["加速", "快点", "加快", "家速", "加素", "加速前进", "快一点", "加快速度"],
        "keywords": ["加", "速", "快", "点", "速", "度"],
        "min_length": 2,
        "require_full": False
    },
    "减速": {
        "code": 'D',
        "pinyin": ["jian su"],
        "pinyin_no_tone": ["jian su"],
        "aliases": ["减速", "慢点", "减慢", "减素", "减速前进", "慢一点", "降低速度"],
        "keywords": ["减", "速", "慢", "点", "降", "低"],
        "min_length": 2,
        "require_full": False
    },
    "开始": {
        "code": None,
        "pinyin": ["kai shi"],
        "pinyin_no_tone": ["kai shi"],
        "aliases": [
            "开始", "启动", "开", "开使", "凯始",
            "开始运动", "开始前进", "启动前进", "出发", "起步", "开车",
            "开启", "开动", "启动运行", "开始运行","还是"
        ],
        "keywords": ["开", "始", "启", "动", "发", "步", "车"],
        "min_length": 1,
        "require_full": False
    }
}

# 预编译正则表达式
chinese_pattern = re.compile(r'[\u4e00-\u9fff]+')
tone_pattern = re.compile(r'\d')

# 预计算所有指令的拼音变体
for cmd, config in command_config.items():
    config["pinyin_compact"] = [p.replace(' ', '') for p in config["pinyin_no_tone"]]
    config["pinyin_keywords"] = [p.replace(' ', '') for p in config["pinyin_no_tone"]]

# 初始化识别器
recognizer = KaldiRecognizer(model, 16000)

# 音频输入
p = pyaudio.PyAudio()
stream = p.open(format=pyaudio.paInt16, channels=1, rate=16000, input=True, frames_per_buffer=2048)  # 减小缓冲区
stream.start_stream()

print("\n" + "=" * 60)
print("🎤 语音识别系统 - 高精度低延迟版")
print("=" * 60)
print("🔊 系统状态: 等待唤醒")
print("💡 说'开始'或'启动'来激活系统")
print("📋 支持指令: 前进(F), 后退(B), 向左(L), 向右(R), 停止(S)")
print("=" * 60 + "\n")


# 🚀 防重复触发管理器
class CommandManager:
    def __init__(self):
        self.last_command = None
        self.last_command_time = 0
        self.last_command_text = ""
        self.command_count = 0
        self.COMMAND_COOLDOWN = 0.5  # 500ms冷却时间
        self.SAME_TEXT_THRESHOLD = 0.8  # 文本相似度阈值

    def should_send(self, command_code, text=""):
        """检查是否应该发送指令"""
        current_time = time.time()

        # 1. 冷却时间检查
        if self.last_command == command_code:
            time_diff = current_time - self.last_command_time
            if time_diff < self.COMMAND_COOLDOWN:
                return False, f"冷却中({time_diff:.2f}s)"

        # 2. 文本相似度检查（防止同一句话重复触发）
        if text and self.last_command_text:
            similarity = self.text_similarity(text, self.last_command_text)
            if similarity > self.SAME_TEXT_THRESHOLD:
                return False, f"文本相似度高({similarity:.2f})"

        return True, "可以发送"

    def text_similarity(self, text1, text2):
        """计算文本相似度（简单版）"""
        if not text1 or not text2:
            return 0
        # 使用集合计算Jaccard相似度
        set1 = set(text1)
        set2 = set(text2)
        intersection = len(set1.intersection(set2))
        union = len(set1.union(set2))
        return intersection / union if union > 0 else 0

    def record_sent(self, command_code, text=""):
        """记录已发送的指令"""
        self.last_command = command_code
        self.last_command_time = time.time()
        self.last_command_text = text
        self.command_count += 1


# 初始化命令管理器
cmd_manager = CommandManager()

# 缓存最近处理的文本
recent_texts = []
MAX_RECENT_TEXTS = 10

# 拼音缓存
pinyin_cache = {}


def get_cached_pinyin(text):
    """获取缓存的拼音"""
    if text in pinyin_cache:
        return pinyin_cache[text]

    pinyin_list = lazy_pinyin(text, style=Style.TONE3)
    pinyin_str = ' '.join(pinyin_list)
    pinyin_no_tone = tone_pattern.sub('', pinyin_str)
    pinyin_cache[text] = pinyin_no_tone
    return pinyin_no_tone


def levenshtein_distance(s1, s2):
    """计算编辑距离"""
    if len(s1) < len(s2):
        return levenshtein_distance(s2, s1)
    if len(s2) == 0:
        return len(s1)

    previous_row = list(range(len(s2) + 1))
    for i, c1 in enumerate(s1):
        current_row = [i + 1]
        for j, c2 in enumerate(s2):
            insertions = previous_row[j + 1] + 1
            deletions = current_row[j] + 1
            substitutions = previous_row[j] + (c1 != c2)
            current_row.append(min(insertions, deletions, substitutions))
        previous_row = current_row

    return previous_row[-1]


def smart_match(text, is_partial=False):
    """智能匹配指令（结合多种策略）"""
    if not text:
        return None, None, 0

    # 🚀 策略1：快速直接匹配（适用于部分结果）
    chinese_matches = chinese_pattern.findall(text)
    if not chinese_matches:
        return None, None, 0

    chinese_text = ''.join(chinese_matches)

    # 检查长度要求
    if len(chinese_text) < 1:
        return None, None, 0

    # 🚀 策略2：检查最近处理过的文本
    if chinese_text in recent_texts:
        return None, None, 0
    recent_texts.append(chinese_text)
    if len(recent_texts) > MAX_RECENT_TEXTS:
        recent_texts.pop(0)

    best_match = None
    best_code = None
    best_score = 0

    # 🚀 策略3：直接文本匹配（最高优先级）
    for command, config in command_config.items():
        # 检查最小长度
        if len(chinese_text) < config.get("min_length", 1):
            continue

        # 检查是否直接匹配
        if command in chinese_text:
            return command, config["code"], 1.0

        # 检查别名
        for alias in config["aliases"]:
            if alias in chinese_text:
                score = min(0.95, len(alias) / len(chinese_text) * 0.95)
                return command, config["code"], score

    # 🚀 策略4：关键词匹配
    for command, config in command_config.items():
        if len(chinese_text) < config.get("min_length", 1):
            continue

        # 计算关键词匹配分数
        keyword_score = 0
        for keyword in config["keywords"]:
            if keyword in chinese_text:
                keyword_score += 0.1

        if keyword_score > 0:
            match_score = min(0.8, keyword_score)
            if match_score > best_score:
                best_score = match_score
                best_match = command
                best_code = config["code"]

    # 🚀 策略5：拼音匹配（用于完整结果）
    if not is_partial and best_score < 0.7:
        text_pinyin = get_cached_pinyin(chinese_text)

        for command, config in command_config.items():
            if len(chinese_text) < config.get("min_length", 1):
                continue

            # 拼音匹配
            for pinyin_variant in config["pinyin_no_tone"]:
                distance = levenshtein_distance(text_pinyin, pinyin_variant)
                max_len = max(len(text_pinyin), len(pinyin_variant))
                score = 1 - (distance / max_len) if max_len > 0 else 0

                # 对于需要完整匹配的指令，提高阈值
                if config.get("require_full", False) and score < 0.8:
                    continue

                if score > best_score:
                    best_score = score
                    best_match = command
                    best_code = config["code"]

    return best_match, best_code, best_score


def send_serial_command_smart(command_code, text=""):
    """智能发送串口指令"""
    should_send, reason = cmd_manager.should_send(command_code, text)

    if not should_send:
        print(f"⏳ 跳过: {reason}")
        return False

    if ser and ser.is_open:
        try:
            ser.write(command_code.encode('ascii'))
            ser.flush()
            print(f"📤 发送: '{command_code}' (识别: '{text[:20]}...')")
            cmd_manager.record_sent(command_code, text)
            return True
        except Exception as e:
            print(f"❌ 串口发送失败: {e}")
            return False
    else:
        print(f"💻 模拟发送: '{command_code}' (识别: '{text[:20]}...')")
        cmd_manager.record_sent(command_code, text)
        return True


def check_timeout():
    """检查系统是否超时"""
    global SYSTEM_ACTIVE, LAST_ACTIVE_TIME
    current_time = time.time()

    if SYSTEM_ACTIVE and (current_time - LAST_ACTIVE_TIME > ACTIVE_TIMEOUT):
        SYSTEM_ACTIVE = False
        print("\n⏰ 系统已超时，请说'开始'重新唤醒")
        return True
    return False


def update_active_time():
    """更新最后活跃时间"""
    global LAST_ACTIVE_TIME
    LAST_ACTIVE_TIME = time.time()


# 主循环 - 🚀 使用Partial Result实现低延迟
print("🎤 等待语音指令...")

try:
    partial_count = 0
    full_count = 0

    while True:
        # 检查超时
        check_timeout()

        # 读取音频数据
        data = stream.read(2048, exception_on_overflow=False)

        if len(data) == 0:
            continue

        # 🚀 处理完整结果（主要用于唤醒词和确认）
        if recognizer.AcceptWaveform(data):
            result = json.loads(recognizer.Result())
            text = result.get('text', '').strip()

            if text:
                full_count += 1
                print(f"\n📝 完整结果: '{text}'")

                # 使用智能匹配
                command, command_code, score = smart_match(text, is_partial=False)

                if command and score > 0.5:
                    print(f"🎯 匹配: {command} (置信度: {score:.2f})")

                    if command == "开始":
                        SYSTEM_ACTIVE = True
                        update_active_time()
                        print("✅ 系统已唤醒！")
                    elif SYSTEM_ACTIVE and command_code:
                        if send_serial_command_smart(command_code, text):
                            update_active_time()

        # 🚀 关键：处理部分结果（实时响应）
        else:
            partial_result = json.loads(recognizer.PartialResult())
            text = partial_result.get('partial', '').strip()

            if text and len(text) >= 2:  # 至少2个字符才处理
                partial_count += 1

                # 实时显示识别结果（调试用）
                if partial_count % 5 == 0:
                    print(f"🔍 实时识别: '{text}'")

                # 只在系统激活时处理运动指令
                if SYSTEM_ACTIVE:
                    # 快速匹配（使用简化的匹配逻辑，提高速度）
                    chinese_matches = chinese_pattern.findall(text)
                    if chinese_matches:
                        chinese_text = ''.join(chinese_matches)

                        # 快速检查常用指令
                        if "前进" in chinese_text or "向前" in chinese_text or "往前" in chinese_text:
                            send_serial_command_smart('F', text)
                            update_active_time()
                        elif "后退" in chinese_text or "向后" in chinese_text or "往后" in chinese_text:
                            send_serial_command_smart('B', text)
                            update_active_time()
                        elif "向左" in chinese_text or "往左" in chinese_text or "左转" in chinese_text:
                            send_serial_command_smart('L', text)
                            update_active_time()
                        elif "向右" in chinese_text or "往右" in chinese_text or "右转" in chinese_text:
                            send_serial_command_smart('R', text)
                            update_active_time()
                        elif "停止" in chinese_text or "停下" in chinese_text or "停车" in chinese_text:
                            send_serial_command_smart('S', text)
                            update_active_time()

                # 检查唤醒词（即使系统未激活）
                elif not SYSTEM_ACTIVE and ("开始" in text or "启动" in text):
                    SYSTEM_ACTIVE = True
                    update_active_time()
                    print(f"\n✅ 系统唤醒！识别到: '{text}'")

except KeyboardInterrupt:
    print("\n🛑 程序被用户中断")
except Exception as e:
    print(f"\n❌ 程序运行出错: {e}")
    import traceback

    traceback.print_exc()
finally:
    # 清理资源
    print("\n🧹 正在清理资源...")
    if ser and ser.is_open:
        ser.close()
        print("🔌 串口已关闭")
    stream.stop_stream()
    stream.close()
    p.terminate()
    print("🎤 音频设备已关闭")
    print(f"\n📊 最终统计:")
    print(f"  部分结果: {partial_count}次")
    print(f"  完整结果: {full_count}次")
    print(f"  指令发送: {cmd_manager.command_count}次")
    print("✅ 程序已安全退出")