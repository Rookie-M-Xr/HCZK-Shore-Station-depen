// Copyright 2026.
// License: All rights reserved.

#include "crash_trace.h"

#if defined(_WIN64) || defined(__x86_64__)
#ifndef _M_X64
#define _M_X64 1
#endif
#endif

#include "backward.hpp"

#include <algorithm>
#include <cstdarg>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <filesystem>
#include <map>
#include <string>
#include <system_error>
#include <vector>

#ifdef _WIN32
#include <process.h>
#include <psapi.h>
#endif

#ifdef __GNUC__
#include <cxxabi.h>
#endif

#ifndef _WIN32
#include <unistd.h>
#endif

// MSVC 用 PDB / DbgHelp；MinGW 才读 nm 导出的 .sym
#if defined(_WIN32) && !defined(_MSC_VER)
#define SHORE_CRASH_USE_NM_SYM 1
#else
#define SHORE_CRASH_USE_NM_SYM 0
#endif

namespace hczk::crash
{
namespace
{
constexpr char kCrashFileNamePrefix[] = "crash_";
constexpr char kSymbolFileSuffix[] = ".sym";
constexpr char kPdbFileSuffix[] = ".pdb";

// 崩溃文件输出目录（UTF-8）
std::string g_dump_directory_utf8;
// 同目录滚动日志文件名。空则崩溃时不追加
std::string g_log_file_name;
// 崩溃文件首行的产品名
std::string g_product_name;
// 非崩溃路径提示。崩溃处理器不调用
NoticeFn g_notice = nullptr;
// 主动打栈后刷新调用方日志
FlushFn g_flush = nullptr;

void notice(NoticeLevel level, const char *fmt, ...)
{
    if (g_notice == nullptr || fmt == nullptr) {
        return;
    }
    char buffer[1024] = {0};
    va_list args;
    va_start(args, fmt);
    std::vsnprintf(buffer, sizeof(buffer), fmt, args);
    va_end(args);
    g_notice(level, buffer);
}

struct SymbolEntry
{
    // 链接时虚地址
    std::uint64_t vma = 0;
    // 已反修饰的函数名
    std::string name;
    // 缩短后的源文件路径
    std::string file;
    // 符号定义行号
    int line = 0;
};

struct ModuleSymbols
{
    // 磁盘 PE 的 ImageBase，不能用 ASLR 后内存里被改过的值
    std::uint64_t file_image_base = 0x140000000ull;
    // 按 vma 升序
    std::vector<SymbolEntry> entries;
};

// 模块路径（小写） -> 符号表
std::map<std::wstring, ModuleSymbols> g_symbol_tables;

std::filesystem::path utf8Path(const std::string &path_utf8)
{
    return std::filesystem::u8path(path_utf8);
}

std::string makeCrashFileName()
{
    std::time_t now = std::time(nullptr);
    std::tm local{};
#ifdef _WIN32
    localtime_s(&local, &now);
#else
    localtime_r(&now, &local);
#endif
    char stamp[32] = {0};
    std::snprintf(stamp, sizeof(stamp), "%04d%02d%02d_%02d%02d%02d",
                  local.tm_year + 1900,
                  local.tm_mon + 1,
                  local.tm_mday,
                  local.tm_hour,
                  local.tm_min,
                  local.tm_sec);
    return std::string(kCrashFileNamePrefix) + stamp + ".txt";
}

int currentProcessId()
{
#ifdef _WIN32
    return _getpid();
#else
    return static_cast<int>(::getpid());
#endif
}

FILE *openUtf8File(const std::filesystem::path &path, bool append)
{
#ifdef _WIN32
    return _wfopen(path.wstring().c_str(), append ? L"a" : L"w");
#else
    return std::fopen(path.string().c_str(), append ? "a" : "w");
#endif
}

void appendCrashMarker(const std::string &crash_path_utf8, const char *reason)
{
    if (g_dump_directory_utf8.empty() || g_log_file_name.empty()) {
        return;
    }
    const std::filesystem::path log_path =
        utf8Path(g_dump_directory_utf8) / g_log_file_name;
    FILE *fp = openUtf8File(log_path, true);
    if (fp == nullptr) {
        return;
    }
    std::fprintf(fp,
                 "[FATAL] crash stack written: %s",
                 crash_path_utf8.c_str());
    if (reason != nullptr && reason[0] != '\0') {
        std::fprintf(fp, " reason: %s", reason);
    }
    std::fprintf(fp, "\n");
    std::fflush(fp);
    std::fclose(fp);
}

std::string demangleName(const std::string &name)
{
    if (name.empty() || name == "??") {
        return {};
    }
#ifdef __GNUC__
    std::string mangled = name;
    if (mangled.size() >= 2 && mangled[0] == 'Z') {
        mangled.insert(mangled.begin(), '_');
    }
    int status = 0;
    char *demangled = abi::__cxa_demangle(mangled.c_str(), nullptr, nullptr, &status);
    if (status == 0 && demangled != nullptr) {
        std::string result = demangled;
        std::free(demangled);
        return result;
    }
#endif
    return name;
}

std::string shortenSourcePath(const std::string &raw)
{
    if (raw.empty() || raw == "??" || raw == "??:0") {
        return {};
    }
    std::error_code error;
    const std::filesystem::path canonical =
        std::filesystem::weakly_canonical(utf8Path(raw), error);
    const std::string text = error ? raw : canonical.u8string();
    auto pos = text.find("/src/");
    if (pos == std::string::npos) {
        pos = text.find("\\src\\");
    }
    if (pos != std::string::npos) {
        return text.substr(pos + 1);
    }
    pos = text.find("/communication/");
    if (pos == std::string::npos) {
        pos = text.find("\\communication\\");
    }
    if (pos != std::string::npos) {
        return text.substr(pos + 1);
    }
    return text;
}

bool shouldKeepSymbolName(const std::string &name)
{
    if (name.empty() || name[0] == '.') {
        return false;
    }
    if (name.rfind("_GLOBAL__", 0) == 0) {
        return false;
    }
    if (name == "__DTOR_LIST__" || name == "__CTOR_LIST__"
        || name == "__DTOR_LIST_END__" || name == "__CTOR_LIST_END__") {
        return false;
    }
    return true;
}

#ifdef _WIN32
std::wstring normalizeModulePath(std::wstring path)
{
    for (wchar_t &ch : path) {
        if (ch >= L'A' && ch <= L'Z') {
            ch = static_cast<wchar_t>(ch - L'A' + L'a');
        }
        if (ch == L'/') {
            ch = L'\\';
        }
    }
    return path;
}

std::uint64_t readFileImageBase(const std::wstring &exe_path)
{
    FILE *fp = _wfopen(exe_path.c_str(), L"rb");
    if (fp == nullptr) {
        return 0x140000000ull;
    }
    IMAGE_DOS_HEADER dos{};
    if (std::fread(&dos, sizeof(dos), 1, fp) != 1 || dos.e_magic != IMAGE_DOS_SIGNATURE) {
        std::fclose(fp);
        return 0x140000000ull;
    }
    if (std::fseek(fp, dos.e_lfanew, SEEK_SET) != 0) {
        std::fclose(fp);
        return 0x140000000ull;
    }
    IMAGE_NT_HEADERS64 nt{};
    if (std::fread(&nt, sizeof(nt), 1, fp) != 1 || nt.Signature != IMAGE_NT_SIGNATURE) {
        std::fclose(fp);
        return 0x140000000ull;
    }
    std::fclose(fp);
    if (nt.OptionalHeader.ImageBase == 0) {
        return 0x140000000ull;
    }
    return nt.OptionalHeader.ImageBase;
}

std::string wideToUtf8(const std::wstring &wide)
{
    if (wide.empty()) {
        return {};
    }
    const int size = WideCharToMultiByte(CP_UTF8,
                                         0,
                                         wide.c_str(),
                                         static_cast<int>(wide.size()),
                                         nullptr,
                                         0,
                                         nullptr,
                                         nullptr);
    if (size <= 0) {
        return {};
    }
    std::string utf8(static_cast<std::size_t>(size), '\0');
    WideCharToMultiByte(CP_UTF8,
                        0,
                        wide.c_str(),
                        static_cast<int>(wide.size()),
                        utf8.data(),
                        size,
                        nullptr,
                        nullptr);
    return utf8;
}

bool queryModule(void *addr, std::wstring *path, std::uint64_t *rva)
{
    HMODULE handle = nullptr;
    if (!GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS
                                | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
                            reinterpret_cast<LPCWSTR>(addr),
                            &handle)
        || handle == nullptr) {
        return false;
    }
    wchar_t file_path[MAX_PATH] = {0};
    if (GetModuleFileNameW(handle, file_path, MAX_PATH) == 0) {
        return false;
    }
    *path = normalizeModulePath(file_path);
    const auto runtime = reinterpret_cast<std::uint64_t>(addr);
    const auto module_base = reinterpret_cast<std::uint64_t>(handle);
    if (runtime < module_base) {
        return false;
    }
    *rva = runtime - module_base;
    return true;
}

bool parseNmLine(const std::string &line, SymbolEntry *entry)
{
    if (line.empty()) {
        return false;
    }
    char *end = nullptr;
    const std::uint64_t vma = std::strtoull(line.c_str(), &end, 16);
    if (end == line.c_str()) {
        return false;
    }
    while (*end == ' ' || *end == '\t') {
        ++end;
    }
    if (*end == '\0') {
        return false;
    }
    const char type = *end;
    if (type != 'T' && type != 't' && type != 'W' && type != 'w') {
        return false;
    }
    ++end;
    while (*end == ' ' || *end == '\t') {
        ++end;
    }
    std::string rest = end;
    while (!rest.empty() && (rest.back() == '\r' || rest.back() == '\n')) {
        rest.pop_back();
    }
    std::string name = rest;
    std::string location;
    const auto tab = rest.find('\t');
    if (tab != std::string::npos) {
        name = rest.substr(0, tab);
        location = rest.substr(tab + 1);
    }
    if (!shouldKeepSymbolName(name)) {
        return false;
    }
    entry->vma = vma;
    entry->name = name;
    entry->file.clear();
    entry->line = 0;
    if (!location.empty()) {
        const auto colon = location.find_last_of(':');
        if (colon != std::string::npos) {
            entry->file = shortenSourcePath(location.substr(0, colon));
            entry->line = std::atoi(location.c_str() + static_cast<int>(colon) + 1);
        } else {
            entry->file = shortenSourcePath(location);
        }
    }
    return true;
}

std::size_t loadSymbolTable(const std::wstring &module_path, const std::filesystem::path &sym_path)
{
    ModuleSymbols module;
    module.file_image_base = readFileImageBase(module_path);
    FILE *fp = _wfopen(sym_path.wstring().c_str(), L"r");
    if (fp == nullptr) {
        g_symbol_tables[module_path] = std::move(module);
        return 0;
    }
    std::string line;
    line.resize(65536);
    while (std::fgets(line.data(), static_cast<int>(line.size()), fp) != nullptr) {
        SymbolEntry entry;
        if (!parseNmLine(line.c_str(), &entry)) {
            continue;
        }
        if (!module.entries.empty() && module.entries.back().vma == entry.vma) {
            if (entry.file.empty()) {
                continue;
            }
            module.entries.back() = std::move(entry);
            continue;
        }
        module.entries.push_back(std::move(entry));
    }
    std::fclose(fp);
    const std::size_t count = module.entries.size();
    g_symbol_tables[module_path] = std::move(module);
    return count;
}

const ModuleSymbols *symbolsForModule(const std::wstring &module_path)
{
    auto it = g_symbol_tables.find(module_path);
    if (it != g_symbol_tables.end()) {
        return &it->second;
    }
    if (module_path.empty()) {
        return nullptr;
    }
    const std::filesystem::path beside =
        std::filesystem::path(module_path).replace_extension(kSymbolFileSuffix);
    loadSymbolTable(module_path, beside);
    it = g_symbol_tables.find(module_path);
    if (it == g_symbol_tables.end()) {
        return nullptr;
    }
    return &it->second;
}

const SymbolEntry *lookupSymbol(const std::vector<SymbolEntry> &table, std::uint64_t vma)
{
    if (table.empty() || vma < table.front().vma) {
        return nullptr;
    }
    const auto it = std::upper_bound(
        table.begin(),
        table.end(),
        vma,
        [](std::uint64_t addr, const SymbolEntry &entry) {
            return addr < entry.vma;
        });
    if (it == table.begin()) {
        return nullptr;
    }
    return &*(it - 1);
}

bool usefulResolvedName(const std::string &name)
{
    if (name.empty() || name == "??") {
        return false;
    }
    if (name.find("DTOR_LIST") != std::string::npos
        || name.find("CTOR_LIST") != std::string::npos) {
        return false;
    }
    return true;
}

void writeStackFrame(FILE *fp,
                     std::size_t index,
                     const std::string &func,
                     const std::string &file,
                     int line,
                     const std::string &module_name,
                     std::uint64_t rva)
{
    if (!file.empty() && line > 0) {
        std::fprintf(fp, "#%-2zu %s:%d\n", index, file.c_str(), line);
        if (!func.empty()) {
            std::fprintf(fp, "    %s\n", func.c_str());
        }
    } else if (!file.empty()) {
        std::fprintf(fp, "#%-2zu %s\n", index, file.c_str());
        if (!func.empty()) {
            std::fprintf(fp, "    %s\n", func.c_str());
        }
    } else if (!func.empty()) {
        std::fprintf(fp, "#%-2zu %s\n", index, func.c_str());
    } else {
        std::fprintf(fp, "#%-2zu %s\n", index, module_name.c_str());
    }
    std::fprintf(fp,
                 "    %s + 0x%llx\n",
                 module_name.c_str(),
                 static_cast<unsigned long long>(rva));
}

void resolveDbgHelpFrame(backward::Printer &printer,
                         const backward::Trace &trace,
                         void *lookup,
                         std::string *func,
                         std::string *file,
                         int *line)
{
    backward::ResolvedTrace request(trace);
    request.addr = lookup != nullptr ? lookup : trace.addr;
    const backward::ResolvedTrace resolved = printer.resolver().resolve(request);
    std::string name = resolved.source.function.empty()
                           ? resolved.object_function
                           : resolved.source.function;
    if (!usefulResolvedName(name)) {
        name = demangleName(resolved.object_function);
    }
    if (!usefulResolvedName(name)) {
        name.clear();
    }
    *func = std::move(name);
    *file = shortenSourcePath(resolved.source.filename);
    *line = static_cast<int>(resolved.source.line);
}

void loadMainExecutableSymbols()
{
    wchar_t exe_path[MAX_PATH] = {0};
    if (GetModuleFileNameW(nullptr, exe_path, MAX_PATH) == 0) {
        notice(NoticeLevel::Warn, "crash symbols: cannot get exe path");
        return;
    }
#if SHORE_CRASH_USE_NM_SYM
    const std::wstring module_path = normalizeModulePath(exe_path);
    const std::filesystem::path sym_path =
        std::filesystem::path(module_path).replace_extension(kSymbolFileSuffix);
    const std::size_t count = loadSymbolTable(module_path, sym_path);
    if (count == 0) {
        notice(NoticeLevel::Warn, "crash symbols missing: %s", wideToUtf8(sym_path.wstring()).c_str());
        return;
    }
    const auto it = g_symbol_tables.find(module_path);
    const std::uint64_t image_base =
        it == g_symbol_tables.end() ? 0x140000000ull : it->second.file_image_base;
    notice(NoticeLevel::Info, "crash symbols loaded %zu image_base=0x%llx from %s",
           count,
           static_cast<unsigned long long>(image_base),
           wideToUtf8(sym_path.wstring()).c_str());
#else
    // 提前初始化 DbgHelp / 加载 PDB，崩溃时 StackWalk64 才有符号
    static const auto *warmup = new backward::Printer();
    (void)warmup;
    const std::filesystem::path pdb_path =
        std::filesystem::path(exe_path).replace_extension(kPdbFileSuffix);
    std::error_code error;
    if (!std::filesystem::exists(pdb_path, error)) {
        notice(NoticeLevel::Warn, "crash symbols missing: %s", wideToUtf8(pdb_path.wstring()).c_str());
        return;
    }
    notice(NoticeLevel::Info, "crash symbols pdb %s", wideToUtf8(pdb_path.wstring()).c_str());
#endif
}

void writeResolvedWindowsStack(FILE *fp, backward::StackTrace &st, backward::Printer &printer)
{
    printer.resolver().load_stacktrace(st);
    std::fprintf(fp, "---- stack (#0 is the crash site) ----\n");
#if SHORE_CRASH_USE_NM_SYM
    std::size_t exe_symbols = 0;
    std::uint64_t exe_image_base = 0x140000000ull;
    wchar_t exe_path[MAX_PATH] = {0};
    if (GetModuleFileNameW(nullptr, exe_path, MAX_PATH) > 0) {
        const auto *exe_table = symbolsForModule(normalizeModulePath(exe_path));
        if (exe_table != nullptr) {
            exe_symbols = exe_table->entries.size();
            exe_image_base = exe_table->file_image_base;
        }
    }
    std::fprintf(fp,
                 "symbols %zu image_base 0x%llx\n",
                 exe_symbols,
                 static_cast<unsigned long long>(exe_image_base));
#else
    wchar_t exe_path[MAX_PATH] = {0};
    if (GetModuleFileNameW(nullptr, exe_path, MAX_PATH) > 0) {
        const std::filesystem::path pdb_path =
            std::filesystem::path(exe_path).replace_extension(kPdbFileSuffix);
        std::error_code error;
        std::fprintf(fp,
                     "resolver pdb %s (%s)\n",
                     wideToUtf8(pdb_path.filename().wstring()).c_str(),
                     std::filesystem::exists(pdb_path, error) ? "ok" : "missing");
    } else {
        std::fprintf(fp, "resolver pdb\n");
    }
#endif

    for (std::size_t i = 0; i < st.size(); ++i) {
        void *addr = st[i].addr;
        void *lookup = (i == 0 || addr == nullptr)
                           ? addr
                           : static_cast<std::uint8_t *>(addr) - 1;
        std::wstring module_path;
        std::uint64_t rva = 0;
        std::string module_name = "?";
        if (lookup != nullptr && queryModule(lookup, &module_path, &rva)) {
            module_name = std::filesystem::path(module_path).filename().u8string();
        }

        std::string func;
        std::string file;
        int line = 0;
#if SHORE_CRASH_USE_NM_SYM
        const ModuleSymbols *table = symbolsForModule(module_path);
        const SymbolEntry *symbol = nullptr;
        if (table != nullptr && !table->entries.empty()) {
            const std::uint64_t vma = table->file_image_base + rva;
            symbol = lookupSymbol(table->entries, vma);
        }
        if (symbol != nullptr) {
            func = symbol->name;
            file = symbol->file;
            line = symbol->line;
        } else {
            resolveDbgHelpFrame(printer, st[i], lookup, &func, &file, &line);
        }
#else
        resolveDbgHelpFrame(printer, st[i], lookup, &func, &file, &line);
#endif
        writeStackFrame(fp, i, func, file, line, module_name, rva);
    }
    std::fprintf(fp, "\n");
}
#endif

void writeCrashDump(backward::StackTrace &st,
                    backward::Printer &printer,
                    bool allow_logger)
{
    printer.address = true;
    printer.object = true;
    printer.snippet = false;
    printer.color_mode = backward::ColorMode::never;

    const char *reason = backward::crash_reason_buffer();
    const std::string file_name = makeCrashFileName();

    std::error_code error;
    std::filesystem::path directory = utf8Path(g_dump_directory_utf8);
    if (directory.empty()) {
        directory = std::filesystem::current_path(error);
    }
    std::filesystem::create_directories(directory, error);
    const std::filesystem::path path = directory / file_name;

    FILE *fp = openUtf8File(path, false);
    if (fp != nullptr) {
        std::fprintf(fp, "%s crash\n", g_product_name.empty() ? "app" : g_product_name.c_str());
        if (reason != nullptr && reason[0] != '\0') {
            std::fprintf(fp, "reason: %s\n", reason);
        }
        std::fprintf(fp, "pid: %d\n", currentProcessId());
        std::fprintf(fp, "file: %s\n\n", file_name.c_str());
#ifdef _WIN32
        writeResolvedWindowsStack(fp, st, printer);
#endif
        std::fflush(fp);
        std::fclose(fp);
    }

    const std::string path_utf8 = path.u8string();
    if (allow_logger) {
        notice(NoticeLevel::Fatal, "crash stack written: %s", path_utf8.c_str());
        if (g_flush != nullptr) {
            g_flush();
        }
    } else {
        // 崩溃路径不走 spdlog，避免正好持有日志锁时死锁
        appendCrashMarker(path_utf8, reason);
    }
}

void hookDump(backward::StackTrace &st, backward::Printer &printer)
{
#ifdef _WIN32
    // 后台报告线程里写文件；写完直接结束进程，避免 abort 再次进入处理器死锁
    writeCrashDump(st, printer, false);
    std::_Exit(EXIT_FAILURE);
#else
    writeCrashDump(st, printer, false);
#endif
}
}  // namespace

