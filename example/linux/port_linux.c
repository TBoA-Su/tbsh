/**
 * @file port_linux.c
 * @brief Linux 平台完整移植示例
 * @version 0.1.3
 */

#include "tbsh.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <termios.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <signal.h>

/* ============================================================
 * 终端控制模块
 * ============================================================ */

static struct termios orig_termios;
static int term_initialized = 0;

static void restore_terminal(void) {
    if (term_initialized) {
        tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig_termios);
        term_initialized = 0;
    }
}

static void signal_fun(int signal_val) {
    (void) signal_val;
    restore_terminal();
}

static void setup_terminal(void) {
    if (term_initialized) return;

    if (tcgetattr(STDIN_FILENO, &orig_termios) != 0) {
        perror("tcgetattr");
        return;
    }

    struct termios raw = orig_termios;
    raw.c_lflag &= ~(ICANON | ECHO | ISIG); /* 禁用行缓冲、回显、信号 */
    raw.c_iflag &= ~(IXON | ICRNL); /* 禁用流控、CR 转换 */
    raw.c_oflag |= (OPOST | ONLCR); /* 启用输出处理，\n 转换为 \r\n */
    raw.c_cc[VMIN] = 0; /* 非阻塞读取 */
    raw.c_cc[VTIME] = 1; /* 100ms 超时 */

    if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw) != 0) {
        perror("tcsetattr");
        return;
    }

    /* 设置非阻塞 I/O */
    int flags = fcntl(STDIN_FILENO, F_GETFL, 0);
    if (flags != -1) {
        fcntl(STDIN_FILENO, F_SETFL, flags | O_NONBLOCK);
    }

    term_initialized = 1;
    atexit(restore_terminal);

    /* 处理信号，确保终端恢复 */
    signal(SIGINT, signal_fun);
    signal(SIGTERM, signal_fun);
}

/* ============================================================
 * Shell 平台接口实现（必须实现）
 * ============================================================ */

void shell_putchar(char c) {
    putchar(c);
    fflush(stdout);
}

bool shell_kbhit(void) {
    setup_terminal();
    int ch = getchar();
    if (ch != EOF) {
        ungetc(ch, stdin);
        return true;
    }
    return false;
}

char shell_getchar(void) {
    setup_terminal();
    int ch;
    while ((ch = getchar()) == EOF) {
        usleep(1000); /* 1ms 轮询间隔 */
    }
    return (char) ch;
}

/* ============================================================
 * 扩展命令实现
 * ============================================================ */

/* 系统工具命令 */
static int cmd_exit(int argc, char *argv[]) {
    (void) argc;
    (void) argv;
    restore_terminal();
    exit(0);
    return 0; /* 不会执行到这里 */
}

static int cmd_ps(int argc, char *argv[]) {
    (void) argc;
    (void) argv;
    return system("ps aux | head -20");
}

static int cmd_date(int argc, char *argv[]) {
    (void) argc;
    (void) argv;
    return system("date '+%Y-%m-%d %H:%M:%S'");
}

static int cmd_whoami(int argc, char *argv[]) {
    (void) argc;
    (void) argv;
    return system("whoami");
}

/* ============================================================
 * 初始化与主函数
 * ============================================================ */

/* 注册所有扩展命令 */
static void register_linux_commands(void) {
    shell_register("exit", cmd_exit, "exit shell");
    shell_register("quit", cmd_exit, "alias for exit");
    shell_register("ps", cmd_ps, "list processes");
    shell_register("date", cmd_date, "show date/time");
    shell_register("whoami", cmd_whoami, "show current user");
}

int main(int argc, char *argv[]) {
    (void) argc;
    (void) argv;

    /* 初始化 Shell（输出版本和提示符）*/
    shell_init();

    /* 注册扩展命令 */
    register_linux_commands();

    /* 交互模式主循环 */
    fflush(stdout);

    while (1) {
        shell_task();
        usleep(1000); /* 1ms 轮询，降低 CPU 占用 */
    }

    return 0;
}
