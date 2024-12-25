// Copyright (C) 2024 Ryan Bester

#include "gui.hpp"

#include "ImGuiFileDialog.h"

#include "cmdline.hpp"

#include <iostream>

#if defined(WIN32)
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX

#include <Windows.h>
#include <ShellScalingApi.h>

#endif

namespace canary::gui {

// TODO: Implement software filter
    std::vector<std::string> excluded_ids = {
            "10210040",
            "10220040",
            "1022A040",
            "1022C040",
            "1022E040",
            "10230040",
            "10240040",
            "10242040",
            "10244060",
            "10248040", // Battery voltage?
            "10250040",
            "10264040",
            "102A8097",
            "102AA097",
            "102AC097",
            "102C0040",
//        "102CA040", // Engine Information?
            "102CC040",
            "102CE040",
            "102D0040",
            "102E0040",
            "10304058",
            "10306099",
            "10308060",
            "10324058",
            "10448060", // Fuel level?
            "106B0040",
            "106B8040",
            "106C0040",
            "106D0080",
            "106E0097",
            "10708040",
            "10734099",
            "1077C040",
            "10780040",
            "10788040",
            "10800040",
            "10806040",
            "10814099",
            "108E0080",
            "10AE8060",
            "10EC4040",
            "10EC8040",
            "13FFE040",
            "13FFE058",
            "13FFE060",
            "13FFE066",
            "13FFE068",
            "13FFE080",
            "13FFE097",
            "13FFE099",
            "13FFE0BB",
            "621",
            "624",
            "62C",
            "10466040",
            "106D4099"
    };

    float gui::get_monitor_scale() {
        float scale = 1.0f;

#if defined(WIN32)
        auto primary_monitor = MonitorFromPoint(POINT{0, 0}, MONITOR_DEFAULTTOPRIMARY);
        UINT dpi_x = 0, dpi_y = 0;

        auto result = GetDpiForMonitor(primary_monitor, MDT_EFFECTIVE_DPI, &dpi_x, &dpi_y);
        if (result == S_OK) {
            scale = static_cast<float>(dpi_x) / 96.0f;
        } else {
            std::cout << "Failed to retrieve DPI, defaulting to 100% scaling" << std::endl;
            scale = 1.0f;
        }
#endif

        return scale;
    }

