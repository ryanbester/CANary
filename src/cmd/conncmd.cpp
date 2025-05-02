// Copyright (C) 2024 Ryan Bester

#include "conncmd.hpp"
#include "commanddispatcher.hpp"

#include "../main.hpp"
#include "../conns.hpp"
#include "../socket.hpp"

#include <nlohmann/json.hpp>

#include <sstream>

namespace canary::command {
    const std::string conn_cmd::get_name() const {
        return "conns";
    }

    const std::string conn_cmd::get_description() const {
        return "Manages connections.";
    }

    const std::unordered_set<std::string> conn_cmd::get_args() const {
        return {"list", "details", "add", "edit", "delete", "connect", "disconnect"};
    }

    const std::unordered_set<std::string> conn_cmd::get_opts() const {
        return {"name", "can_type", "can_params", "canaryd_enabled", "canaryd_host", "canaryd_port", "n"};
    }

    int conn_cmd::execute(const parsed_command &args, command_line &out) {
        const auto &action = args[0];

        if (action == "list") {
            return list_action(args, out);
        } else if (action == "details") {
            return details_action(args, out);
        } else if (action == "add") {
            return add_action(args, out);
        } else if (action == "edit") {
            return edit_action(args, out);
        } else if (action == "delete") {
            return delete_action(args, out);
        } else if (action == "connect") {
            return connect_action(args, out);
        } else if (action == "disconnect") {
            return disconnect_action(args, out);
        }

        return 0;
    }

    int conn_cmd::get_item(const canary::command::parsed_command &args, canary::command::command_line &out,
                            canary::config::connection *&item) {
        item = nullptr;

        if (!args.options.contains("n")) {
            out.error("Item not specified");
            return -1;
        }

        int item_num = std::stoi(args.options.at("n"));

        if (item_num < 1 || item_num > APP_CONFIG.connections.size()) {
            out.error("Number out of range");
            return -1;
        }

        item = &APP_CONFIG.connections[item_num - 1];
        return item_num;
    }

    int conn_cmd::list_action(const canary::command::parsed_command &args, canary::command::command_line &out) {
        int count = APP_CONFIG.connections.size();
        out.print("The following {} connections have been defined:", count);

        int i = 1;
        for (const auto &conn: APP_CONFIG.connections) {
            out.print("{}) {}", i, conn.name);
            i++;
        }

        out.print("");
        out.print("View details with conns details -n [number]");

        return 0;
    }

    int conn_cmd::details_action(const canary::command::parsed_command &args, canary::command::command_line &out) {
        canary::config::connection *item;
        if (-1 == get_item(args, out, item)) {
            return 1;
        }

        print_conn_details(out, *item);

        return 0;
    }

    int conn_cmd::add_action(const parsed_command &args, command_line &out) {
        std::string name, can_type, can_params, canaryd_host;
        bool canaryd_enabled(false);
        int canaryd_port = -1;

        if (args.options.contains("name")) name = args.options.at("name");
        else {
            out.error("Name not specified");
            return 1;
        }

        if (args.options.contains("can_type")) can_type = args.options.at("can_type");
        else {
            out.error("CAN connection type not specified");
            return 1;
        }

        if (args.options.contains("can_params")) can_params = args.options.at("can_params");
        if (args.options.contains("canaryd_enabled")) {
            std::istringstream(args.options.at("canaryd_enabled")) >> std::boolalpha >> canaryd_enabled;
        }

        if (canaryd_enabled) {
            if (args.options.contains("canaryd_host")) canaryd_host = args.options.at("canaryd_host");
            if (args.options.contains("canaryd_port")) canaryd_port = std::stoi(args.options.at("canaryd_port"));
        }

        std::map<std::string, nlohmann::json> params;
        auto comma_split_params = split_string(can_params, ",");
        for (const auto &param_kv: comma_split_params) {
            auto kv_split_param = split_string(param_kv, "=");
            if (kv_split_param.size() != 2) continue;

            params[kv_split_param[0]] = kv_split_param[1];
        }

        canary::config::connection conn(name, can_type, params, canaryd_enabled, canaryd_host, canaryd_port);
        APP_CONFIG.connections.push_back(conn);

        out.print("Created the following connection:");
        print_conn_details(out, conn);

        return 0;
    }

