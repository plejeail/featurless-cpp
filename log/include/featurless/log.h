//===-- logger.h ----------------------------------------------------------===//
//                         SIMPLE LOGGING LIBRARY
//
//
// Log level:
// Filter logs by level at compile time and runtime.
// Compile time:
// Set FEATURLESS_LOG_MIN_LEVEL before including "featurless/logger.h"
// TRACE, DEBUG, INFO, WARNING, ERROR, CRITICAL
//
// Log format:
// Date;Time;Thread;Level;File;Line;Function;Message
// Example:
// int main()
// {
// }
//===----------------------------------------------------------------------===//
#include <fstream>
#include <mutex>
#include <string>
#include <string_view>

#define FLOG_LEVEL_TRACE 0
#define FLOG_LEVEL_DEBUG 1
#define FLOG_LEVEL_INFO  2
#define FLOG_LEVEL_WARN  3
#define FLOG_LEVEL_ERROR 4
#define FLOG_LEVEL_FATAL 5
#define FLOG_LEVEL_NONE  6

#ifndef FEATURLESS_LOG_MIN_LEVEL
#define FEATURLESS_LOG_MIN_LEVEL FLOG_LEVEL_INFO
#endif

#if FEATURLESS_LOG_MIN_LEVEL <= FLOG_LEVEL_TRACE
#define FLOG_TRACE(message)                                                                   \
    featurless::log::logger().write(                                                          \
      featurless::__level_to_string<featurless::log::level::trace>(),                         \
      __FEATURLESS_STRINGIZE(__LINE__), 0, __func__, featurless::__pretty_filename(__FILE__), \
      message)
#else
#define FLOG_TRACE(message)
#endif
#if FEATURLESS_LOG_MIN_LEVEL <= FLOG_LEVEL_DEBUG
#define FLOG_DEBUG(message)                                                                   \
    featurless::log::logger().write(                                                          \
      featurless::__level_to_string<featurless::log::level::debug>(),                         \
      __FEATURLESS_STRINGIZE(__LINE__), 0, __func__, featurless::__pretty_filename(__FILE__), \
      message)
#else
#define FLOG_DEBUG(message)
#endif
#if FEATURLESS_LOG_MIN_LEVEL <= FLOG_LEVEL_INFO
#define FLOG_INFO(message)                                                                         \
    featurless::log::logger().write(featurless::__level_to_string<featurless::log::level::info>(), \
                                    __FEATURLESS_STRINGIZE(__LINE__), 0, __func__,                 \
                                    featurless::__pretty_filename(__FILE__), message)
#else
#define FLOG_INFO(message)
#endif
#if FEATURLESS_LOG_MIN_LEVEL <= FLOG_LEVEL_WARN
#define FLOG_WARN(message)                                                                    \
    featurless::log::logger().write(                                                          \
      featurless::__level_to_string<featurless::log::level::warning>(),                       \
      __FEATURLESS_STRINGIZE(__LINE__), 0, __func__, featurless::__pretty_filename(__FILE__), \
      message)
#else
#define FLOG_WARN(message)
#endif
#if FEATURLESS_LOG_MIN_LEVEL <= FLOG_LEVEL_ERROR
#define FLOG_ERROR(message)                                                                   \
    featurless::log::logger().write(                                                          \
      featurless::__level_to_string<featurless::log::level::error>(),                         \
      __FEATURLESS_STRINGIZE(__LINE__), 0, __func__, featurless::__pretty_filename(__FILE__), \
      message)
#else
#define FLOG_ERROR(message)
#endif
#if FEATURLESS_LOG_MIN_LEVEL <= FLOG_LEVEL_FATAL
#define FLOG_FATAL(message)                                                                   \
    featurless::log::logger().write(                                                          \
      featurless::__level_to_string<featurless::log::level::fatal>(),                         \
      __FEATURLESS_STRINGIZE(__LINE__), 0, __func__, featurless::__pretty_filename(__FILE__), \
      message)
#else
#define FLOG_FATAL(message)
#endif

namespace featurless
{
class log
{
public:
    enum class level : char
    {
        trace = 0,
        debug = 1,
        info = 2,
        warning = 3,
        error = 4,
        fatal = 5,
        _nb_levels
    };

    static void init(const char* logfile_path, size_t max_size_kB = 0, short max_files = 0);

    static log& logger() noexcept { return _instance; }
    void write(const std::string_view level,
               const std::string_view line,
               const int32_t thread_id,
               const std::string_view function,
               const std::string_view src_file,
               const std::string_view message);

private:
    static log _instance;
    void write_record(const std::string_view level,
                      const std::string_view line,
                      const int32_t thread_id,
                      const std::string_view function,
                      const std::string_view src_file,
                      const std::string_view message);
    void rotate();
    std::string build_file_name(int file_number = 0);

    std::ofstream _ofstream;
    size_t _current_file_size;
    size_t _max_file_size;
    short _max_files;

    std::string _file_path;
    std::string _file_name;
    std::string _file_ext;
    std::mutex _mutex;
};


#if FEATURLESS_LOG_MIN_LEVEL < FLOG_LEVEL_NONE
#define __FEATURLESS_STRINGIZE(x)    __FEATURLESS_STRINGIZE_DT(x)
#define __FEATURLESS_STRINGIZE_DT(x) #x

consteval std::string_view __pretty_filename(const std::string_view filename)
{  // to be used only on __FILE__ macro
#ifdef _WIN32
    auto it = std::find(filename.rbegin() + 3, filename.rend(), '\\');
#else
    auto it = --std::find(filename.rbegin() + 3, filename.rend(), '/');
#endif
    return std::string_view(&(*it), &*filename.cend() - &*it);
}

template<featurless::log::level lvl>
consteval std::string_view __level_to_string() noexcept
{
    using level = featurless::log::level;
    if constexpr (lvl == level::trace)
        return std::string_view("trace");
    else if constexpr (lvl == level::debug)
        return std::string_view("debug");
    else if constexpr (lvl == level::info)
        return std::string_view("info ");
    else if constexpr (lvl == level::warning)
        return std::string_view("warn ");
    else if constexpr (lvl == level::error)
        return std::string_view("error");
    else if constexpr (lvl == level::fatal)
        return std::string_view("fatal");
    else
        return std::string_view(" ??? ");
}
#endif
}  // namespace featurless
