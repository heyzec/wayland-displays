#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>

std::shared_ptr<spdlog::logger> logger = nullptr;

void init_logger() {
  logger = spdlog::stdout_color_mt("app_logger");
  spdlog::set_pattern("[%^%l%$] %v");
  spdlog::set_level(spdlog::level::debug);
}
