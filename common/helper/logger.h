#pragma once

#include <string>
#include <memory>

#include "common/helper/singleton.h"
#include "spdlog/sinks/stdout_sinks.h"
#include "spdlog/spdlog.h"

namespace spd = spdlog;

class Logger {
public:
    Logger() : Logger("default") {}

    Logger(const std::string &moduleLibraryName) {
        // delegate = spd::rotating_logger_mt(moduleName, ".", 1048576 * 5, 3);
        delegate = spd::stdout_logger_mt(moduleLibraryName);

        delegate->flush_on(parseLevel());
        spd::set_level(parseLevel());
        spd::set_pattern("[%Y-%m-%d %H:%M:%S] [%^%L%$] [%n-%t] %v");
    }

    ~Logger() { spd::drop(moduleName); }

    std::shared_ptr<spdlog::logger> getLogger() { return delegate; }

private:
    spd::level::level_enum parseLevel() {
        spd::level::level_enum level = spdlog::level::info;
        const char *levelEnv         = getenv("SPD_LOG_LEVEL");
        if (nullptr != levelEnv && strlen(levelEnv) > 0) {
            level = spd::level::from_str(levelEnv);
        }
        return level;
    }

private:
    std::string moduleName;
    std::shared_ptr<spdlog::logger> delegate;
};

static inline Logger &defaultLogger() {
    return Singleton<Logger>::getInstance();
}

#define LLOG_TRACE(logger, ...)    logger.getLogger()->trace(__VA_ARGS__)
#define LLOG_DEBUG(logger, ...)    logger.getLogger()->debug(__VA_ARGS__)
#define LLOG_INFO(logger, ...)     logger.getLogger()->info(__VA_ARGS__)
#define LLOG_WARN(logger, ...)     logger.getLogger()->warn(__VA_ARGS__)
#define LLOG_ERROR(logger, ...)    logger.getLogger()->error(__VA_ARGS__)
#define LLOG_CRITICAL(logger, ...) logger.getLogger()->critical(__VA_ARGS__)

#define LOG_TRACE(...)    defaultLogger().getLogger()->trace(__VA_ARGS__)
#define LOG_DEBUG(...)    defaultLogger().getLogger()->debug(__VA_ARGS__)
#define LOG_INFO(...)     defaultLogger().getLogger()->info(__VA_ARGS__)
#define LOG_WARN(...)     defaultLogger().getLogger()->warn(__VA_ARGS__)
#define LOG_ERROR(...)    defaultLogger().getLogger()->error(__VA_ARGS__)
#define LOG_CRITICAL(...) defaultLogger().getLogger()->critical(__VA_ARGS__)
