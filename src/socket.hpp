// Copyright (C) 2024 Ryan Bester

#ifndef __CANARY_SOCKET__
#define __CANARY_SOCKET__

#include <string>
#include <utility>
#include <functional>

namespace canary {

    class socket {
    public:
        socket(std::string host, int port) : m_host(std::move(host)), m_port(port) {};

        inline void set_error_handler(std::function<void(std::string message)> error_handler) {
            m_error_handler = std::move(error_handler);
        }

        int connect();

        int recv(char *buf, int len);

        int send(const char *buf, int len);

        void close();

    protected:
        virtual inline int on_connect() { return 0; };

        virtual inline void on_close() {};

    private:
        std::string m_host;
        int m_port;
        int m_fd{-1};

        std::function<void(std::string message)> m_error_handler{};

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
