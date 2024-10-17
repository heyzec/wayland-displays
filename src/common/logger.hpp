#include <spdlog/spdlog.h>

extern std::shared_ptr<spdlog::logger> logger;

void init_logger();

// Aliases for logging macros
#define log_debug(...) logger->debug(__VA_ARGS__)
#define log_info(...) logger->info(__VA_ARGS__)
#define log_warn(...) logger->warn(__VA_ARGS__)
#define log_error(...) logger->error(__VA_ARGS__)
#define log_critical(...) logger->critical(__VA_ARGS__)
