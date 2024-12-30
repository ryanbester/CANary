// Copyright (C) 2024 Ryan Bester

#ifndef __CANARY_COMMAND_HELP__
#define __CANARY_COMMAND_HELP__

#include "commandbase.hpp"

namespace canary::command {
    class command_dispatcher;

    class help_cmd : public command_base {
    public:
        explicit help_cmd(const command_dispatcher &cmd_dispatcher) : m_cmd_dispatcher(cmd_dispatcher) {};

        [[nodiscard]] const std::string get_name() const override;

        [[nodiscard]] const std::string get_description() const override;

        [[nodiscard]] const std::unordered_set<std::string> get_args() const override;

        [[nodiscard]] const std::unordered_set<std::string> get_opts() const override;

        int execute(const parsed_command &args, command_line &out) override;

    private:
        const command_dispatcher &m_cmd_dispatcher;
    };
}

#endif
