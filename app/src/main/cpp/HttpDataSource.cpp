#include "HttpDataSource.h"
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <cstring>
#include <android/log.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define LOG_TAG "HttpDataSource"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)

HttpDataSource::HttpDataSource() = default;

HttpDataSource::~HttpDataSource() {
    close();
}

bool
HttpDataSource::parseUrl(const std::string &url, std::string &host, std::string &path, int &port) {
    std::string temp = url;
    if (temp.find("http://") == 0) temp = temp.substr(7);

    size_t pathPos = temp.find('/');
    if (pathPos != std::string::npos) {
        host = temp.substr(0, pathPos);
        path = temp.substr(pathPos);
    } else {
        host = temp;
        path = "/";
    }

    size_t portPos = host.find(':');
    if (portPos != std::string::npos) {
        port = std::stoi(host.substr(portPos + 1));
        host = host.substr(0, portPos);
    } else {
        port = 80;
    }
    return !host.empty();
}

bool HttpDataSource::open(const std::string &url) {
    close();
    mUrl = url;

    std::string host, path;
    int port = 80;
    if (!parseUrl(mUrl, host, path, port)) return false;

    struct hostent *server = gethostbyname(host.c_str());
    if (!server) return false;

    mSocketFd = socket(AF_INET, SOCK_STREAM, 0);
    if (mSocketFd < 0) return false;

    struct sockaddr_in serv_addr{};
    serv_addr.sin_family = AF_INET;
    std::memcpy(&serv_addr.sin_addr.s_addr, server->h_addr, server->h_length);
    serv_addr.sin_port = htons(port);

    if (connect(mSocketFd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
        ::close(mSocketFd);
        mSocketFd = -1;
        return false;
    }

    std::string request = "GET " + path + " HTTP/1.0\r\nHost: " + host +
                          "\r\nUser-Agent: NativePlayer/1.0\r\n\r\n";
    send(mSocketFd, request.c_str(), request.length(), 0);

    // Strip HTTP Headers
    char buffer[1024];
    std::string headers;
    while (true) {
        ssize_t bytes = recv(mSocketFd, buffer, 1, 0);
        if (bytes <= 0) break;
        headers += buffer[0];
        if (headers.find("\r\n\r\n") != std::string::npos) break;
    }

    mIsRunning = true;
    LOGI("HTTP connection established for %s", host.c_str());
    return true;
}

ssize_t HttpDataSource::read(uint8_t *destination, size_t maxBytes) {
    if (mSocketFd < 0 || !mIsRunning) return -1;
    return recv(mSocketFd, destination, maxBytes, 0);
}

void HttpDataSource::close() {
    mIsRunning = false;
    if (mSocketFd >= 0) {
        ::close(mSocketFd);
        mSocketFd = -1;
    }
}