// Copyright (C) 2024 Ryan Bester

#include "cmdline.hpp"
#include "imgui.h"
#include "gui.hpp"

namespace canary::gui {
    cmdline_state cmdline::m_state;

    void cmdline::show_command_line(gui &gui) {
        static const ImVec4 COLOR_YELLOW(1.0f, 1.0f, 0.0f, 1.0f);
        static const ImVec4 COLOR_RED(1.0f, 0.0f, 0.0f, 1.0f);
        static const ImVec4 COLOR_GRAY(0.6f, 0.6f, 0.6f, 1.0f);

        ImGui::Begin("Command Line");
        {
            if (ImGui::BeginChild("CommandLineScrollingRegion", ImVec2(ImGui::GetWindowWidth(),
                                                                       ImGui::GetWindowHeight() - 60))) {
                ImGui::PushFont(gui.font_monospace);
                for (const auto &line: gui.m_command_dispatcher.get_command_line().get_lines()) {
                    if ((line.starts_with(">") || line.starts_with("->")) && m_state.colored_text) {
                        ImGui::TextColored(COLOR_GRAY, "%s", line.c_str());
                    } else if (line.starts_with("[WARN]") && m_state.colored_text) {
                        ImGui::TextColored(COLOR_YELLOW, "%s", line.c_str());
                    } else if (line.starts_with("[ERROR]") && m_state.colored_text) {
                        ImGui::TextColored(COLOR_RED, "%s", line.c_str());
                    } else {
                        ImGui::TextUnformatted(line.c_str());
                    }
                }
                ImGui::PopFont();

                if (m_state.auto_scroll) {
                    ImGui::SetScrollHereY(1.0f);
                }

                if (ImGui::GetScrollY() < ImGui::GetScrollMaxY()) {
                    m_state.auto_scroll = false;
                }
            }
            ImGui::EndChild();

            if (ImGui::BeginPopup("Options")) {
                ImGui::Checkbox("Auto-scroll", &m_state.auto_scroll);
                ImGui::Checkbox("Colored Text", &m_state.colored_text);
                ImGui::EndPopup();
            }

            if (ImGui::Button("Options")) {
                ImGui::OpenPopup("Options");
            }

            auto padding = ImGui::GetStyle().WindowPadding;
            ImGui::PushItemWidth(-padding.x);

            ImGui::SameLine();

            auto input_text_flags = ImGuiInputTextFlags_EnterReturnsTrue | ImGuiInputTextFlags_EscapeClearsAll |
                                    ImGuiInputTextFlags_CallbackCompletion | ImGuiInputTextFlags_CallbackHistory;
            if (ImGui::InputText("##Command", m_state.input, IM_ARRAYSIZE(m_state.input),
                                 input_text_flags,
                                 [](ImGuiInputTextCallbackData *data) -> int {
                                     auto state = (cmdline_state *) data->UserData;

                                     switch (data->EventFlag) {

                                         case ImGuiInputTextFlags_CallbackCompletion:
                                             std::cout << "Callback completion" << std::endl;
                                             break;
                                         case ImGuiInputTextFlags_CallbackHistory:
                                             int &history_pos = state->history_pos;
                                             const int prev_history_pos = history_pos;

                                             auto &history = state->history;

                                             if (data->EventKey == ImGuiKey_UpArrow) {
                                                 if (history_pos == -1) {
                                                     // New line; set to end of history
                                                     history_pos = history.size() - 1;
                                                 } else if (history_pos > 0) {
                                                     // Keep iterating back through history
                                                     history_pos--;
                                                 }
                                             } else if (data->EventKey == ImGuiKey_DownArrow) {
                                                 if (history_pos != -1) {
                                                     if (++history_pos >=
                                                         history.size()) {
                                                         history_pos = -1;
                                                     }
                                                 }
                                             }

                                             if (prev_history_pos != history_pos) {
                                                 const char *history_str = (history_pos >= 0)
                                                                           ? history[history_pos]
                                                                           : "";
                                                 data->DeleteChars(0, data->BufTextLen);
                                                 data->InsertChars(0, history_str);
                                             }

                                             break;
                                     }
                                     return 0;
                                 }, &m_state)) {

                m_state.history.push_back(strdup(m_state.input));
                gui.m_command_dispatcher.execute_command(m_state.input);

                memset(m_state.input, 0, 256);
                m_state.input_focus = true;
                m_state.auto_scroll = true;
            }

            if (ImGui::IsKeyPressed(ImGuiKey_F2)) {
                m_state.input_focus = true;
            }

            if (ImGui::IsMouseClicked(0)) {
                m_state.input_focus = false;
            }

            if (m_state.input_focus) {
                ImGui::SetKeyboardFocusHere(-1);
            }

            ImGui::End();
        }
    }
}
