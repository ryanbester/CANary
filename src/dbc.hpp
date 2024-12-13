// Copyright (C) 2024 Ryan Bester

#ifndef __CANARY_DBC__
#define __CANARY_DBC__

#include <string>
#include <fstream>
#include <vector>
#include <unordered_map>

namespace canary {
    struct dbc_signal {
        std::string name;
        int start;
        int length;
        bool little_endian;
        bool is_signed;
        float scale;
        float offset;
        float min;
        float max;
        std::string unit;
        std::string receiver;

        dbc_signal(const std::string &name, int start, int length, bool littleEndian, bool isSigned, float scale,
                   float offset, float min, float max, const std::string &unit, const std::string &receiver) : name(
                name), start(start), length(length), little_endian(littleEndian), is_signed(isSigned), scale(scale),
                                                                                                               offset(offset),
                                                                                                               min(min),
                                                                                                               max(max),
                                                                                                               unit(unit),
                                                                                                               receiver(
                                                                                                                       receiver) {}
    };

    struct dbc_message {
        long can_id;
        std::string name;
        int length;
        std::string sender;

        std::vector<dbc_signal> signals;

        dbc_message() = default;

        dbc_message(long canId, const std::string &name, int length, const std::string &sender) : can_id(canId),
                                                                                                 name(name),
                                                                                                 length(length),
                                                                                                 sender(sender) {};
    };

    class dbcfile {
    public:
        std::unordered_map<long, dbc_message> messages;
    };

    class dbcparser {
    public:
        static dbcfile load_dbc_file(const std::string &file_path);
    };

}

#endif
