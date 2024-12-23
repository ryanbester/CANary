// Copyright (C) 2024 Ryan Bester

#ifndef __CANARY_COMMAND_HELP__
#define __CANARY_COMMAND_HELP__

#include "commandbase.hpp"

namespace canary::command {
    class help_cmd : public command_base {
    public:
        [[nodiscard]] const std::string get_name() const override;

        [[nodiscard]] const std::unordered_set<std::string> get_args() const override;

        [[nodiscard]] const std::unordered_set<std::string> get_opts() const override;

        int execute(const parsed_command &args, command_line &out) override;
    };
}

#endif
