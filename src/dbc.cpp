// Copyright (C) 2024 Ryan Bester

#include "dbc.hpp"

#include "main.hpp"

#include <iostream>

namespace canary {
    dbcfile dbcparser::load_dbc_file(const std::string &file_path) {
        dbcfile dbc;

        std::ifstream file(file_path);

        std::string line;
        long last_message_id = 0;
        while (std::getline(file, line)) {
            if (line.starts_with("BO_")) { // Message
                auto parts = split_string(line, " ");

                std::string suffix = ":";

                auto can_id = parts[1];
                auto name = parts[2].substr(0, parts[2].length() - suffix.length());
                auto length = parts[3];
                auto sender = parts[4];

                dbc_message message = dbc_message(std::stol(can_id), name, std::stoi(length), sender);
                dbc.messages.insert(std::make_pair(std::stol(can_id), message));
                last_message_id = message.can_id;

                // TODO: Some lines start with a space
            } else if (line.starts_with(" SG_")) { // Signal
                auto parts = split_string(line, " ");

                auto name = parts[2];

                auto bit_layout_parts = split_string(parts[4], "@");
                auto start_end = split_string(bit_layout_parts[0], "|");

                auto start = start_end[0];
                auto length = start_end[1];
                auto little_endian = (bit_layout_parts[1][0] == '1');
                auto is_signed = (bit_layout_parts[1][1] == '-');

                auto scale_offset_parts = split_string(parts[5].substr(1, parts[5].length() - 2), ",");

                auto scale = scale_offset_parts[0];
                auto offset = scale_offset_parts[1];

                auto min_max_parts = split_string(parts[6].substr(1, parts[6].length() - 2), "|");

                auto min = min_max_parts[0];
                auto max = min_max_parts[1];

                auto unit = parts[7];
                unit.erase(std::remove(unit.begin(), unit.end(), '"'), unit.end());

                auto receiver = parts[9];

                dbc_signal signal(
                        name, std::stoi(start), std::stoi(length), little_endian, is_signed,
                        std::stof(scale), std::stof(offset), std::stof(min), std::stof(max),
                        unit, receiver
                        );

                dbc.messages.at(last_message_id).signals.push_back(signal);
            }
        }

        return dbc;
    }
}

