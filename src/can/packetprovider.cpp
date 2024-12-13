// Copyright (C) 2024 Ryan Bester

#include "packetprovider.hpp"

namespace canary::can {
    const std::vector<std::string> &packetprovider::get_received_packets() {
        return received_packets;
    }

    void packetprovider::add_packet(const std::string &packet) {
        received_packets.push_back(packet);
    }

    void packetprovider::clear_packets() {
        received_packets.clear();
    }
}
