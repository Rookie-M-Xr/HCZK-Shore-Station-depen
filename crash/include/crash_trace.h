// Copyright 2026.
// License: All rights reserved.
//
// 进程崩溃堆栈。调用方安装一次，崩溃时把堆栈写到指定目录。
// 不要在其它翻译单元 include backward.hpp。

#ifndef HCZK_CRASH_TRACE_H
#define HCZK_CRASH_TRACE_H

#include <string>

namespace hczk::crash
{
// 安装成功、符号缺失等提示的级别。崩溃处理器里不会回调，避免和日志锁死锁。
enum class NoticeLevel
{
    // 信息
    Info,
    // 警告
    Warn,
    // 错误
    Error,
    // 致命
    Fatal
};

// 非崩溃路径上的一行提示。message 已格式化。
using NoticeFn = void (*)(NoticeLevel level, const char *message);

// 把调用方日志立刻刷到磁盘。只在 logCurrentStack 写完后调用。
using FlushFn = void (*)();

// 安装参数。字段都可空：目录空则用当前工作目录，日志文件名空则崩溃时不追加滚动日志。
struct InstallOptions
{
    // 崩溃文件目录，UTF-8
    std::string dump_directory_utf8;
    // 同目录滚动日志文件名。崩溃路径直接追加一行，不经过日志库
    std::string log_file_name;
    // 崩溃文件首行的产品名
    std::string product_name;
};

/***************************************************************
 * @brief 设置非崩溃路径的提示回调
 * @param notice  可空。安装结果和主动打栈时调用
 * @param flush  可空。主动打栈写完文件后调用
 * @note 必须在 install 之前设置。崩溃信号路径不调用这两个函数。
 **************************************************************/
void setNoticeLogger(NoticeFn notice, FlushFn flush = nullptr);

/***************************************************************
 * @brief 安装崩溃堆栈捕获，进程内只调用一次
 * @param options  输出目录、滚动日志文件名、产品名
 * @note 处理器一直活到进程结束。MSVC 用与 exe 同目录的 .pdb；MinGW 用同目录 .sym。
 *       崩溃文件里 #0 是崩溃点。
 **************************************************************/
bool install(const InstallOptions &options);

/***************************************************************
 * @brief 把当前线程调用栈写入崩溃文件
 * @param reason  原因说明，可空
 **************************************************************/
void logCurrentStack(const char *reason);
}  // namespace hczk::crash

#endif  // HCZK_CRASH_TRACE_H
