# tbsh

一个极简、可移植的嵌入式 Shell，专为单片机和资源受限环境设计，同时完整支持 Linux 平台。

## 特性

- **零依赖**：纯 C 实现，不依赖操作系统或标准库
- **极小体积**：代码 < 5KB，RAM < 1KB
- **跨平台**：支持 Linux、STM32、ESP32、AVR 等单片机

## 目录结构

```
tbsh/
├── readme.md                   # readme
├── src                         # 源码
│   └── tbsh.c                  # 核心实现
└── inc                         # 示例
    └── tbsh.h                  # 头文件（接口定义）
```

## 配置选项

在编译时通过宏定义配置：

| 宏                             | 默认值    | 说明           |
|:------------------------------|:-------|:-------------|
| `SHELL_CMD_BUF_SIZE`          | 64     | 命令行最大长度      |
| `SHELL_ARGC_MAX`              | 8      | 命令最大参数个数     |
| `SHELL_CMD_MAX`               | 24     | 最大命令数        |
| `SHELL_PROMPT`                | "> "   | shell 提示符    |
| `SHELL_BANNER()`              | "TBoA" | shell banner |
| `SHELL_TAB_COMPLETION_ENABLE` | 1      | Tab 补全       |

## 内置命令

### 基础命令

| 命令            | 说明   | 示例                 |
|:--------------|:-----|:-------------------|
| `help [cmd]`  | 显示帮助 | `help`, `help ls`  |
| `echo <text>` | 输出文本 | `echo Hello World` |
| `clear`       | 清屏   | `clear`            |

### 自定义命令

```c
#include "tbsh.h"

// 命令处理函数
int cmd_led(int argc, char *argv[]) {
    if (argc < 2) {
        shell_println("Usage: led <on|off>");
        return 1;
    }
    
    if (strcmp(argv[1], "on") == 0) {
        HAL_GPIO_WritePin(LED_GPIO_Port, LED_Pin, GPIO_PIN_SET);
    } else {
        HAL_GPIO_WritePin(LED_GPIO_Port, LED_Pin, GPIO_PIN_RESET);
    }
    return 0;
}

int main(void) {
    shell_init();
    
    // 注册命令：名称, 函数, 帮助信息
    shell_register("led", cmd_led, "control LED on/off");
    
    while (1) {
        shell_task();  // 非阻塞，需循环调用
    }
}
```

## 移植指南

### 移植检查清单

| 项目                | 必需 | 说明       |
|:------------------|:---|:---------|
| `shell_putchar()` | 是  | 字符输出     |
| `shell_kbhit()`   | 是  | 非阻塞输入检查  |
| `shell_getchar()` | 是  | 读取字符     |
| 内存分配              | 可选 | 动态命令注册需要 |

### 必须实现的接口

```c
void shell_putchar(char c);     // 输出单个字符
bool shell_kbhit(void);          // 检查是否有输入（非阻塞）
char shell_getchar(void);        // 读取一个字符
```

## 平台移植示例

### Linux（完整参考实现）

见 `port_linux.c`，已包含：

- 终端控制（非阻塞输入）
- 常用命令（ps/date/whoami 等）

### 裸机（无 OS，直接寄存器操作）

```c
/* port_baremetal.c - 无操作系统，直接操作硬件 */

#include "tbsh.h"

/* 假设的 UART 寄存器 */
#define UART_DR     (*(volatile uint32_t *)0x40008000)
#define UART_FR     (*(volatile uint32_t *)0x40008018)
#define UART_FR_TXFF  (1 << 5)
#define UART_FR_RXFE  (1 << 4)

/* 简单的环形缓冲区 */
#define RX_BUF_SIZE 64
static volatile uint8_t rx_buf[RX_BUF_SIZE];
static volatile uint8_t rx_head = 0;
static volatile uint8_t rx_tail = 0;

/* UART 中断服务程序 */
void UART_IRQHandler(void) {
    uint8_t c = UART_DR;
    uint8_t next = (rx_head + 1) % RX_BUF_SIZE;
    if (next != rx_tail) {
        rx_buf[rx_head] = c;
        rx_head = next;
    }
}

void shell_putchar(char c) {
    while (UART_FR & UART_FR_TXFF);  /* 等待发送缓冲区空 */
    UART_DR = c;
}

bool shell_kbhit(void) {
    return rx_head != rx_tail;
}

char shell_getchar(void) {
    while (!shell_kbhit());
    uint8_t c = rx_buf[rx_tail];
    rx_tail = (rx_tail + 1) % RX_BUF_SIZE;
    return (char)c;
}

/* 可选：简单的 Flash 文件系统（如 LittleFS） */
/* 或直接使用固定地址存储 */

int main(void) {
    /* 初始化硬件 */
    SystemInit();
    UART_Init(115200);
    
    shell_init();
    
    while (1) {
        shell_task();
        /* 其他后台任务 */
    }
}
```

## 调试技巧

1. **输出重定向**：将 `shell_putchar` 同时输出到串口和屏幕
2. **命令回显**：使用 `shell_set_echo(true)` 查看输入
4. **堆栈检查**：在 `shell_task` 前后检查栈使用情况

## 交互操作

| 按键          | 功能          |
|:------------|:------------|
| `Tab`       | 命令补全 / 显示候选 |
| `Ctrl+C`    | 取消当前输入      |
| `Ctrl+U`    | 清除整行        |
| `Backspace` | 删除字符        |

## 许可证

MIT License

## 版本历史

- **v0.1.1** - 添加 Tab 补全
- **v0.1.0** - 初始版本
