#pragma once

#include <string>
#include <thread>
#include <atomic>
#include <vector>

class HttpDataSource {
public:
    HttpDataSource();

    ~HttpDataSource();

    bool open(const std::string &url);

    void close();

    ssize_t read(uint8_t *destination, size_t maxBytes);

private:
    static bool parseUrl(const std::string &url, std::string &host, std::string &path, int &port);

    std::string mUrl;
    std::thread mThread;
    std::atomic<bool> mIsRunning{false};
    int mSocketFd{-1};
};