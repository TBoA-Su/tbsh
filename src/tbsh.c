/***
 *         ,----,
 *       ,/   .`|                                  ,--,
 *     ,`   .'  :     ,---,.    .--.--.          ,--.'|
 *   ;    ;     /   ,'  .'  \  /  /    '.     ,--,  | :
 * .'___,/    ,'  ,---.' .' | |  :  /`. /  ,---.'|  : '
 * |    :     |   |   |  |: | ;  |  |--`   |   | : _' |
 * ;    |.';  ;   :   :  :  / |  :  ;_     :   : |.'  |
 * `----'  |  |   :   |    ;   \  \    `.  |   ' '  ; :
 *     '   :  ;   |   :     \   `----.   \ '   |  .'. |
 *     |   |  '   |   |   . |   __ \  \  | |   | :  | '
 *     '   :  |   '   :  '; |  /  /`--'  / '   : |  : ;
 *     ;   |.'    |   |  | ;  '--'.     /  |   | '  ,/
 *     '---'      |   :   /     `--'---'   ;   : ;--'
 *                |   | ,'                 |   ,/
 *                `----'                   '---'
 *
 */

#include "tbsh.h"

#define SHELL_VERSION_STR "tbsh v0.1.0"

/* ============================================================
 * 字符串工具（零依赖实现）
 * ============================================================ */

static int shell_strlen(const char *s) {
    int len = 0;
    while (*s++)
        len++;
    return len;
}

static int shell_strcmp(const char *s1, const char *s2) {
    while (*s1 && (*s1 == *s2)) {
        s1++;
        s2++;
    }
    return *(unsigned char *) s1 - *(unsigned char *) s2;
}

static int shell_strncmp(const char *s1, const char *s2, size_t n) {
    while (n && *s1 && (*s1 == *s2)) {
        s1++;
        s2++;
        n--;
    }
    return n ? (*(unsigned char *) s1 - *(unsigned char *) s2) : 0;
}

static char *shell_strchr(const char *s, int c) {
    while (*s) {
        if (*s == (char) c)
            return (char *) s;
        s++;
    }
    return NULL;
}

static void shell_strcpy(char *dst, const char *src) {
    while ((*dst++ = *src++));
}

static char *shell_strncpy(char *dst, const char *src, size_t n) {
    char *d = dst;
    while (n && (*d++ = *src++))
        n--;
    while (n--)
        *d++ = '\0';
    return dst;
}

static void shell_memset(void *s, int c, size_t n) {
    unsigned char *p = (unsigned char *) s;
    while (n--)
        *p++ = (unsigned char) c;
}

/* ============================================================
 * Shell 上下文
 * ============================================================ */

typedef struct {
    /* 命令缓冲区 */
    char buf[SHELL_CMD_BUF_SIZE];
    uint8_t pos;

    /* 解析后的参数 */
    char *argv[SHELL_ARGC_MAX + 1]; /* 预留 argv[argc] = NULL */
    uint8_t argc;

    /* 命令表 */
    shell_cmd_t cmds[SHELL_CMD_MAX];
    uint8_t cmd_count;

    /* 状态标志 */
    bool echo_enabled;
    bool script_mode;
} shell_context_t;

static shell_context_t shell_ctx;

/* ============================================================
 * 输出工具函数
 * ============================================================ */

void shell_puts(const char *s) {
    while (*s)
        shell_putchar(*s++);
}

void shell_println(const char *s) {
    shell_puts(s);
    shell_putchar('\r');
    shell_putchar('\n');
}

void shell_show_prompt(void) {
    shell_putchar('\r');
    shell_puts(SHELL_PROMPT);
}

void shell_show_banner(void) {
    SHELL_BANNER();
}

void shell_show_unknown_cmd(const char *cmd) {
    shell_putchar('\r');
    shell_puts("Unknown cmd: ");
    shell_println(cmd);
}

static void shell_clear_line(void) {
    shell_putchar('\x1B');
    shell_putchar('[');
    shell_putchar('2');
    shell_putchar('K');
    shell_putchar('\r');
}

/* ============================================================
 * 命令解析与执行核心
 * ============================================================ */

