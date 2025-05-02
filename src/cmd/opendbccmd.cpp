// Copyright (C) 2025 Ryan Bester

#include "commanddispatcher.hpp"

#include "opendbccmd.hpp"
#include "../main.hpp"

#include <fstream>

const std::string canary::command::open_dbc_cmd::get_name() const {
    return "opendbc";
}

const std::string canary::command::open_dbc_cmd::get_description() const {
    return "Opens a DBC file";
}

const std::unordered_set<std::string> canary::command::open_dbc_cmd::get_args() const {
    return {};
}

const std::unordered_set<std::string> canary::command::open_dbc_cmd::get_opts() const {
    return {"path"};
}

int
canary::command::open_dbc_cmd::execute(const canary::command::parsed_command &args, canary::command::command_line &out) {
    std::string path;
    if (args.options.contains("path")) path = args.options.at("path");
    else {
        out.error("Path not specified");
        return 1;
    }

    get_gui()->m_state.dbc_file = canary::dbcparser::load_dbc_file(path);

    out.print("Opened DBC file: {}", path);

    return 0;
}
