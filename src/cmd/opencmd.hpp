// Copyright (C) 2025 Ryan Bester

#ifndef __CANARY_COMMAND_OPEN__
#define __CANARY_COMMAND_OPEN__

#include "commandbase.hpp"

namespace canary::command {
    class open_cmd : public command_base {
        [[nodiscard]] const std::string get_name() const override;

        [[nodiscard]] const std::string get_description() const override;

        [[nodiscard]] const std::unordered_set<std::string> get_args() const override;

        [[nodiscard]] const std::unordered_set<std::string> get_opts() const override;

        int execute(const parsed_command &args, command_line &out) override;
    };
}

#endif
