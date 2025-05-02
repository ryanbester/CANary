// Copyright (C) 2024 Ryan Bester

#ifndef __CANARY_CONNS__
#define __CANARY_CONNS__

#include <vector>
#include <thread>

#include "socket.hpp"
#include "can/packetprovider.hpp"


namespace canary {

    class connection {
    public:
        socket m_socket;
        canary::can::packetprovider m_packet_provider;
        std::thread &m_thread;
    };

    class conns {
    public:
        static std::vector<connection> connections;
    };
}

#endif
