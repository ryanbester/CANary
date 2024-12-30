// Copyright (C) 2024 Ryan Bester

#ifndef __CANARY_SOCKET__
#define __CANARY_SOCKET__

#include <string>
#include <utility>
#include <functional>

#include "config.hpp"

namespace canary {

    class socket {
    public:
        bool m_connected{false};

        socket(std::string host, int port) : m_host(std::move(host)), m_port(port) {};

        inline void set_error_handler(std::function<void(std::string message)> error_handler) {
            m_error_handler = std::move(error_handler);
        }

        int connect();

        // Should use recv_when_ready
        int recv(char *buf, int len);

        // Should use send_when_ready
        int send(const char *buf, int len);

        int recv_when_ready(char *buf, int len, int cooldown = -1);

        int send_when_ready(const char *buf, int len, int cooldown = -1);

        int select(bool want_write, bool &read, bool &write);

        void close();

    protected:
        virtual inline int on_connect() { return 0; };

        virtual inline void on_close() {};

    private:
        std::string m_host;
        int m_port;
        int m_fd{-1};

        std::function<void(std::string message)> m_error_handler{};

        void set_non_blocking();

        void cleanup();
    };

    class socketcand : public socket {
    public:
        socketcand(std::string host, int port, std::string socketcand_interface) : socket(std::move(host), port),
                                                                                   m_interface(std::move(
                                                                                           socketcand_interface)) {};

    protected:
        int on_connect() override;

    private:
        std::string m_interface;
    };

}
#endif
