// Copyright (C) 2024 Ryan Bester

#include "dbc.hpp"

#include "main.hpp"

#include <iostream>
#include <sstream>

namespace canary {
    dbcfile dbcparser::load_dbc_file(const std::string &file_path) {
        dbcfile dbc;

        std::ifstream file(file_path);

        std::string line;
        long long last_message_id = 0;
        while (std::getline(file, line)) {
            if (line.starts_with("BO_")) { // Message
                auto parts = split_string(line, " ");

                std::string suffix = ":";

                auto can_id = parts[1];
                auto name = parts[2].substr(0, parts[2].length() - suffix.length());
                auto length = parts[3];
                auto sender = parts[4];

                try {
                    dbc_message message = dbc_message(std::stoll(can_id), name, std::stoi(length), sender);
                    dbc.messages.insert(std::make_pair(std::stoll(can_id), message));
                    last_message_id = message.can_id;
                } catch (const std::out_of_range &e) {
                    std::cerr << "CAN ID out of range: " << can_id << " - " << e.what() << std::endl;
                    continue;
                }

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

                try {
                    dbc.messages.at(last_message_id).signals.push_back(signal);
                } catch (const std::out_of_range &e) {
                    std::cerr << "Out of range " << e.what() << std::endl;
                }
            }
        }

        return dbc;
    }

    std::optional<dbc_message> dbcfile::find_message(std::string can_id_hex) {
        if (can_id_map.contains(can_id_hex)) {
            auto can_id = can_id_map[can_id_hex];
            if (can_id == 0) return std::nullopt;
            return messages[can_id];
        }

        for (const auto &[can_id, message]: messages) {
            // DBC file represents IDs as a decimal, convert here to a hex string
            std::stringstream stream;
            stream << std::hex << can_id;
            std::string dbc_can_id_hex(stream.str());

            // TODO: Use dbc_options_first_n variable
            // TODO: Skip first character for now, until offset implemented
            auto can_id_first_n = dbc_can_id_hex.substr(1, 4);
            auto to_find_first_n = can_id_hex.substr(1, 4);

            std::transform(can_id_first_n.begin(), can_id_first_n.end(), can_id_first_n.begin(),
                           ::toupper);
            std::transform(to_find_first_n.begin(), to_find_first_n.end(), to_find_first_n.begin(),
                           ::toupper);

            if (can_id_first_n == to_find_first_n) {
                can_id_map[can_id_hex] = can_id;
                return message;
            }
        }

        can_id_map[can_id_hex] = 0;
        return std::nullopt;
    }
}

