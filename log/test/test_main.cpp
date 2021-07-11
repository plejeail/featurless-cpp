#include <chrono>
#include <iostream>
#include <thread>

#include <iomanip>
#include <sstream>
#define FEATURLESS_LOG_MIN_LEVEL FLOG_LEVEL_TRACE
// #define FEATURLESS_LOG_USE_UTC
#include <featurless/log.h>

int main()
{
    std::iostream::sync_with_stdio(false);

    featurless::log::init("myapp.log", 30000, 10);
    FLOG_DEBUG("OK TEST");
    std::chrono::steady_clock::time_point t1 = std::chrono::steady_clock::now();
    for (int i = 0; i < 30; ++i)
    {
        FLOG_DEBUG(
          "As a data scientist, reading and writing data from/to CSV is one of the most common"
          " tasks I do on the daily. R, my language of choice, makes this easy with read.csv() "
          "and write.csv()");
        FLOG_DEBUG(
          "As a data scientist, reading and writing data from/to CSV is one of the most common"
          " tasks I do on the daily. R, my language of choice, makes this easy with read.csv() "
          "and write.csv()");
        FLOG_DEBUG(
          "As a data scientist, reading and writing data from/to CSV is one of the most common"
          " tasks I do on the daily. R, my language of choice, makes this easy with read.csv() "
          "and write.csv()");
        FLOG_DEBUG(
          "As a data scientist, reading and writing data from/to CSV is one of the most common"
          " tasks I do on the daily. R, my language of choice, makes this easy with read.csv() "
          "and write.csv()");
        FLOG_DEBUG(
          "As a data scientist, reading and writing data from/to CSV is one of the most common"
          " tasks I do on the daily. R, my language of choice, makes this easy with read.csv() "
          "and write.csv()");
        FLOG_DEBUG(
          "As a data scientist, reading and writing data from/to CSV is one of the most common"
          " tasks I do on the daily. R, my language of choice, makes this easy with read.csv() "
          "and write.csv()");
        FLOG_DEBUG(
          "As a data scientist, reading and writing data from/to CSV is one of the most common"
          " tasks I do on the daily. R, my language of choice, makes this easy with read.csv() "
          "and write.csv()");
        FLOG_DEBUG(
          "As a data scientist, reading and writing data from/to CSV is one of the most common"
          " tasks I do on the daily. R, my language of choice, makes this easy with read.csv() "
          "and write.csv()");
        FLOG_DEBUG(
          "As a data scientist, reading and writing data from/to CSV is one of the most common"
          " tasks I do on the daily. R, my language of choice, makes this easy with read.csv() "
          "and write.csv()");
        FLOG_DEBUG(
          "As a data scientist, reading and writing data from/to CSV is one of the most common"
          " tasks I do on the daily. R, my language of choice, makes this easy with read.csv() "
          "and write.csv()");
    }
    std::chrono::steady_clock::time_point t2 = std::chrono::steady_clock::now();
    for (int i = 0; i < 30; ++i)
    {
        std::cout
          << "[2021-06-28 11:41:52][000000000000][debug][main]@(test_main.cpp,20) OK TEST\n";
        std::cout
          << "[2021-06-28 11:41:52][000000000000][debug][main]@(test_main.cpp,20) OK TEST\n";
        std::cout
          << "[2021-06-28 11:41:52][000000000000][debug][main]@(test_main.cpp,20) OK TEST\n";
        std::cout
          << "[2021-06-28 11:41:52][000000000000][debug][main]@(test_main.cpp,20) OK TEST\n";
        std::cout
          << "[2021-06-28 11:41:52][000000000000][debug][main]@(test_main.cpp,20) OK TEST\n";
        std::cout
          << "[2021-06-28 11:41:52][000000000000][debug][main]@(test_main.cpp,20) OK TEST\n";
        std::cout
          << "[2021-06-28 11:41:52][000000000000][debug][main]@(test_main.cpp,20) OK TEST\n";
        std::cout
          << "[2021-06-28 11:41:52][000000000000][debug][main]@(test_main.cpp,20) OK TEST\n";
        std::cout
          << "[2021-06-28 11:41:52][000000000000][debug][main]@(test_main.cpp,20) OK TEST\n";
        std::cout
          << "[2021-06-28 11:41:52][000000000000][debug][main]@(test_main.cpp,20) OK TEST\n";
    }
    std::chrono::steady_clock::time_point t3 = std::chrono::steady_clock::now();

    std::stringstream s;
    s << std::hex << std::this_thread::get_id() << '\n';
    std::cout << "thread: " << s.str();

    std::cout << "time (ns): "
              << std::chrono::duration_cast<std::chrono::nanoseconds>((t2 - t1)).count() / 300
              << '\n'
              << "base (ns): "
              << std::chrono::duration_cast<std::chrono::nanoseconds>((t3 - t2)).count() / 300
              << "ns ("
              << static_cast<float>(
                   std::chrono::duration_cast<std::chrono::nanoseconds>((t2 - t1)).count())
                   / static_cast<float>(
                     std::chrono::duration_cast<std::chrono::nanoseconds>((t3 - t2)).count())
              << ")\n";
}