/* 解析命令行到 argc/argv */
static void shell_parse_line(char *line) {
    shell_ctx.argc = 0;
    char *p = line;

    while (*p && shell_ctx.argc < SHELL_ARGC_MAX) {
        while (*p == ' ' || *p == '\t')
            p++;
        if (!*p)
            break;

        shell_ctx.argv[shell_ctx.argc++] = p;

        while (*p && *p != ' ' && *p != '\t')
            p++;
        if (*p)
            *p++ = '\0';
    }

    /* 确保 argv[argc] 为 NULL，符合 C 标准 argv 约定 */
    shell_ctx.argv[shell_ctx.argc] = NULL;
}

/* 执行已解析的命令 */
static int shell_do_execute(void) {
    if (shell_ctx.argc == 0)
        return 0;

    /* 查找内置命令 */
    for (uint8_t i = 0; i < shell_ctx.cmd_count; i++) {
        if (shell_strcmp(shell_ctx.argv[0], shell_ctx.cmds[i].name) == 0) {
            return shell_ctx.cmds[i].func(shell_ctx.argc, shell_ctx.argv);
        }
    }

    shell_show_unknown_cmd(shell_ctx.argv[0]);
    return -1;
}

/* ============================================================
 * 键盘输入处理
 * ============================================================ */

static void shell_key_char(char c) {
    if (shell_ctx.pos >= SHELL_CMD_BUF_SIZE - 1) return;

    /* 末尾追加 */
    shell_ctx.buf[shell_ctx.pos] = c;
    shell_ctx.pos++;

    shell_putchar(c);
}

static void shell_key_up_arrow(void) {
}

static void shell_key_down_arrow(void) {
}

static void shell_key_right_arrow(void) {
}

static void shell_key_left_arrow(void) {
}

static void shell_key_backspace(void) {
    if (shell_ctx.pos == 0) return;

    shell_ctx.pos--;
    if (shell_ctx.echo_enabled) {
        shell_putchar('\b');
        shell_putchar(' ');
        shell_putchar('\b');
    }
}

static void shell_key_delete(void) {
    shell_key_backspace();
}

static void shell_key_enter(void) {
    if (shell_ctx.echo_enabled) {
        shell_println("");
    }

    shell_ctx.buf[shell_ctx.pos] = '\0';
    shell_parse_line(shell_ctx.buf);
    shell_do_execute();

    /* 重置状态 */
    shell_ctx.pos = 0;
    shell_memset(shell_ctx.buf, 0, SHELL_CMD_BUF_SIZE);
    if (!shell_ctx.script_mode) {
        shell_show_prompt();
    }
}

static void shell_key_tab(void) {
}

static void shell_key_ctrl_c(void) {
    shell_ctx.pos = 0;
    shell_println("^C");
    shell_memset(shell_ctx.buf, 0, SHELL_CMD_BUF_SIZE);
    shell_show_prompt();
}

static void shell_key_ctrl_u(void) {
    shell_ctx.pos = 0;
    shell_clear_line();
    shell_memset(shell_ctx.buf, 0, SHELL_CMD_BUF_SIZE);
    shell_show_prompt();
}

/* ============================================================
 * 内置命令实现
 * ============================================================ */

int cmd_help(int argc, char *argv[]) {
    (void) argc;

    if (argc > 1) {
        /* 显示特定命令帮助 */
        for (uint8_t i = 0; i < shell_ctx.cmd_count; i++) {
            if (shell_strcmp(argv[1], shell_ctx.cmds[i].name) == 0) {
                shell_puts(shell_ctx.cmds[i].name);
                shell_puts(": ");
                shell_println(shell_ctx.cmds[i].help ? shell_ctx.cmds[i].help : "no help");
                return 0;
            }
        }
        shell_show_unknown_cmd(argv[1]);
        return 1;
    }

    shell_puts(SHELL_VERSION_STR);
    shell_println(" - Type 'help <cmd>' for specific command help");
    /* 显示所有命令 */
    shell_println("Commands:");
    for (uint8_t i = 0; i < shell_ctx.cmd_count; i++) {
        shell_puts("  ");
        shell_puts(shell_ctx.cmds[i].name);
        shell_puts(" - ");
        shell_println(shell_ctx.cmds[i].help ? shell_ctx.cmds[i].help : "no help");
    }
    return 0;
}

int cmd_echo(int argc, char *argv[]) {
    for (int i = 1; i < argc; i++) {
        shell_puts(argv[i]);
        if (i < argc - 1)
            shell_putchar(' ');
    }
    shell_println("");
    return 0;
}

int cmd_clear(int argc, char *argv[]) {
    (void) argc;
    (void) argv;

    /* 清屏并移动光标到左上角 */
    shell_puts("\033[2J\033[H");
    return 0;
}

