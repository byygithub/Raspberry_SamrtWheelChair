#ifndef UART_MASTER_H
#define UART_MASTER_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <termios.h>
#include <errno.h>


int uart_init(const char *uart_path);          // 仅传串口路径，无多余参数
int uart_send_char(int fd, char c);            // 专门发送字符（适配你的F/B/L/R/S）
int uart_send_bytes(int fd, const unsigned char *buf, int len); // 保留多字节接口
void uart_close(int fd);                       // 关闭串口

#endif // UART_MASTER_H
