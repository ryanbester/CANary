// Copyright (C) 2024 Ryan Bester

#include "commanddispatcher.hpp"
#include "commandbase.hpp"

#include <iostream>

namespace canary::command {
    int command_dispatcher::execute_command(const std::string &command_line) {
        auto parsed = parse_command_line(command_line);

        if (!m_commands.contains(parsed.cmd_name)) {
            // No command with that name
            m_out.error("Command not found: {}", parsed.cmd_name);
            return 0;
        }

        auto cmd = m_commands[parsed.cmd_name];

        auto valid_args = cmd->get_args();
        for (const auto &arg: parsed.args) {
            if (!valid_args.contains(arg)) {
                m_out.error("Unknown argument: {}", arg);
                return 0;
            }
        }

        auto valid_opts = cmd->get_opts();
        for (const auto &opt: parsed.options) {
            if (!valid_opts.contains(opt.first)) {
                m_out.error("Unknown option: {}", opt.first);
                return 0;
            }
        }

        m_out.print("> {}", command_line);
        int res = cmd->execute(parsed, m_out);
        m_out.print("-> {}", res);

        return res;
    }

    void command_dispatcher::register_command(std::shared_ptr<command_base> command) {
        m_commands[command->get_name()] = command;
    }

    std::string command_dispatcher::trim(const std::string &cmd_line) const {
        size_t first = cmd_line.find_first_not_of(' ');
        if (first == std::string::npos) {
            // Command line is all whitespace
            return "";
        }
        size_t last = cmd_line.find_last_not_of(' ');
        return cmd_line.substr(first, last - first + 1);
    }

    parsed_command command_dispatcher::parse_command_line(const std::string &cmd_line) const {
        parsed_command result;
        auto trimmed_cmd_line = trim(cmd_line);

        bool in_quotes(false);
        std::string temp;
        std::vector<std::string> tokens;

        for (const auto &c: trimmed_cmd_line) {
            if (std::isspace(c) && !in_quotes) {
                if (!temp.empty()) {
                    tokens.push_back(temp);
                    temp.clear();
                }
            } else if (c == '"') {
                in_quotes = !in_quotes;
            } else {
                temp += c;
            }
        }

        if (!temp.empty()) {
            tokens.push_back(temp);
        }

        if (!tokens.empty()) {
            result.cmd_name = tokens.front();
            tokens.erase(tokens.begin());
        }

        for (auto i = 0; i < tokens.size(); ++i) {
            if (tokens[i].starts_with("-")) {
                std::string opt_name = tokens[i];
                std::string opt_val;

                if (i + 1 < tokens.size() && !tokens[i + 1].starts_with("-")) {
                    opt_val = tokens[++i];
                }

                // Remove leading dashes from option name
                auto opt_pos = opt_name.find_first_not_of('-');
                if (opt_pos != std::string::npos) {
                    opt_name.erase(0, opt_pos);
                }

                result.options[opt_name] = opt_val;
            } else {
                result.args.push_back(tokens[i]);
            }
        }

        return result;
    }

    const command_line &command_dispatcher::get_command_line() const {
        return m_out;
    }

    const std::vector<std::string> &command_line::get_lines() const {
        return m_lines;
    }
}
