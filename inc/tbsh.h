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

#ifndef __TBSH_H__
#define __TBSH_H__

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {



#endif

/* ============================================================
 * 可配置宏
 * ============================================================ */

/* 缓冲区与限制 */
#ifndef SHELL_CMD_BUF_SIZE
#define SHELL_CMD_BUF_SIZE 64 /* 单行命令最大长度 */
#endif

#ifndef SHELL_ARGC_MAX
#define SHELL_ARGC_MAX 8 /* 最大参数个数 */
#endif

#ifndef SHELL_CMD_MAX
#define SHELL_CMD_MAX 24 /* 最大注册命令数 */
#endif

/* 提示符 */
#ifndef SHELL_PROMPT
#define SHELL_PROMPT "tbsh> "
#endif

#ifndef SHELL_BANNER
#define SHELL_BANNER()                                                        \
    do                                                                        \
    {                                                                         \
        shell_println("  ______  ______   ______   __  __");                  \
        shell_println(" /\\__  _\\/\\  == \\ /\\  ___\\ /\\ \\_\\ \\");       \
        shell_println(" \\/_/\\ \\/\\ \\  __< \\ \\___  \\\\ \\  __ \\");     \
        shell_println("    \\ \\_\\ \\ \\_____\\\\/\\_____\\\\ \\_\\ \\_\\"); \
        shell_println("     \\/_/  \\/_____/ \\/_____/ \\/_/\\/_/");          \
    } while (0)
#endif

/* 功能开关 */
#ifndef SHELL_TAB_COMPLETION_ENABLE
#define SHELL_TAB_COMPLETION_ENABLE 1 /* Tab 命令补全 */
#endif

/* ============================================================
 * 类型定义
 * ============================================================ */

/**
 * @brief 命令回调函数类型
 * @param argc 参数个数
 * @param argv 参数数组
 * @return 0 成功，1 失败
 */
typedef int (*shell_cmd_func_t)(int argc, char *argv[]);

/**
 * @brief 命令描述结构
 * @param name 命令名
 * @param func 执行函数
 * @param help 帮助文本
 */
typedef struct {
    const char *name; /* 命令名 */
    shell_cmd_func_t func; /* 执行函数 */
    const char *help; /* 帮助文本 */
} shell_cmd_t;

/* ============================================================
 * 平台抽象接口（必须由用户实现）
 * ============================================================ */
/**
 * @brief 输出一个字符
 * @param c 要输出的字符
 */
extern void shell_putchar(char c);

/**
 * @brief 检查是否有输入字符可读（非阻塞）
 * @return true 有输入，false 无输入
 */
extern bool shell_kbhit(void);

/**
 * @brief 获取一个输入字符
 * @return 输入的字符
 */
extern char shell_getchar(void);

/* ============================================================
 * 核心 API
 * ============================================================ */

/* 初始化与主循环 */
/**
 * @brief 初始化 Shell，注册内置命令，显示欢迎信息
 */
void shell_init(void);

/**
 * @brief 执行 Shell 任务，处理用户输入
 */
void shell_task(void);

/* 命令管理 */
/**
 * @brief 注册命令
 * @param name 命令名字符串，必须是静态/全局常量，函数只保存指针
 * @param help 帮助文本，必须是静态/全局常量，可为 NULL
 */
int shell_register(const char *name, shell_cmd_func_t func, const char *help);

/**
 * @brief 注销命令
 * @param name 命令名字符串
 * @return 0 成功，-1 失败（如命令不存在）
 */
int shell_unregister(const char *name);

/* 回显控制 */
/**
 * @brief 设置回显状态
 * @param enable true为启用，false为禁用
 */
void shell_set_echo(bool enable);

/**
 * @brief 获取回显状态
 * @return true为启用，false为禁用
 */
bool shell_get_echo(void);

/* 输出工具 */
/**
 * @brief 输出字符串
 * @param s 字符串指针
 */
void shell_puts(const char *s);

/**
 * @brief 输出字符串并换行
 * @param s 字符串指针
 */
void shell_println(const char *s);

/* 版本信息 */
/**
 * @brief 获取版本信息
 * @return 版本字符串指针
 */
const char *shell_version(void);

/* ============================================================
 * 内置命令（可直接调用或重新实现）
 * ============================================================ */

/* 通用命令 */
/**
 * @brief help 命令
 * @param argc 参数个数
 * @param argv 参数数组
 * @return 0 成功，1 失败
 */
int cmd_help(int argc, char *argv[]);

/**
 * @brief echo 命令
 * @param argc 参数个数
 * @param argv 参数数组
 * @return 0 成功，1 失败
 */
int cmd_echo(int argc, char *argv[]);

/**
 * @brief clear 命令
 * @param argc 参数个数
 * @param argv 参数数组
 * @return 0 成功，1 失败
 */
int cmd_clear(int argc, char *argv[]);

#ifdef __cplusplus
}
#endif

#endif /* __TBSH_H__ */
