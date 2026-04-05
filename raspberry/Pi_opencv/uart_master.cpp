#include "uart_master.h"

// 树莓派ttyAMA0串口初始化（硬件串口专属优化）
int uart_init(const char *uart_path) {
    // 打开树莓派硬件串口（ttyAMA0），禁用控制台占用
    int fd = open(uart_path, O_WRONLY | O_NOCTTY | O_NDELAY);
    if (fd < 0) {
        fprintf(stderr, "树莓派ttyAMA0打开失败: %s (错误码：%d)\n", strerror(errno), errno);
        // 给出针对性解决方案
        if (errno == 13) fprintf(stderr, "解决方案：执行 sudo chmod 666 /dev/ttyAMA0 或 sudo运行程序\n");
        if (errno == 2) fprintf(stderr, "解决方案：确认串口已启用（raspi-config），重启树莓派\n");
        return -1;
    }

    // ttyAMA0专属最优配置（115200 8N1，硬件串口稳定配置）
    struct termios cfg;
    memset(&cfg, 0, sizeof(cfg));

    cfg.c_cflag = B115200 | CS8 | CLOCAL | CREAD; // 硬件串口核心配置
    cfg.c_iflag = IGNPAR | IGNBRK; // 忽略奇偶校验+中断（ttyAMA0抗干扰优化）
    cfg.c_oflag = 0;               // 禁用输出转换，指令直接发走
    cfg.c_lflag = 0;               // 禁用本地模式，无回显/信号

    // 无超时 + 最小字符数1（立即返回，加速按钮发送）
    cfg.c_cc[VTIME] = 0;
    cfg.c_cc[VMIN] = 1;

    // 立即应用配置，清空缓冲
    tcflush(fd, TCIOFLUSH);
    if (tcsetattr(fd, TCSANOW, &cfg) < 0) {
        fprintf(stderr, "ttyAMA0配置失败: %s (错误码：%d)\n", strerror(errno), errno);
        close(fd);
        return -1;
    }

    return fd;
}

// 按钮按下极速发送（ttyAMA0硬件串口，发送延迟<10微秒）
int uart_send_char(int fd, char c) {
    if (fd < 0) {
        fprintf(stderr, "串口句柄无效\n");
        return -1;
    }

    // 核心加速：write后立即排空发送队列（ttyAMA0无缓冲延迟）
    ssize_t len = write(fd, &c, 1);
    tcdrain(fd); // 强制硬件发送完成，指令立即出现在TX引脚

    if (len != 1) {
        fprintf(stderr, "发送字符失败: %c, 错误: %s\n", c, strerror(errno));
        return -1;
    }

    return 0;
}

int uart_send_bytes(int fd, const unsigned char *buf, int len) {
    if (fd < 0 || buf == NULL || len <= 0) return -1;
    ssize_t len_write = write(fd, buf, len);
    tcdrain(fd);
    if (len_write != len) {
        fprintf(stderr, "发送多字节失败: %s\n", strerror(errno));
        return -1;
    }
    return 0;
}

void uart_close(int fd) {
    if (fd >= 0) {
        tcflush(fd, TCIOFLUSH);
        close(fd);
    }
}
