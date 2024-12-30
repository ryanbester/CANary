// Copyright (C) 2024 Ryan Bester

#ifndef __CANARY_CONFIG__

#define __CANARY_CONFIG__

#include <string>

#include <nlohmann/json.hpp>

#define APP_CONFIG canary::config::config_loader::app_config

namespace canary::config {

    struct ui_options {
        std::unordered_map<std::string, bool> open_dialogs;
    };

    struct connection {
        std::string name;
        std::string can_type;
        std::map<std::string, nlohmann::json> can_params;
        bool canaryd_enabled;
        std::string canaryd_host;
        int canaryd_port;
    };

    struct connection_options {
        bool non_blocking = true;
        int timeout = 5;
        int cooldown = 100;
    };

    struct config {
    public:
        ui_options ui_opts;
        std::vector<connection> connections;
        connection_options conn_opts;
    };

    class config_loader {
    public:
        static const std::string FILENAME;

        static config app_config;

        static void load_config();

        static void save_config();
    };

    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(canary::config::ui_options, open_dialogs);

    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(canary::config::connection, name, can_type, can_params, canaryd_enabled,
                                       canaryd_host, canaryd_port)

    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(canary::config::connection_options, non_blocking, timeout, cooldown)

    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(canary::config::config, ui_opts, connections, conn_opts)
}

#endif