    void gui::set_scale(ImGuiIO &io, float font_size, float scale_factor) {
        auto &style = ImGui::GetStyle();
        style.ScaleAllSizes(scale_factor);

        io.Fonts->Clear();
        font_normal = io.Fonts->AddFontFromFileTTF("resources/fonts/Roboto-Medium.ttf", font_size * scale_factor);
        font_monospace = io.Fonts->AddFontFromFileTTF("resources/fonts/ProggyClean.ttf", font_size * scale_factor);

        // Rebuild the font atlas
        unsigned char *pixels;
        int width, height;
        io.Fonts->GetTexDataAsRGBA32(&pixels, &width, &height);

        // Upload texture
        GLuint font_texture;
        glGenTextures(1, &font_texture);
        glBindTexture(GL_TEXTURE_2D, font_texture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        io.Fonts->TexID = font_texture;

        io.DisplayFramebufferScale = ImVec2(scale_factor, scale_factor);
    }

    void gui::render_frame() {

        // TODO: menu_bar_size in class member
        const ImVec2 menu_bar_size = render_menu_bar();
        setup_docking(menu_bar_size);

        show_packets();

        show_reset_window_pos_dlg();
        show_dbc_options_win();
        show_file_dialogs();
        show_frame_properties();
        show_search();
        show_filter();
        show_conn_mgr();
        show_conn_mgr_edit_dlg();
        show_gauges();
        show_tools();

        cmdline::show_command_line(*this);

        m_first_loop = false;
    }

    ImVec2 gui::render_menu_bar() {

        // TODO: Temp
        auto file_path = std::string("output.dat");

        ImVec2 menu_bar_size;

        if (ImGui::BeginMainMenuBar()) {
            if (ImGui::BeginMenu("File")) {
                if (ImGui::MenuItem("Create")) {
                }
                if (ImGui::MenuItem("Open", "Ctrl+O")) {
                    std::ifstream file(file_path);

                    std::string line;
                    while (std::getline(file, line)) {
                        m_packet_provider.add_packet(line);
                    }

                    IGFD::FileDialogConfig config;
                    config.path = ".";
                    m_state.file_dialogs.emplace_back("ChooseFileDlgKey");
                    ImGuiFileDialog::Instance()->OpenDialog("ChooseFileDlgKey", "Choose File", ".cpp,.h,.hpp", config);
                }
                if (ImGui::MenuItem("Save", "Ctrl+S")) {
                    std::ofstream file(file_path);

                    for (const auto &frame: m_packet_provider.get_received_packets()) {
                        file << frame << std::endl;
                    }

                    file.close();
                }
                if (ImGui::MenuItem("Save as..")) {
                }
                ImGui::EndMenu();
            }
            if (ImGui::BeginMenu("Edit")) {
                ImGui::EndMenu();
            }
            if (ImGui::BeginMenu("View")) {
                if (ImGui::MenuItem("Reset Window Positions")) {
                    m_state.open_dialogs["reset_window_pos"] = true;
                }

                ImGui::EndMenu();
            }
            if (ImGui::BeginMenu("DBC")) {
                if (ImGui::MenuItem("Open DBC")) {
                    IGFD::FileDialogConfig config;
                    config.path = ".";
                    m_state.file_dialogs.emplace_back("ChooseDbcFileDlgKey");
                    ImGuiFileDialog::Instance()->OpenDialog("ChooseDbcFileDlgKey", "Choose DBC File", ".dbc", config);
                }
                ImGui::MenuItem("Options...", nullptr,
                                &state_at_or_init(m_state.open_dialogs, std::string("dbc_options_win")));
                ImGui::EndMenu();
            }
            if (ImGui::BeginMenu("Tools")) {
                ImGui::MenuItem("Connection Manager...", nullptr,
                                &state_at_or_init(m_state.open_dialogs, std::string("connection_mgr")));
                ImGui::EndMenu();
            }
            if (ImGui::BeginMenu("Help")) {
                ImGui::MenuItem("Help");
                ImGui::MenuItem("About");

                ImGui::EndMenu();
            }
            menu_bar_size = ImGui::GetWindowSize();
            ImGui::EndMainMenuBar();
        }

        return menu_bar_size;
    }

    void gui::setup_docking(ImVec2 menu_bar_size) {
        if (m_first_loop && m_first_run) {
            ImVec2 work_pos = ImGui::GetMainViewport()->WorkPos;
            work_pos.y += menu_bar_size.y;
            ImVec2 work_size = ImGui::GetMainViewport()->WorkSize;
            work_size.y -= menu_bar_size.y;
            ImVec2 work_center{work_pos.x + work_size.x * 0.5f, work_pos.y + work_size.y * 0.5f};

            ImGuiID docker_id = ImGui::GetID("MainWindowGroup");
            ImGui::DockBuilderRemoveNode(docker_id);
            ImGui::DockBuilderAddNode(docker_id);

            ImVec2 node_pos{work_center.x - work_size.x * 0.5f, work_center.y - work_size.y * 0.5f};

            ImGui::DockBuilderSetNodeSize(docker_id, work_size);
            ImGui::DockBuilderSetNodePos(docker_id, node_pos);

            ImGuiID main_dock = ImGui::DockBuilderSplitNode(docker_id, ImGuiDir_Left,
                                                            0.7f, nullptr, &docker_id);
            ImGuiID right_pane = ImGui::DockBuilderSplitNode(docker_id, ImGuiDir_Right,
                                                             0.3f, nullptr, &docker_id);

            ImGuiID main_top_pane = ImGui::DockBuilderSplitNode(main_dock, ImGuiDir_Up,
                                                                0.2f, nullptr, &main_dock);
            ImGuiID right_bottom_pane = ImGui::DockBuilderSplitNode(right_pane, ImGuiDir_Down, 0.3f, nullptr,
                                                                    &right_pane);

            ImGui::DockBuilderDockWindow("RPM", main_top_pane);
            ImGui::DockBuilderDockWindow("Speed", main_top_pane);
            ImGui::DockBuilderDockWindow("Socketcand Packets", main_dock);
            ImGui::DockBuilderDockWindow("Tools", right_pane);
            ImGui::DockBuilderDockWindow("Search", right_pane);
            ImGui::DockBuilderDockWindow("Filter", right_pane);
            ImGui::DockBuilderDockWindow("Connection Manager", right_pane);
            ImGui::DockBuilderDockWindow("DBC Options", right_bottom_pane);
            ImGui::DockBuilderDockWindow("Frame Properties", right_bottom_pane);

            ImGui::DockBuilderFinish(docker_id);
        }
    }

    void gui::show_reset_window_pos_dlg() {
        if (state_at_or_init(m_state.open_dialogs, std::string("reset_window_pos"))) {
            ImGui::OpenPopup("ResetWindowPosDlg");
            ImGui::SetWindowPos("ResetWindowPosDlg", ImVec2{0, 0});
        }
        if (ImGui::BeginPopupModal("ResetWindowPosDlg", nullptr)) {
            ImGui::Text("Reset all window positions?");

            if (ImGui::Button("Yes")) {
//                delete_ini_file = true;
// FIXME:
                glfwSetWindowShouldClose(m_win, true);
            }

            if (ImGui::Button("No")) {
                ImGui::CloseCurrentPopup();
                m_state.open_dialogs["reset_window_pos"] = false;
            }

            ImGui::EndPopup();
        }
    }

    void gui::show_file_dialogs() {
        for (const auto &key: m_state.file_dialogs) {
            if (ImGuiFileDialog::Instance()->Display(key)) {
                if (ImGuiFileDialog::Instance()->IsOk()) { // action if OK
                    std::string filePathName = ImGuiFileDialog::Instance()->GetFilePathName();
                    std::string filePath = ImGuiFileDialog::Instance()->GetCurrentPath();

                    if (key == "ChooseDbcFileDlgKey") {
                        m_state.dbc_file = canary::dbcparser::load_dbc_file(filePathName);
                    }
                }

                // close
                ImGuiFileDialog::Instance()->Close();
            }
        }
    }

    void gui::show_dbc_options_win() {
        if (state_at_or_init(m_state.open_dialogs, std::string("dbc_options_win"), false)) {
            if (ImGui::Begin("DBC Options")) {
                ImGui::InputInt("Match first n characters, n:", &m_state.dbc_opt.first_n);
                ImGui::InputInt("Offset by: ", &m_state.dbc_opt.offset);
            }

            ImGui::End();
        }
    }

    void gui::show_packets() {
        ImGui::Begin("Socketcand Packets");
        {
            ImGui::Checkbox("Auto scroll", &m_state.packet_view_opts.auto_scroll);
            ImGui::Checkbox("Pause", &m_state.packet_view_opts.paused);

            if (ImGui::BeginTable("PacketTable", 5,
                                  ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_ScrollY)) {
                ImGui::TableSetupColumn("Index", ImGuiTableColumnFlags_WidthFixed, 50.0f);
                ImGui::TableSetupColumn("Timestamp", ImGuiTableColumnFlags_WidthStretch, 0.1f);
                ImGui::TableSetupColumn("CAN ID", ImGuiTableColumnFlags_WidthFixed, 0.0f);
                ImGui::TableSetupColumn("Len", ImGuiTableColumnFlags_WidthFixed, 50.0f);
                ImGui::TableSetupColumn("Data", ImGuiTableColumnFlags_WidthStretch);

                ImGui::TableSetupScrollFreeze(0, 1);
                ImGui::TableHeadersRow();

                ImGuiListClipper clipper;
                clipper.Begin(static_cast<int>(m_packet_provider.get_received_packets().size()));

//                std::lock_guard<std::mutex> lock(packets_mutex);

//                while (clipper.Step()) {
//                    for (int i = clipper.DisplayStart; i < clipper.DisplayEnd; i++) {
                for (size_t i = 0; i < m_packet_provider.get_received_packets().size(); ++i) {
                    std::vector<std::string> parts = split_string(m_packet_provider.get_received_packets()[i],
                                                                  std::string(" "));
                    if (parts.size() != 6) {
                        // Probably < ok > or malformed packet, ignore/*
                        continue;
                    }

                    if (m_state.packet_filter_enabled) {
                        if (std::find(excluded_ids.begin(), excluded_ids.end(), parts[2]) != excluded_ids.end()) {
                            // Excluded ID, ignore
                            continue;
                        }
                    }

//                    if (parts[4].length() / 2 != 8) {
//                        continue;
//                    }

                    ImGui::TableNextRow();
                    ImGui::TableSetColumnIndex(0);
                    ImGui::Text("%zu", i + 1);
                    ImGui::TableSetColumnIndex(1);
                    ImGui::Text("%s", parts[3].c_str());
                    ImGui::TableSetColumnIndex(2);
                    ImGui::Text("0x%s", parts[2].c_str());
                    ImGui::TableSetColumnIndex(3);
                    ImGui::Text("%zu", parts[4].length() / 2);
                    ImGui::TableSetColumnIndex(4);
//                    ImGui::Text("%s", parts[4].c_str());

                    bool is_selected = (m_state.packet_view_opts.selected_row == i);
                    if (ImGui::Selectable(parts[4].c_str(), is_selected, ImGuiSelectableFlags_SpanAllColumns)) {
                        m_state.packet_view_opts.selected_row = (is_selected ? -1 : i);
                    }

                    if (!m_state.dbc_file.messages.empty()) {
//                        if (dbc_file.messages.contains(std::stol(parts[2]))) {
//                            auto message = dbc_file.messages.at(std::stol(parts[2]));
//                            ImGui::Text(message.name.c_str());
//                        } else {
//                            ImGui::Text("CAN ID not found in DBC file");
//                        }
                        bool found(false);
                        for (const auto &[can_id, message]: m_state.dbc_file.messages) {
                            // DBC file represents IDs as a decimal, convert here to a hex string
                            std::stringstream stream;
                            stream << std::hex << can_id;
                            std::string can_id_hex(stream.str());

                            // TODO: Cache of found IDs
                            // TODO: Use dbc_options_first_n variable
                            // TODO: Skip first character for now, until offset implemented
                            auto can_id_first_n = can_id_hex.substr(1, 4);
                            auto to_find_first_n = parts[2].substr(1, 4);

                            std::transform(can_id_first_n.begin(), can_id_first_n.end(), can_id_first_n.begin(),
                                           ::toupper);
                            std::transform(to_find_first_n.begin(), to_find_first_n.end(), to_find_first_n.begin(),
                                           ::toupper);

                            if (can_id_first_n == to_find_first_n) {
                                found = true;
                                ImGui::Text("Matching CAN ID: 0x%s", can_id_hex.c_str());
                                ImGui::Text("Packet: %s", message.name.c_str());

                                if (is_selected) {
                                    m_state.packet_view_opts.selected_frame = std::make_pair(message, parts[4].c_str());
                                }

                                break;
                            }
                        }

                        if (!found) {
                            ImGui::Text("CAN ID not found in DBC file");
                        }
                    }


                    if (parts[2] == "102CA040") {
                        // Engine information
                        std::vector<bool> bits = hexStringToBitArray(parts[4]);

                        // Engine information
                        bool engineRunning = bits[14];
                        ImGui::Text("    %d", engineRunning);

                        uint16_t engine_speed = extractFromBoolVector(bits, 23);
                        ImGui::Text("    %f", 0 + 0.25 * engine_speed);
                    }

                    if (parts[2] == "10248040") {
                        // Battery voltage
                        std::vector<bool> bits = hexStringToBitArray(parts[4]);

                        uint8_t voltage = extractFromBoolVectorInt(bits, 23);
                        ImGui::Text("    %f V", 3 + 0.1 * voltage);
                    }

                    if (i == m_packet_provider.get_received_packets().size() - 1 &&
                        m_state.packet_view_opts.auto_scroll) {
                        ImGui::SetScrollHereY(1.0f);
                    }

                    // FIXME: Pause can cause listener thread to break
                }
//                }
                ImGui::EndTable();
            }
            ImGui::End();
        }
    }

    void gui::show_frame_properties() {
        ImGui::Begin("Frame Properties");
        {
            auto frame = m_state.packet_view_opts.selected_frame;

            ImGui::Text("Name: %s", frame.first.name.c_str());
            ImGui::Text("Data (hex): %s", frame.second.c_str());

            std::vector<bool> bit_array = hexStringToBitArray(frame.second);
            std::string bit_array_str;
            bit_array_str.reserve(bit_array.size());
            for (bool b: bit_array) {
                bit_array_str += (b ? '1' : '0');
            }
            ImGui::Text("Data (bin): %s", bit_array_str.c_str());

            ImGui::Text("");
            ImGui::Text("Signals:");

            for (const auto &signal: frame.first.signals) {
                if (signal.length == 8) {
                    // Int8
                    auto val = extractFromBoolVectorInt(bit_array, signal.start);
                    auto scaled_val = signal.offset + signal.scale * (float) val;
                    ImGui::Text("%s: %.2f", signal.name.c_str(), scaled_val);
                } else if (signal.length == 16) {
                    // Int16
                    auto val = extractFromBoolVector(bit_array, signal.start);

                    if (!signal.little_endian) {
                        val = swap_endian_16(val);
                    }

                    auto scaled_val = signal.offset + signal.scale * (float) val;
                    ImGui::Text("%s: %.2f", signal.name.c_str(), scaled_val);
                } else {
                    // Unknown type
                    ImGui::Text("%s: (Unknown type)", signal.name.c_str());
                }
            }

            ImGui::End();
        }
    }

    void gui::show_search() {
        ImGui::Begin("Search");
        {
            ImGui::InputInt("Range Start", &m_state.search_opts.search_start);
            ImGui::InputInt("End", &m_state.search_opts.search_end);

            if (m_state.search_opts.search_num == 1) {
                if (ImGui::Button("First search")) {
                    m_state.packet_view_opts.paused = true;

                    //std::lock_guard<std::mutex> lock(packets_mutex);
                    for (const auto &received_packet: m_packet_provider.get_received_packets()) {
                        for (auto term = m_state.search_opts.search_start;
                             term <= m_state.search_opts.search_end; term++) {
                            std::stringstream hex_str;
                            hex_str << std::hex << term;

                            std::vector<std::string> parts = split_string(received_packet, std::string(" "));

                            if (parts[4].find(hex_str.str()) != std::string::npos) {
                                std::stringstream msg;
                                msg << term << " found in packet. Data: ";
                                msg << parts[4];
                                m_state.search_opts.search_1_ids.emplace_back(parts[2], msg.str());
                            }
                        }
                    }


                    m_packet_provider.clear_packets();
                    m_state.packet_view_opts.paused = false;
                    m_state.search_opts.search_num++;
                }
            } else if (m_state.search_opts.search_num == 2) {
                if (ImGui::Button("Second search")) {
                    m_state.packet_view_opts.paused = true;

                    //std::lock_guard<std::mutex> lock(packets_mutex);
                    for (const auto &received_packet: m_packet_provider.get_received_packets()) {
                        for (auto term = m_state.search_opts.search_start;
                             term <= m_state.search_opts.search_end; term++) {
                            std::stringstream hex_str;
                            hex_str << std::hex << term;

                            std::vector<std::string> parts = split_string(received_packet, std::string(" "));

                            if (parts[4].find(hex_str.str()) != std::string::npos) {
                                std::stringstream msg;
                                msg << term << " found in packet. Data: ";
                                msg << parts[4];
                                m_state.search_opts.search_2_ids.emplace_back(parts[2], msg.str());
                            }
                        }
                    }


                    m_packet_provider.clear_packets();
                    m_state.packet_view_opts.paused = false;
                    m_state.search_opts.search_num++;
                }
            } else {
                if (ImGui::Button("Reset")) {
                    m_state.search_opts.search_1_ids.clear();
                    m_state.search_opts.search_2_ids.clear();

                    m_state.search_opts.search_num = 1;
                }
            }


            ImGui::Text("First search results:");
            if (ImGui::BeginTable("FoundPacketsTable", 2,
                                  ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg)) {
                ImGui::TableSetupColumn("CAN ID", ImGuiTableColumnFlags_WidthFixed, 50.0f);
                ImGui::TableSetupColumn("Message", ImGuiTableColumnFlags_WidthStretch);

                ImGui::TableSetupScrollFreeze(0, 1);
                ImGui::TableHeadersRow();


                for (auto &search_1_id: m_state.search_opts.search_1_ids) {
                    ImGui::TableNextRow();
                    ImGui::TableSetColumnIndex(0);
                    ImGui::Text("0x%s", get<0>(search_1_id).c_str());
                    ImGui::TableSetColumnIndex(1);
                    ImGui::Text("%s", get<1>(search_1_id).c_str());
                }
                ImGui::EndTable();

            }


            ImGui::Text("Second search results:");
            if (ImGui::BeginTable("FoundPacketsTable2", 2,
                                  ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg)) {
                ImGui::TableSetupColumn("CAN ID", ImGuiTableColumnFlags_WidthFixed, 50.0f);
                ImGui::TableSetupColumn("Message", ImGuiTableColumnFlags_WidthStretch);

                ImGui::TableSetupScrollFreeze(0, 1);
                ImGui::TableHeadersRow();

                for (auto &search_2_id: m_state.search_opts.search_2_ids) {
                    bool found(false);

                    auto it = std::find_if(m_state.search_opts.search_1_ids.begin(),
                                           m_state.search_opts.search_1_ids.end(),
                                           [&search_2_id](const std::tuple<std::string, std::string> &e) {
                                               return std::get<0>(e) == get<0>(search_2_id);
                                           });
                    if (it != m_state.search_opts.search_1_ids.end()) {
                        found = true;
                    }

                    if (!found) continue;

                    ImGui::TableNextRow();
                    ImGui::TableSetColumnIndex(0);
                    ImGui::Text("0x%s", get<0>(search_2_id).c_str());
                    ImGui::TableSetColumnIndex(1);
                    ImGui::Text("%s", get<1>(search_2_id).c_str());
                }

                ImGui::EndTable();

            }
            ImGui::End();
        }
    }

    void gui::show_filter() {
        ImGui::Begin("Filter");
        {
            ImGui::Checkbox("Enable Packet Filter", &m_state.packet_filter_enabled);

            if (ImGui::BeginTable("FilteredPacketsTable", 1, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg)) {
                ImGui::TableSetupColumn("CAN ID", ImGuiTableColumnFlags_WidthStretch);

                ImGui::TableHeadersRow();

                for (auto &id: excluded_ids) {
                    ImGui::TableNextRow();
                    ImGui::TableSetColumnIndex(0);
                    ImGui::Text("0x%s", id.c_str());
                }

                ImGui::EndTable();
            }

            ImGui::End();
        }
    }

    void gui::show_conn_mgr() {

        if (state_at_or_init(m_state.open_dialogs, std::string("connection_mgr"))) {
            ImGui::Begin("Connection Manager");
            {
                if (ImGui::Button("Add")) {
                    m_state.open_dialogs["conn_mgr_edit"] = true;
                    m_state.current_connection = nullptr;
                }
                ImGui::SameLine();
                auto cursor_pos = ImGui::GetCursorPos();
                ImGui::NewLine();

                if (ImGui::BeginTable("ConnectionsTable", 2, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg)) {
                    ImGui::TableSetupColumn("Name", ImGuiTableColumnFlags_WidthStretch);
                    ImGui::TableSetupColumn("Type", ImGuiTableColumnFlags_WidthStretch);

                    ImGui::TableHeadersRow();

                    int i = 0;
                    for (const auto &conn: canary::config::config_loader::app_config.connections) {
                        ImGui::TableNextRow();
                        ImGui::TableNextColumn();
                        bool is_selected = (m_state.connection_mgr_selected_row == i);
                        if (ImGui::Selectable(conn.name.c_str(), is_selected, ImGuiSelectableFlags_SpanAllColumns)) {
                            m_state.connection_mgr_selected_row = (is_selected ? -1 : i);
                        }
                        ImGui::TableNextColumn();
                        ImGui::Text("%s", conn.type.c_str());

                        i++;
                    }

                    ImGui::EndTable();
                }

                if (m_state.connection_mgr_selected_row > -1) {
                    const canary::config::connection &sel_item = canary::config::config_loader::app_config.connections[m_state.connection_mgr_selected_row];
                    ImGui::Text("Params:");

                    for (const auto &[key, value]: sel_item.params) {
                        ImGui::Text("%s: %s", key.c_str(), value.dump().c_str());
                    }

                    ImGui::SetCursorPos(cursor_pos);

                    if (ImGui::Button("Edit")) {
                        m_state.open_dialogs["conn_mgr_edit"] = true;
                        m_state.current_connection = std::make_unique<const canary::config::connection>(sel_item);
                    }
                    ImGui::SameLine();

                    if (ImGui::Button("Remove")) {
                        m_state.open_dialogs["conn_mgr_remove"] = true;
                    }
                }

                ImGui::SetCursorPosY(ImGui::GetWindowSize().y - ImGui::GetFrameHeightWithSpacing() -
                                     ImGui::GetStyle().WindowPadding.y);

                ImGui::Button("Connect");
                ImGui::SameLine();
                ImGui::Button("Disconnect");

                ImGui::End();
            }
        }

    }

    void gui::show_conn_mgr_edit_dlg() {
        if (state_at_or_init(m_state.open_dialogs, std::string("conn_mgr_edit"))) {
            ImGui::OpenPopup("ConnMgrEditDlg");
        }
        if (ImGui::BeginPopupModal("ConnMgrEditDlg", nullptr)) {
            if (m_state.current_connection != nullptr) {
                ImGui::Text("%s", m_state.current_connection->name.c_str());

                for (const auto &[key, value]: m_state.current_connection->params) {
                    ImGui::Text("%s: %s", key.c_str(), value.dump().c_str());
                }
            }

            if (ImGui::Button("No")) {
                ImGui::CloseCurrentPopup();
                m_state.open_dialogs["conn_mgr_edit"] = false;
            }

            ImGui::EndPopup();
        }
    }

    void gui::show_gauges() {
        ImGui::Begin("RPM");
        {
            ImVec2 centre = ImGui::GetCursorScreenPos();
            centre.x += 100;
            centre.y += 100;

            float gauge_radius = 80.0f;
            float value = m_state.rpm;
            float min_value = 0.0f;
            float max_value = 7000.0f;

            draw_gauge("RPM", value, min_value, max_value, centre, gauge_radius);

            ImGui::Dummy(ImVec2(gauge_radius * 2, gauge_radius * 2));

            ImGui::End();
        }

        ImGui::Begin("Speed");
        {
            ImVec2 centre = ImGui::GetCursorScreenPos();
            centre.x += 100;
            centre.y += 100;

            float gauge_radius = 80.0f;
            float value = m_state.speed;
            float min_value = 0.0f;
            float max_value = 255.0f;

            draw_gauge("km/h", value, min_value, max_value, centre, gauge_radius);

            ImGui::Dummy(ImVec2(gauge_radius * 2, gauge_radius * 2));

            ImGui::End();
        }
    }

    void gui::show_tools() {
        ImGui::Begin("Tools");
        {
            ImGui::Button("Enable Driver Heated Seats");
        }
        ImGui::End();
    }
}