void setNoticeLogger(NoticeFn notice, FlushFn flush)
{
    g_notice = notice;
    g_flush = flush;
}

bool install(const InstallOptions &options)
{
    static bool installed = false;
    if (installed) {
        return true;
    }

    g_dump_directory_utf8 = options.dump_directory_utf8;
    g_log_file_name = options.log_file_name;
    g_product_name = options.product_name;
    backward::crash_dump_hook() = &hookDump;

    try {
        // 故意不释放：析构会停掉报告线程，崩溃必须覆盖整个进程寿命
        static const auto *guard = new backward::SignalHandling();
        (void)guard;
        installed = true;
#ifdef _WIN32
        loadMainExecutableSymbols();
#endif
        notice(NoticeLevel::Info, "crash trace installed, dump dir %s", g_dump_directory_utf8.c_str());
        return true;
    } catch (...) {
        notice(NoticeLevel::Error, "crash trace install failed");
        return false;
    }
}

void logCurrentStack(const char *reason)
{
    backward::set_crash_reason(reason != nullptr && reason[0] != '\0'
                                   ? reason
                                   : "logCurrentStack");

    backward::Printer printer;
    backward::StackTrace st;
    st.load_here(48);
    st.skip_n_firsts(1);
    writeCrashDump(st, printer, true);
}
}  // namespace hczk::crash
