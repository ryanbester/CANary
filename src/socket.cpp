// Copyright (C) 2024 Ryan Bester

#include "socket.hpp"

#include <cstring>
#include <format>
#include <sstream>

#if defined(WIN32)
#include <winsock2.h>
#include <ws2tcpip.h>
#else
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#endif

namespace canary {
    int socket::connect() {
        struct sockaddr_in serv_addr;
        char buffer[1024];

#if defined(WIN32)
        WSADATA wsa;
        if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
            if (m_error_handler)
                m_error_handler(std::format("connect: WinSock initialisation failed. Code: {}", WSAGetLastError()));
            return 1;
        }
#endif
        m_fd = ::socket(AF_INET, SOCK_STREAM, 0);

#if defined(WIN32)
        if (m_fd == INVALID_SOCKET) {
            if (m_error_handler)
                m_error_handler(std::format("connect: WinSock socket creation failed. Code: {}", WSAGetLastError()));
            cleanup();
            return 1;
        }
#else
        if (m_fd < 0) {
            if (m_error_handler) m_error_handler("Error opening socket");
            return 1;
        }
#endif

        memset(&serv_addr, 0, sizeof(serv_addr));
        serv_addr.sin_family = AF_INET;
        serv_addr.sin_port = htons(m_port);

        if (inet_pton(AF_INET, m_host.c_str(), &serv_addr.sin_addr) <= 0) {
            if (m_error_handler) m_error_handler("connect: Invalid address or address not supported");
            return 1;
        }

        int res = ::connect(m_fd, (struct sockaddr *) &serv_addr, sizeof(serv_addr));
#if defined(WIN32)
        if (res == SOCKET_ERROR) {
            if (m_error_handler)
                m_error_handler(std::format("connect: Connection to the server failed. Code: {}", WSAGetLastError()));
            cleanup();
            return 1;
        }
#else
        if (res < 0) {
            if (m_error_handler) m_error_handler("Connection failed");
            return 1;
        }
#endif

        return on_connect();
    }

    int socket::recv(char *buf, int len) {
        int n = ::recv(m_fd, buf, len, 0);
#if defined(WIN32)
        if (n == SOCKET_ERROR) {
            if (m_error_handler)
                m_error_handler(std::format("recv: Failed to receive data. Code: {}", WSAGetLastError()));
        }
#else
        if (n < 0) {
            if (m_error_handler) m_error_handler("Failed to receive data");
        }
#endif
        return n;
    }

    int socket::send(const char *buf, int len) {
        int n = ::send(m_fd, buf, len, 0);

#if defined(WIN32)
        if (n == SOCKET_ERROR) {
            if (m_error_handler)
                m_error_handler(std::format("send: Failed to send data. Code: {}", WSAGetLastError()));
        }
#else
        if (n < 0) {
            if (m_error_handler) m_error_handler("Failed to send data");
        }
#endif
        return n;
    }


    void socket::close() {
        on_close();

#if defined(WIN32)
        closesocket(m_fd);
        // TODO: call cleanup on every error code path above
        cleanup();
#else
        ::close(m_fd);
#endif
    }

    void socket::cleanup() {
#if defined(WIN32)
        WSACleanup();
#endif
    }

    int socketcand::on_connect() {
        // Receive hello message
        char buffer[256] = {0};

        int n = recv(buffer, sizeof(buffer) - 1);
        if (n <= 0) {
            return n;
        }

        auto init_msg = std::stringstream();

        init_msg << "< open " << m_interface << " >\n";
        send(init_msg.str().c_str(), strlen(init_msg.str().c_str()));

        const char *rawmode_msg = "< rawmode >\n";
        send(rawmode_msg, strlen(rawmode_msg));

        return socket::on_connect();
    }
}
