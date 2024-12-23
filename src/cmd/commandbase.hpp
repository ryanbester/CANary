// Copyright (C) 2024 Ryan Bester

#ifndef __CANARY_COMMAND_BASE__
#define __CANARY_COMMAND_BASE__

#include <string>
#include <vector>
#include <unordered_map>
#include <unordered_set>

namespace canary::command {
    struct parsed_command {
        std::string cmd_name;
        std::vector<std::string> args;
        std::unordered_map<std::string, std::string> options;
    };

    class command_line;

    class command_base {
    public:
        virtual ~command_base() = default;

        [[nodiscard]] virtual const std::string get_name() const = 0;

        [[nodiscard]] virtual const std::unordered_set<std::string> get_args() const = 0;

        [[nodiscard]] virtual const std::unordered_set<std::string> get_opts() const = 0;

        virtual int execute(const parsed_command &args, command_line &out) = 0;
    };
}

#endif
