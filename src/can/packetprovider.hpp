// Copyright (C) 2024 Ryan Bester

#ifndef __CANARY_PACKETPROVIDER__
#define __CANARY_PACKETPROVIDER__

#include <vector>
#include <string>

namespace canary::can {

    class packetprovider {
    public:
        const std::vector<std::string> &get_received_packets();

        void add_packet(const std::string &packet);

        void clear_packets();

    private:
        std::vector<std::string> received_packets;
    };

}

#endif