/* ============================================================
 * 公共 API 实现
 * ============================================================ */

void shell_init(void) {
    shell_memset(&shell_ctx, 0, sizeof(shell_ctx));
    shell_ctx.echo_enabled = true;
    shell_ctx.pos = 0;

    /* 注册通用命令 */
    shell_register("help", cmd_help, "show help");
    shell_register("echo", cmd_echo, "echo text");
    shell_register("clear", cmd_clear, "clear screen");

    /* 显示启动信息 */
    if (!shell_ctx.script_mode) {
        shell_println("");
        shell_show_banner();
        shell_println("");
        shell_println(SHELL_VERSION_STR);
        shell_println("[help:list all commands]");
        shell_show_prompt();
    }
}

void shell_task(void) {
    if (!shell_kbhit())
        return;

    char c = shell_getchar();

    /* 处理转义序列 */
    if (c == '\x1B') {
        if (!shell_kbhit())
            return;
        char c2 = shell_getchar();
        if (c2 == '[') {
            if (!shell_kbhit())
                return;
            char c3 = shell_getchar();

            switch (c3) {
                case 'A': {
                    /* Up : \x1B[A */
                    shell_key_up_arrow();
                    return;
                }
                case 'B': {
                    /* Down : \x1B[B */
                    shell_key_down_arrow();
                    return;
                }
                case 'C': {
                    /* Right : \x1B[C */
                    shell_key_right_arrow();
                    return;
                }
                case 'D': {
                    /* Left : \x1B[D */
                    shell_key_left_arrow();
                    return;
                }
                case '3': {
                    /* Delete : \x1B[3~ */
                    if (!shell_kbhit()) return;
                    char c4 = shell_getchar();
                    if (c4 == '~') {
                        shell_key_delete();
                    }
                    return;
                }
                default: {
                    return;
                }
            }
        }
        return;
    }

    /* 处理控制字符 */
    if (c == '\r' || c == '\n') {
        /* 回车执行 */
        shell_key_enter();
    } else if (c == '\t') {
        /* Tab */
        shell_key_tab();
    } else if (c == '\b' || c == 0x7F) {
        /* 退格 */
        shell_key_backspace();
    } else if (c == 0x03) {
        /* Ctrl+C */
        shell_key_ctrl_c();
    } else if (c == 0x15) {
        /* Ctrl+U 清空行 */
        shell_key_ctrl_u();
    } else if (c >= 0x20 && c < 0x7F) {
        /* 可打印字符 */
        shell_key_char(c);
    }
}

int shell_register(const char *name, shell_cmd_func_t func, const char *help) {
    if (!name || !func || shell_ctx.cmd_count >= SHELL_CMD_MAX) {
        return -1;
    }

    /* 检查重复 */
    for (uint8_t i = 0; i < shell_ctx.cmd_count; i++) {
        if (shell_strcmp(shell_ctx.cmds[i].name, name) == 0) {
            return -1;
        }
    }

    shell_ctx.cmds[shell_ctx.cmd_count].name = name;
    shell_ctx.cmds[shell_ctx.cmd_count].func = func;
    shell_ctx.cmds[shell_ctx.cmd_count].help = help;
    shell_ctx.cmd_count++;
    return 0;
}

int shell_unregister(const char *name) {
    if (!name || shell_ctx.cmd_count == 0) {
        return -1;
    }

    for (uint8_t i = 0; i < shell_ctx.cmd_count; i++) {
        if (shell_strcmp(shell_ctx.cmds[i].name, name) == 0) {
            /* 移动后续命令 */
            for (uint8_t j = i; j + 1 < shell_ctx.cmd_count; j++) {
                shell_ctx.cmds[j] = shell_ctx.cmds[j + 1];
            }
            shell_ctx.cmd_count--;
            /* 清除最后一个元素 */
            shell_ctx.cmds[shell_ctx.cmd_count].name = NULL;
            shell_ctx.cmds[shell_ctx.cmd_count].func = NULL;
            shell_ctx.cmds[shell_ctx.cmd_count].help = NULL;
            return 0;
        }
    }
    return -1;
}

void shell_set_echo(bool enable) {
    shell_ctx.echo_enabled = enable;
}

bool shell_get_echo(void) {
    return shell_ctx.echo_enabled;
}

const char *shell_version(void) {
    return SHELL_VERSION_STR;
}
