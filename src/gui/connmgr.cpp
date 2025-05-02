// Copyright (C) 2024 Ryan Bester

#include "connmgr.hpp"
#include "imgui.h"
#include "gui.hpp"

namespace canary::gui {
    connmgr_state connmgr::m_state;

    const char *connmgr::can_types[] = {
            "socketcand",
            "test",
            "direct",
            "serial"
    };

    void connmgr::show_conn_mgr(gui &gui) {
        if (state_at_or_init(gui.m_state.open_dialogs, std::string("connection_mgr"))) {
            ImGui::Begin("Connection Manager");
            {
                if (ImGui::Button("Add")) {
                    gui.m_state.open_dialogs["conn_mgr_edit"] = true;
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
                        ImGui::Text("%s", conn.can_type.c_str());

                        i++;
                    }

                    ImGui::EndTable();
                }

                if (m_state.connection_mgr_selected_row > -1) {
                    const canary::config::connection &sel_item = APP_CONFIG.connections[m_state.connection_mgr_selected_row];
                    ImGui::Text("Params:");

                    for (const auto &[key, value]: sel_item.can_params) {
                        ImGui::Text("%s: %s", key.c_str(), value.dump().c_str());
                    }

                    ImGui::Spacing();

                    ImGui::Text("CANaryd Enabled: %s", sel_item.canaryd_enabled ? "true" : "false");
                    ImGui::Text("CANaryd Host: %s", sel_item.canaryd_host.c_str());
                    ImGui::Text("CANaryd Port: %d", sel_item.canaryd_port);

                    ImGui::SetCursorPos(cursor_pos);

                    if (ImGui::Button("Edit")) {
                        gui.m_state.open_dialogs["conn_mgr_edit"] = true;
                        m_state.current_connection = std::make_unique<const canary::config::connection>(sel_item);
                    }
                    ImGui::SameLine();

                    if (ImGui::Button("Remove")) {
                        gui.m_state.open_dialogs["conn_mgr_remove"] = true;
                    }
                }

                ImGui::SetCursorPosY(ImGui::GetWindowSize().y - ImGui::GetFrameHeightWithSpacing() -
                                     ImGui::GetStyle().WindowPadding.y);

                ImGui::Button("Connect");
                ImGui::SameLine();
                ImGui::Button("Disconnect");

                if (ImGui::BeginPopup("Options")) {
                    ImGui::Checkbox("Non-blocking", &APP_CONFIG.conn_opts.non_blocking);
                    ImGui::InputInt("Timeout", &APP_CONFIG.conn_opts.timeout);
                    ImGui::InputInt("Cooldown", &APP_CONFIG.conn_opts.cooldown);
                    ImGui::EndPopup();
                }

                ImGui::SameLine();
                if (ImGui::Button("Options")) {
                    ImGui::OpenPopup("Options");
                }

                ImGui::End();
            }
        }
    }

    void connmgr::show_conn_mgr_edit_dlg(gui &gui) {
        if (state_at_or_init(gui.m_state.open_dialogs, std::string("conn_mgr_edit"))) {
            ImGui::OpenPopup("ConnMgrEditDlg");
        }

        if (ImGui::BeginPopupModal("ConnMgrEditDlg", nullptr)) {
            if (m_state.current_connection != nullptr) {
                // Edit
                ImGui::Text("%s", m_state.current_connection->name.c_str());

                for (const auto &[key, value]: m_state.current_connection->can_params) {
                    ImGui::Text("%s: %s", key.c_str(), value.dump().c_str());
                }
            } else {
                // Create
                ImGui::Text("Add New");

                ImGui::InputText("Name", m_state.name, IM_ARRAYSIZE(m_state.name));

                ImGui::Combo("CAN Type", &m_state.can_type, can_types, sizeof(can_types ) / sizeof(can_types[0]));
                // TODO: Params

                ImGui::Checkbox("CANaryd Enabled", &m_state.canaryd_enabled);
                ImGui::InputText("CANaryd Host", m_state.canaryd_host, IM_ARRAYSIZE(m_state.canaryd_host));
                ImGui::InputInt("CANaryd Port", &m_state.canaryd_port);
            }

            if (ImGui::Button("Add")) {
                std::stringstream cmd_text;
                cmd_text << "conns add";
                cmd_text << " -name " << m_state.name;
                cmd_text << " -can_type " << can_types[m_state.can_type];
                cmd_text << " -can_params " << "host=192.168.0.123,port=4568";
                cmd_text << " -canaryd_enabled " << (m_state.canaryd_enabled ? "true" : "false");
                cmd_text << " -canaryd_host " << m_state.canaryd_host;
                cmd_text << " -canaryd_port " << m_state.canaryd_port;

                // TODO: Better error handling
                if (0 == gui.m_command_dispatcher.execute_command(cmd_text.str())) {
                    ImGui::CloseCurrentPopup();
                    gui.m_state.open_dialogs["conn_mgr_edit"] = false;
                } else {
                    // Error
                }
            }

            ImGui::SameLine();
            if (ImGui::Button("Cancel")) {
                ImGui::CloseCurrentPopup();
                gui.m_state.open_dialogs["conn_mgr_edit"] = false;
            }

            ImGui::EndPopup();
        }
    }
}
