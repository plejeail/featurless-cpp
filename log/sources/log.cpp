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

featurless::log featurless::log::_instance;

struct featurless::log::impl
{
    char* _streambuffer{ nullptr };
    std::ofstream _ofstream;
    std::size_t _current_file_size{ 0 };
    std::size_t _max_file_size{ 0 };
    short _max_files{ 0 };

    std::string _file_path;
    std::string _file_name;
    std::string _file_ext;
    std::mutex _mutex;
};

inline std::size_t estimate_record_size(std::size_t dynamic_size) noexcept
{
    return 50 + dynamic_size;
}

template<bool use_utc>
void featurless::log::write(const std::string_view lvl_str,
                            const std::string_view line,
                            const std::string_view function,
                            const std::string_view src_file,
                            const std::string_view message)
{
    std::size_t record_size = estimate_record_size(line.size() + function.size() + src_file.size() + message.size());

    if ((_data->_current_file_size + record_size) > _data->_max_file_size && _data->_max_files > 0) [[unlikely]]
        rotate();

    write_record<use_utc>(lvl_str, line, function, src_file, message);
}

template void featurless::log::write<false>(const std::string_view lvl_str,
                                            const std::string_view line,
                                            const std::string_view function,
                                            const std::string_view src_file,
                                            const std::string_view message);

template void featurless::log::write<true>(const std::string_view lvl_str,
                                           const std::string_view line,
                                           const std::string_view function,
                                           const std::string_view src_file,
                                           const std::string_view message);

template<typename int_t>
inline void copy_hex(char* dest, int_t integer) noexcept
{
    constexpr std::array<char, 16> digits{ '0', '1', '2', '3', '4', '5', '6', '7',
                                           '8', '9', 'a', 'b', 'c', 'd', 'e', 'f' };
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
#elif defined(__GNUC__) && !defined(__INTEL_COMPILER)  // dont know if it works on intel comiler
    auto stupid_std_id = std::this_thread::get_id();
    return *reinterpret_cast<unsigned long int*>(&stupid_std_id);
#else                                                  // no thread id for you, sorry
    constexpr int noid{ 0 };
    return noid;
#endif
}

inline tm __featurless_localtime_s() noexcept
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

inline tm __featurless_gmtime_s() noexcept
{
    time_t time_now;  // NOLINT
    time(&time_now);
    tm t{};
#if defined(_WIN32) && defined(__BORLANDC__)
    ::gmtime_s(&time_now, &t);
#elif defined(_WIN32) && defined(__MINGW32__) && !defined(__MINGW64_VERSION_MAJOR)
    t = *::gmtime(&time_now);
#elif defined(_WIN32)
    ::gmtime_s(&t, &time_now);
#else
    ::gmtime_r(&time_now, &t);
#endif
    return t;
}

static void copy_int(char* dest, int integer) noexcept
{
    dest[0] = static_cast<char>('0' + integer / 10);
    dest[1] = static_cast<char>('0' + integer % 10);
}

template<bool use_utc>
inline void featurless::log::write_record(const std::string_view lvl_str,
                                          const std::string_view line,
                                          const std::string_view function,
                                          const std::string_view src_file,
                                          const std::string_view message)
{
    tm time_info;  // NOLINT
    if constexpr (use_utc)
        time_info = __featurless_gmtime_s();
    else
        time_info = __featurless_localtime_s();

    std::size_t length_buffer = estimate_record_size(line.size() + function.size() + src_file.size() + message.size());
#if defined(_WIN32)
    char* msg_buffer = reinterpret_cast<char*>(_alloca(length_buffer));
#elif defined(__GNUC__)
    char* msg_buffer = reinterpret_cast<char*>(alloca(length_buffer));
#else
    char* msg_buffer = reinterpret_cast<char*>(malloc(length_buffer));
#endif
    std::memcpy(msg_buffer, "[2000-00-00 00:00:00][000000000000][     ][", 44);

    copy_int(msg_buffer + 3, time_info.tm_year - 100);
    copy_int(msg_buffer + 6, time_info.tm_mon + 1);
    copy_int(msg_buffer + 9, time_info.tm_mday);
    copy_int(msg_buffer + 12, time_info.tm_hour);
    copy_int(msg_buffer + 15, time_info.tm_min);
    copy_int(msg_buffer + 18, time_info.tm_sec);
    copy_hex(msg_buffer + 33, fucking_std_thread_id());
    std::memcpy(msg_buffer + 36, lvl_str.data(), lvl_str.size());
    char* ptr_data = msg_buffer + 43;
    std::memcpy(ptr_data, function.data(), function.size());
    ptr_data += function.size();
    *(ptr_data++) = ']';
    *(ptr_data++) = '@';
    *(ptr_data++) = '(';
    std::memcpy(ptr_data, src_file.data(), src_file.size());
    ptr_data += src_file.size();
    *(ptr_data++) = ',';
    std::memcpy(ptr_data, line.data(), line.size());
    ptr_data += line.size();
    *(ptr_data++) = ')';
    *(ptr_data++) = '\t';
    std::memcpy(ptr_data, message.data(), message.size());
    ptr_data[message.size()] = '\n';

    std::scoped_lock s{ _data->_mutex };
    _data->_current_file_size += length_buffer;
    _data->_ofstream.write(msg_buffer, static_cast<std::streamsize>(length_buffer));

#if !defined(_WIN32) && !defined(__GNUC__)
    free(msg_buffer);
#endif
}

