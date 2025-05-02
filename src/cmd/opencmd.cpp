// Copyright (C) 2025 Ryan Bester

#include "commanddispatcher.hpp"

#include "opencmd.hpp"
#include "../main.hpp"

#include <fstream>

const std::string canary::command::open_cmd::get_name() const {
    return "open";
}

const std::string canary::command::open_cmd::get_description() const {
    return "Opens a saved file";
}

const std::unordered_set<std::string> canary::command::open_cmd::get_args() const {
    return {};
}

const std::unordered_set<std::string> canary::command::open_cmd::get_opts() const {
    return {"path"};
}

int
canary::command::open_cmd::execute(const canary::command::parsed_command &args, canary::command::command_line &out) {
    std::string path;
    if (args.options.contains("path")) path = args.options.at("path");
    else {
        out.error("Path not specified");
        return 1;
    }

    std::ifstream file(path);

    std::string line;
    while (std::getline(file, line)) {
        get_gui()->m_packet_provider.add_packet(line);
    }

    out.print("Opened file: {}", path);

    return 0;
}
