// Copyright (C) 2024 Ryan Bester

#include "helpcmd.hpp"
#include "commanddispatcher.hpp"

#include <iostream>

namespace canary::command {

    const std::string help_cmd::get_name() const {
        return "help";
    }

    int help_cmd::execute(const parsed_command &args, command_line &out) {
        out.print("Help:");

        return 0;
    }

    const std::unordered_set<std::string> help_cmd::get_args() const {
        return {"cmd"};
    }

    const std::unordered_set<std::string> help_cmd::get_opts() const {
        return std::unordered_set<std::string>();
    }
}
