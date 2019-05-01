#pragma once

#include "spdlog/spdlog.h"

class Log
{
public:
    static void Init(const std::string& name);
    inline static std::shared_ptr<spdlog::logger>& GetLogger() { return s_Logger; }

private:
    static std::shared_ptr<spdlog::logger> s_Logger;
};

#define DX_TRACE(...)       Log::GetLogger()->trace(__VA_ARGS__)
#define DX_INFO(...)        Log::GetLogger()->info(__VA_ARGS__)
#define DX_WARN(...)        Log::GetLogger()->warn(__VA_ARGS__)
#define DX_ERROR(...)       Log::GetLogger()->error(__VA_ARGS__)
#define DX_CRITICAL(...)    Log::GetLogger()->critical(__VA_ARGS__)

