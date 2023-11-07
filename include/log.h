#pragma once

#ifndef MOD_ID
#define MOD_ID "whatauth"
#endif

#ifndef VERSION
#define VERSION "0.1.0"
#endif

#define LOG_IDENTIFIER "[" MOD_ID "|" VERSION "]"

#include <fmt/compile.h>
#include <fmt/core.h>

#if defined(QUEST)
// quest specific includes and logging
#include <android/log.h>

#define LOG_LEVEL(lvl, str, ...)                                                \
  do {                                                                          \
    std::string __ss = fmt::format(FMT_COMPILE(str) __VA_OPT__(, __VA_ARGS__)); \
    __android_log_write(lvl, LOG_IDENTIFIER, __ss.c_str());                     \
  } while (0)

#define LEVEL_INFO ANDROID_LOG_INFO
#define LEVEL_FATAL ANDROID_LOG_FATAL
#define LEVEL_ERROR ANDROID_LOG_ERROR
#define LEVEL_DEBUG ANDROID_LOG_DEBUG
#define LEVEL_WARN ANDROID_LOG_WARN
#define LEVEL_VERBOSE ANDROID_LOG_VERBOSE
#else

#define LOG_LEVEL(lvl, str, ...) \
  fmt::print(FMT_COMPILE(lvl " " LOG_IDENTIFIER ": " str "\r\n") __VA_OPT__(, __VA_ARGS__))

#define LEVEL_INFO "I"
#define LEVEL_FATAL "C"
#define LEVEL_ERROR "E"
#define LEVEL_DEBUG "D"
#define LEVEL_WARN "W"
#define LEVEL_VERBOSE "V"
#endif

#define LOG_INFO(str, ...) LOG_LEVEL(LEVEL_INFO, str __VA_OPT__(, __VA_ARGS__))
#define LOG_CRITICAL(str, ...) LOG_LEVEL(LEVEL_FATAL, str __VA_OPT__(, __VA_ARGS__))
#define LOG_FATAL(str, ...) LOG_LEVEL(LEVEL_FATAL, str __VA_OPT__(, __VA_ARGS__))
#define LOG_ERROR(str, ...) LOG_LEVEL(LEVEL_ERROR, str __VA_OPT__(, __VA_ARGS__))
#define LOG_DEBUG(str, ...) LOG_LEVEL(LEVEL_DEBUG, str __VA_OPT__(, __VA_ARGS__))
#define LOG_WARN(str, ...) LOG_LEVEL(LEVEL_WARN, str __VA_OPT__(, __VA_ARGS__))
#define LOG_VERBOSE(str, ...) LOG_LEVEL(LEVEL_VERBOSE, str __VA_OPT__(, __VA_ARGS__))
