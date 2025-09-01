#pragma once
#include "logger.h"
#include <memory>
#include <spdlog/logger.h>

class ILogger {
public:
    virtual ~ILogger() = default;
    virtual void info(const std::string& message) = 0;
    virtual void debug(const std::string& message) = 0;
    virtual void warn(const std::string& message) = 0;
    virtual void error(const std::string& message) = 0;
};

// Adapter for spdlog
class SpdlogAdapter : public ILogger {
private:
    // SPD use shared pointer for it internal logic, so we should use shared pointer here
    std::shared_ptr<spdlog::logger> logger_;
    
public:
    explicit SpdlogAdapter(std::shared_ptr<spdlog::logger> logger) 
        : logger_(std::move(logger)) {}
    
    void info(const std::string& message) override {
        if (logger_) logger_->info(message);
    }
    
    void debug(const std::string& message) override {
        if (logger_) logger_->debug(message);
    }
    
    void warn(const std::string& message) override {
        if (logger_) logger_->warn(message);
    }
    
    void error(const std::string& message) override {
        if (logger_) logger_->error(message);
    }
};