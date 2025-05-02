// Copyright (C) 2024 Ryan Bester

#ifndef __CANARY_COMMAND_CONN__
#define __CANARY_COMMAND_CONN__

#include "commandbase.hpp"

#include "../config.hpp"

namespace canary::command {
    class conn_cmd : public command_base {
    public:
        [[nodiscard]] const std::string get_name() const override;

        [[nodiscard]] const std::string get_description() const override;

        [[nodiscard]] const std::unordered_set<std::string> get_args() const override;

        [[nodiscard]] const std::unordered_set<std::string> get_opts() const override;

        int execute(const parsed_command &args, command_line &out) override;

    private:
        int get_item(const canary::command::parsed_command &args, canary::command::command_line &out,
                      canary::config::connection *&item);

        int list_action(const parsed_command &args, command_line &out);

        int details_action(const parsed_command &args, command_line &out);

        int add_action(const parsed_command &args, command_line &out);

        int edit_action(const parsed_command &args, command_line &out);

        int delete_action(const parsed_command &args, command_line &out);

        int connect_action(const parsed_command &args, command_line &out);

        int disconnect_action(const parsed_command &args, command_line &out);

        void print_conn_details(command_line &out, const canary::config::connection &conn);
    };
}

#endif
