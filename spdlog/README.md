# spdlog 1.15.3

来源：https://github.com/gabime/spdlog  
许可证：MIT  
用途：主体日志底层。业务与插件代码不要直接 include spdlog，统一使用 `common/w_log.h` 中的 `W_INFO` / `W_DEBUG` / `W_WARN` / `W_ERROR`。
