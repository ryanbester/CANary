// Copyright (C) 2024 Ryan Bester

#include "gsm.hpp"
#include "imgui.h"

namespace canary::gui {
    void gsm::show_gsm_window(gui &gui) {
        ImGui::Begin("GSM");
        {
            ImGui::Button("AT+CPIN?");
            if (ImGui::IsItemHovered()) {
                ImGui::SetTooltip("Test SIM pin");
            }

            ImGui::End();
        }
    }
}
