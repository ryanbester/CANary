// Copyright (C) 2024 Ryan Bester

#ifndef __CANARY_COMMAND_DISPATCHER__
#define __CANARY_COMMAND_DISPATCHER__

#include <string>
#include <unordered_map>
#include <vector>
#include <memory>
#include <iostream>
#include <format>
#include <sstream>

namespace canary::command {
    struct parsed_command;

    struct command_base;

    class command_line {
    public:
        template<typename... Args>
        inline void print(std::string_view fmt, Args &&... args) {
            std::ostringstream oss;
            oss << std::vformat(fmt, std::make_format_args(std::forward<Args>(args)...));
            m_lines.push_back(oss.str());
        }

        template<typename... Args>
        inline void warn(std::string_view fmt, Args &&... args) {
            std::ostringstream oss;
            oss << "[WARN] " << std::vformat(fmt, std::make_format_args(std::forward<Args>(args)...));
            m_lines.push_back(oss.str());
        }

        template<typename... Args>
        inline void error(std::string_view fmt, Args &&... args) {
            std::ostringstream oss;
            oss << "[ERROR] " << std::vformat(fmt, std::make_format_args(std::forward<Args>(args)...));
            m_lines.push_back(oss.str());
        }

        [[nodiscard]] const std::vector<std::string> &get_lines() const;

    private:
        std::vector<std::string> m_lines;
    };

    class command_dispatcher {
    public:
        int execute_command(const std::string &command_line);

        void register_command(std::shared_ptr<command_base> command);

        [[nodiscard]] parsed_command parse_command_line(const std::string &cmd_line) const;

        [[nodiscard]] const command_line &get_command_line() const;

    private:
        command_line m_out;

        std::unordered_map<std::string, std::shared_ptr<command_base>> m_commands;

        [[nodiscard]] std::string trim(const std::string &cmd_line) const;
    };


}

#endif
