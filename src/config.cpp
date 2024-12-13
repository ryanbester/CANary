// Copyright (C) 2024 Ryan Bester

#include "config.hpp"

#include <fstream>

namespace canary {
    const std::string canary::config::config_loader::FILENAME = "config.json";

    canary::config::config canary::config::config_loader::app_config;

    void canary::config::config_loader::load_config() {
        std::ifstream i(config_loader::FILENAME);
        auto data = nlohmann::json::parse(i);
        config_loader::app_config = data.template get<canary::config::config>();
        i.close();
    }

    void canary::config::config_loader::save_config() {
        std::ofstream o(config_loader::FILENAME);
        nlohmann::json new_config = config_loader::app_config;
        o << new_config;
        o.close();
    }
}
