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
#include <fcntl.h>
#endif

#include "config.hpp"

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

        if (APP_CONFIG.conn_opts.non_blocking) {
            set_non_blocking();
        }

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
            int err = WSAGetLastError();
            if (err != WSAEWOULDBLOCK) {
                if (m_error_handler)
                    m_error_handler(std::format("connect: Connection to the server failed. Code: {}", err));
                cleanup();
                return 1;
            }

        }
#else
        if (res < 0) {
            if (errno != EINPROGRESS) {
                if (m_error_handler) m_error_handler("Connection failed");
                return 1;
            }
        }
#endif

        m_connected = true;

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

    int socket::recv_when_ready(char *buf, int len, int cooldown) {
        if (cooldown == -1) {
            cooldown = APP_CONFIG.conn_opts.cooldown;
        }

        bool read = false, write = false;
        while (!read) {
            select(false, read, write);

            int optval;
            int optlen = sizeof(optval);
            if (getsockopt(m_fd, SOL_SOCKET, SO_ERROR, (char *) &optval, &optlen) < 0) {
                if (m_error_handler) m_error_handler(std::format("recv_when_ready: getsockopt failed with error: {}", WSAGetLastError()));
                return false;
            }
            if (optval != 0) {
                // Connection failed
                if (m_error_handler) m_error_handler(std::format("recv_when_ready: Connect failed with error: {}", optval));
                return false;
            }

            if (read) {
                break;
            }

#if defined(WIN32)
            Sleep(cooldown);
#else
            usleep(cooldown * 1000);
#endif
        }

        return recv(buf, len);
    }

    int socket::send_when_ready(const char *buf, int len, int cooldown) {
        if (cooldown == -1) {
            cooldown = APP_CONFIG.conn_opts.cooldown;
        }

        bool read = false, write = false;
        while (!write) {
            select(true, read, write);
            if (write) {
                break;
            }

#if defined(WIN32)
            Sleep(cooldown);
#else
            usleep(cooldown * 1000);
#endif
        }

        return send(buf, len);
    }

    int socket::select(bool want_write, bool &read, bool &write) {
        fd_set read_fds, write_fds;
        FD_ZERO(&read_fds);
        FD_ZERO(&write_fds);

        FD_SET(m_fd, &read_fds);
        if (want_write) FD_SET(m_fd, &write_fds);

        struct timeval timeout;
        timeout.tv_sec = APP_CONFIG.conn_opts.timeout;
        timeout.tv_usec = 0;

        int activity = ::select(m_fd + 1, &read_fds, &write_fds, nullptr, &timeout);
        if (activity < 0) {
            if (m_error_handler) m_error_handler("select: Error in select");
            return activity;
        }

        if (activity == 0) {
            // No activity detected
            return activity;
        }

        read = false;
        write = false;

        if (FD_ISSET(m_fd, &read_fds)) {
            read = true;
        }

        if (FD_ISSET(m_fd, &write_fds)) {
            write = true;
        }

        return 1;
    }

    void socket::close() {
        on_close();

#if defined(WIN32)
        closesocket(m_fd);
        cleanup();
#else
        ::close(m_fd);
#endif

        m_connected = false;
    }

    void socket::set_non_blocking() {
#if defined(WIN32)
        u_long mode = 1;
        ioctlsocket(m_fd, FIONBIO, &mode);
#else
        int flags = fcntl(m_fd, F_GETFL, 0);
        fcntl(m_fd, F_SETFL, flags | O_NONBLOCK);
#endif
    }

    void socket::cleanup() {
#if defined(WIN32)
        WSACleanup();
#endif
    }

    int socketcand::on_connect() {
        char buffer[256] = {0};

        int n = recv_when_ready(buffer, sizeof(buffer) - 1);
        if (n <= 0) {
            return n;
        }

        auto init_msg = std::stringstream();

        init_msg << "< open " << m_interface << " >\n";
        send_when_ready(init_msg.str().c_str(), strlen(init_msg.str().c_str()));

        const char *rawmode_msg = "< rawmode >\n";
        send_when_ready(rawmode_msg, strlen(rawmode_msg));

        return socket::on_connect();
    }
}
