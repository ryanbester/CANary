// Copyright (C) 2024 Ryan Bester

#include "commanddispatcher.hpp"

#include "helpcmd.hpp"

#include <algorithm>

namespace canary::command {

    const std::string help_cmd::get_name() const {
        return "help";
    }

    const std::string help_cmd::get_description() const {
        return "Displays help information.";
    }

    const std::unordered_set<std::string> help_cmd::get_args() const {
        auto cmds = m_cmd_dispatcher.get_registered_commands();
        std::unordered_set<std::string> valid_args;

        std::transform(cmds.begin(), cmds.end(), std::inserter(valid_args, valid_args.end()), [](const auto &pair) {
            return pair.first;
        });

        return valid_args;
    }

    const std::unordered_set<std::string> help_cmd::get_opts() const {
        return std::unordered_set<std::string>();
    }

    int help_cmd::execute(const parsed_command &args, command_line &out) {
        if (args.args.empty()) {
            out.print("Valid commands:");

            auto cmds = m_cmd_dispatcher.get_registered_commands();
            for (const auto &cmd: cmds) {
                out.print(cmd.first);
            }
        } else {
            auto target_cmd = args.args[0];

            auto cmds = m_cmd_dispatcher.get_registered_commands();
            if (!cmds.contains(target_cmd)) {
                out.error("Specified command does not exist");
                return 1;
            }

            auto cmd = cmds.at(target_cmd);

            out.print("Help for \"{}\"", cmd->get_name());
            out.print("");

            out.print(cmd->get_description());
            out.print("");

            out.print("Arguments:");
            for (const auto &arg : cmd->get_args()) {
                out.print("    {}", arg);
            }
            out.print("");

            out.print("Options:");
            for (const auto &opt : cmd->get_opts()) {
                out.print("    {}", opt);
            }
        }

        return 0;
    }
}
