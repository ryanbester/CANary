// Copyright (C) 2024 Ryan Bester

#ifndef __CANARY_CONFIG__

#define __CANARY_CONFIG__

#include <string>

#include <nlohmann/json.hpp>

namespace canary::config {

    struct connection {
        std::string name;
        std::string type;
        std::map<std::string, nlohmann::json> params;
    };

    struct config {
    public:
        std::vector<connection> connections;
    };

    class config_loader {
    public:
        static const std::string FILENAME;

        static config app_config;

        static void load_config();

        static void save_config();
    };

    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(canary::config::connection, name, type, params)

    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(canary::config::config, connections)
}

#endif