void featurless::log::rotate()
{
    std::scoped_lock s{ _data->_mutex };
    _data->_ofstream.close();

    std::string filename_current;
    std::string filename_new;
    for (int file_number = _data->_max_files - 2; file_number >= 0; --file_number)
    {
        std::error_code nothrow_if_fail;
        build_file_name(filename_current, file_number);
        build_file_name(filename_new, file_number + 1);
        std::filesystem::rename(filename_current, filename_new, nothrow_if_fail);
    }
    _data->_current_file_size = 0;

    _data->_ofstream.open(filename_current, std::ios::binary);
    _instance._data->_ofstream.rdbuf()->pubseekpos(static_cast<std::streamoff>(_instance._data->_max_file_size));
    _instance._data->_ofstream.write("\n", 1);
    _instance._data->_ofstream.rdbuf()->pubseekpos(0);
}

void featurless::log::build_file_name(std::string& filename, int file_number)
{
    constexpr std::size_t estimated_number_digits = 2;  // if more than 2 digits, 2x allocation
    filename.reserve(_data->_file_name.size() + _data->_file_ext.size() + estimated_number_digits);
    filename.resize(0);
    filename += _data->_file_name;
    if (file_number > 0) [[likely]]
    {
        filename += '.';
        filename += std::to_string(file_number);
    }
    if (!_data->_file_ext.empty())
    {
        filename += _data->_file_ext;
    }
}


void featurless::log::init(const char* logfile_path,
                           std::size_t max_size_kB,
                           short max_files,
                           std::size_t buffer_size_kB)
{
    _instance._data = new impl();
    if (max_files < 0)
        abort(/* logger::init max number if files less than 0*/);

    _instance._data->_max_file_size = max_size_kB * 1000;
    _instance._data->_max_files = max_files;

    std::filesystem::path p{ logfile_path };
    std::error_code file_size_fail;
    _instance._data->_current_file_size = std::filesystem::file_size(logfile_path, file_size_fail);

    if (file_size_fail)
        _instance._data->_current_file_size = 0;

    _instance._data->_file_ext = p.extension();
    p.replace_extension();
    _instance._data->_file_name = p;
    p.remove_filename();
    if (!p.empty())
        std::filesystem::create_directories(p);

    if (buffer_size_kB > 0)
    {
        buffer_size_kB *= 1000;
        _instance._data->_streambuffer = new char[buffer_size_kB];
        _instance._data->_ofstream.rdbuf()->pubsetbuf(_instance._data->_streambuffer,
                                                      static_cast<std::streamsize>(buffer_size_kB));
    }

    _instance._data->_ofstream.open(logfile_path, std::ios_base::app | std::ios::binary);
    // avoid fragmentation by writing at the end directly
    _instance._data->_ofstream.rdbuf()->pubseekpos(static_cast<std::streamoff>(_instance._data->_max_file_size));
    _instance._data->_ofstream.write("\n", 1);
    _instance._data->_ofstream.rdbuf()->pubseekpos(static_cast<std::streamoff>(_instance._data->_current_file_size));
}

featurless::log::~log()
{
    delete[] _data->_streambuffer;
    delete _data;
}
