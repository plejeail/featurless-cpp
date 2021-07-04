#include "featurless/log.h"

#include <cstring>
#include <filesystem>
#include <fstream>
#include <mutex>
#include <string>
#include <string_view>
#include <thread>

#ifdef _WIN32
#include <sys/time.h>
#else  // UNIX LIKE
#include <sys/timeb.h>
#endif

struct featurless::log::impl
{
    std::ofstream _ofstream;
    size_t _current_file_size;
    size_t _max_file_size;
    short _max_files;

    std::string _file_path;
    std::string _file_name;
    std::string _file_ext;
    std::mutex _mutex;
};

featurless::log featurless::log::_instance;

inline tm localtime_s() noexcept
{
    time_t time_now;  // NOLINT
    time(&time_now);
    tm t{};
#if defined(_WIN32) && defined(__BORLANDC__)
    ::localtime_s(&time_now, &t);
#elif defined(_WIN32) && defined(__MINGW32__) && !defined(__MINGW64_VERSION_MAJOR)
    t = *::localtime(&time_now);
#elif defined(_WIN32)
    ::localtime_s(&t, &time_now);
#else
    ::localtime_r(&time_now, &t);
#endif
    return t;
}

constexpr size_t estimate_record_size(size_t dynamic_size) noexcept
{
    return 48 + dynamic_size;
}

void featurless::log::write(const std::string_view level,
                            const std::string_view line,
                            const std::string_view function,
                            const std::string_view src_file,
                            const std::string_view message)
{
    size_t record_size =
      estimate_record_size(line.size() + function.size() + src_file.size() + message.size());

    if ((_data->_current_file_size + record_size) > _data->_max_file_size  //
        && _data->_max_files > 0) [[unlikely]]
        rotate();
    _data->_current_file_size += record_size;

    write_record(level, line, function, src_file, message);
}

template<typename int_t>
static void copy_int(char* dest, int_t integer)
{
    while (integer > 0)
    {
        *dest = '0' + static_cast<char>(integer % 10);
        integer /= 10;
        --dest;
    }
}

template<typename int_t>
static void copy_hex(char* dest, int_t integer)
{
    constexpr std::array<char, 16> digits{ '0', '1', '2', '3', '4', '5', '6', '7',
                                           '8', '9', 'A', 'B', 'C', 'D', 'E', 'F' };
    while (integer > 0)
    {
        *dest = digits[integer % 16];
        integer /= 16;
        --dest;
    }
}

inline unsigned long int fucking_std_thread_id() noexcept
{
#if _MSC_VER > 0
    auto stupid_std_id = std::this_thread::get_id();
    return *reinterpret_cast<unsigned int*>(&stupid_std_id);
#elif defined(__GNUC__)  // dont know if it works on intel comiler
    auto stupid_std_id = std::this_thread::get_id();
    return *reinterpret_cast<unsigned long int*>(&stupid_std_id);
#else                    // no thread id for you
    constexpr int noid{ 0 };
    return noid;
#endif
}

void featurless::log::write_record(const std::string_view level,
                                   const std::string_view line,
                                   const std::string_view function,
                                   const std::string_view src_file,
                                   const std::string_view message)
{
    tm time_info = localtime_s();
    std::string msg_buffer = "[0000-00-00 00:00:00][000000000000][      ][";
    msg_buffer.reserve(msg_buffer.size() + line.size() + function.size() + src_file.size()
                       + message.size() + 6);

    copy_int(msg_buffer.data() + 4, time_info.tm_year + 1900);
    copy_int(msg_buffer.data() + 7, time_info.tm_mon + 1);
    copy_int(msg_buffer.data() + 10, time_info.tm_mday);
    copy_int(msg_buffer.data() + 13, time_info.tm_hour);
    copy_int(msg_buffer.data() + 16, time_info.tm_min);
    copy_int(msg_buffer.data() + 19, time_info.tm_sec);
    auto dumb_comitee_id = std::this_thread::get_id();
    copy_hex(msg_buffer.data() + 33, fucking_std_thread_id());
    std::memcpy(msg_buffer.data() + 36, level.data(), level.size());
    msg_buffer += function;
    msg_buffer += "]@(";  // Location
    msg_buffer += src_file;
    msg_buffer += ',';
    msg_buffer += line;
    msg_buffer += ") ";
    msg_buffer += message;
    msg_buffer += '\n';

    std::scoped_lock s{ _data->_mutex };
    _data->_ofstream << msg_buffer;
}

void featurless::log::rotate()
{
    std::scoped_lock s{ _data->_mutex };
    _data->_ofstream.close();

    for (int file_number = _data->_max_files - 2; file_number >= 0; --file_number)
    {
        std::error_code nothrow_if_fail;
        std::filesystem::rename(build_file_name(file_number), build_file_name(file_number + 1),
                                nothrow_if_fail);
    }
    _data->_current_file_size = 0;
    _data->_ofstream.open(build_file_name(0));
}

std::string featurless::log::build_file_name(int file_number)
{
    constexpr size_t estimated_number_digits = 2;  // if more than 2 digits, 2x allocation
    std::string filename;
    filename.reserve(_data->_file_name.size() + _data->_file_ext.size() + estimated_number_digits);
    filename += _data->_file_name;
    if (file_number > 0)
    {
        filename += '.';
        filename += std::to_string(file_number);
    }
    if (!_data->_file_ext.empty())
    {
        filename += _data->_file_ext;
    }

    return filename;
}

void featurless::log::init(const char* logfile_path, size_t max_size_kB, short max_files)
{
    _instance._data = new impl();
    if (max_files < 0)
        abort(/* logger::init max number if files less than 0*/);

    _instance._data->_max_file_size = max_size_kB * 1000;
    _instance._data->_max_files = max_files;

    std::filesystem::path p{ logfile_path };
    std::error_code nothrow_if_fail;
    _instance._data->_current_file_size = std::filesystem::file_size(logfile_path, nothrow_if_fail);

    if (nothrow_if_fail)
        _instance._data->_current_file_size = 0;

    _instance._data->_file_ext = p.extension();
    p.replace_extension();
    _instance._data->_file_name = p;
    p.remove_filename();
    std::filesystem::create_directories(p);

    _instance._data->_ofstream.open(_instance.build_file_name(0), std::ios_base::app);
}

featurless::log::~log()
{
    delete _data;
}
