// Copyright (C) 2024 Ryan Bester

#ifndef __CANARY_GUI__
#define __CANARY_GUI__

#include <memory>
#include <fstream>
#include <unordered_map>
#include <map>

#include "../main.hpp"

#include "GLFW/glfw3.h"

#include "imgui.h"
#include "imgui_internal.h"

#include "../can/packetprovider.hpp"
#include "../dbc.hpp"
#include "../config.hpp"
#include "../cmd/commanddispatcher.hpp"

namespace canary::gui {

    template<typename K, typename V>
    V &state_at_or_init(std::unordered_map<K, V> &map, const K &key, const V &default_value = V()) {
        if (map.count(key) < 1) {
            // Create key if it doesn't exist
            map.insert(std::make_pair(key, default_value));
        }

        return map.at(key);
    }

    struct search_options {
        int search_start = 0;
        int search_end = 0;
        std::vector<std::tuple<std::string, std::string>> search_1_ids{};
        std::vector<std::tuple<std::string, std::string>> search_2_ids{};
        int search_num = 1;
    };

    struct packet_view_options {
        bool auto_scroll;
        bool paused;
        int selected_row;
        std::pair<dbc_message, std::string> selected_frame;
    };

    struct dbc_options {
        int first_n = -1;
        int offset = 0;
    };

    struct state {
        std::unordered_map<std::string, bool> open_dialogs;
        std::vector<std::string> file_dialogs;
        dbc_options dbc_opt;
        packet_view_options packet_view_opts;
        canary::dbcfile dbc_file;
        bool packet_filter_enabled = true;
        search_options search_opts;
        int speed = 0;
        int rpm = 0;
    };

    class gui {
    public:
        GLFWwindow *m_win;
        canary::can::packetprovider &m_packet_provider;
        canary::command::command_dispatcher &m_command_dispatcher;
        bool m_first_loop = true;
        bool m_first_run;

        ImFont *font_normal;
        ImFont *font_monospace;

        state m_state;

        gui(GLFWwindow *win, canary::can::packetprovider &packet_provider,
            canary::command::command_dispatcher &command_dispatcher)
                : m_win(win),
                  m_packet_provider(packet_provider),
                  m_command_dispatcher(command_dispatcher) {
            std::ifstream ini_file("imgui.ini");
            m_first_run = !ini_file.good();
            ini_file.close();
        }

        // Loads UI options from config
        void load_options();

        static float get_monitor_scale();

        void set_scale(ImGuiIO &io, float font_size, float scale_factor);

        void render_frame();

        // Saves UI options to config
        void save_options();

    private:
        ImVec2 render_menu_bar();

        void setup_docking(const ImVec2 menu_bar_size);

        void show_packets();

        void show_reset_window_pos_dlg();

        void show_dbc_options_win();

        void show_file_dialogs();

        void show_frame_properties();

        void show_search();

        void show_filter();

        void show_gauges();

        void show_tools();
    };

}

#endif