    int conn_cmd::edit_action(const canary::command::parsed_command &args, canary::command::command_line &out) {
        canary::config::connection *item;
        if (-1 == get_item(args, out, item)) {
            return 1;
        }

        std::string name, can_type, can_params, canaryd_host;
        bool canaryd_enabled(false);
        int canaryd_port = -1;

        if (args.options.contains("name")) name = args.options.at("name");
        else {
            out.error("Name not specified");
            return 1;
        }

        if (args.options.contains("can_type")) can_type = args.options.at("can_type");
        else {
            out.error("CAN connection type not specified");
            return 1;
        }

        if (args.options.contains("can_params")) can_params = args.options.at("can_params");
        if (args.options.contains("canaryd_enabled")) {
            std::istringstream(args.options.at("canaryd_enabled")) >> std::boolalpha >> canaryd_enabled;
        }

        if (canaryd_enabled) {
            if (args.options.contains("canaryd_host")) canaryd_host = args.options.at("canaryd_host");
            if (args.options.contains("canaryd_port")) canaryd_port = std::stoi(args.options.at("canaryd_port"));
        }

        std::map<std::string, nlohmann::json> params;
        auto comma_split_params = split_string(can_params, ",");
        for (const auto &param_kv: comma_split_params) {
            auto kv_split_param = split_string(param_kv, "=");
            if (kv_split_param.size() != 2) continue;

            params[kv_split_param[0]] = kv_split_param[1];
        }

        canary::config::connection conn(name, can_type, params, canaryd_enabled, canaryd_host, canaryd_port);
        APP_CONFIG.connections.push_back(conn);

        out.print("Modified the following connection:");
        print_conn_details(out, conn);

        return 0;
    }

    int conn_cmd::delete_action(const canary::command::parsed_command &args, canary::command::command_line &out) {
        canary::config::connection *item;
        int item_num = get_item(args, out, item);
        if (-1 == item_num) {
            return 1;
        }

        canary::config::connection copied_item(*item);

        APP_CONFIG.connections.erase(APP_CONFIG.connections.begin() + (item_num - 1));

        out.print("Deleted the following connection:");
        print_conn_details(out, copied_item);

        return 0;
    }

    int conn_cmd::connect_action(const canary::command::parsed_command &args, canary::command::command_line &out) {
        canary::config::connection *item;
        if (-1 == get_item(args, out, item)) {
            return 1;
        }

        canary::socket socket(item->canaryd_host, item->canaryd_port);
        socket.set_error_handler([&out](const std::string &msg) {
            out.error("{}", msg);
        });

        int connect_res = socket.connect();
        if (connect_res != 0) {
            out.error("Error connecting to socket");
            return 1;
        }

        out.print("Connected to socket");

        return 0;
    }

    int conn_cmd::disconnect_action(const canary::command::parsed_command &args, canary::command::command_line &out) {
        canary::config::connection *item;
        if (-1 == get_item(args, out, item)) {
            return 1;
        }
        return 0;
    }

    void conn_cmd::print_conn_details(command_line &out, const config::connection &conn) {
        out.print("Name: {}", conn.name);
        out.print("CAN Type: {}", conn.can_type);

        out.print("CAN Params:");
        for (const auto &[key, value]: conn.can_params) {
            auto dumped_val = value.dump();
            out.print("    {}: {}", key, dumped_val);
        }

        out.print("CANaryd Enabled: {}", conn.canaryd_enabled);
        out.print("CANaryd Host: {}", conn.canaryd_host);
        out.print("CANaryd Port: {}", conn.canaryd_port);
    }
}
