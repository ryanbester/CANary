// Copyright (C) 2024 Ryan Bester

#ifndef __CANARY_GUI_CMDLINE__
#define __CANARY_GUI_CMDLINE__

#include <vector>

namespace canary::gui {

    class gui;

    struct cmdline_state {
        char input[256] = {0};
        int history_pos = -1;
        std::vector<char *> history;
        bool input_focus = false;
        bool auto_scroll = true;
        bool colored_text = true;
    };

    class cmdline {
    public:
        static cmdline_state m_state;

        static void show_command_line(gui &gui);
    };

}

#endif
