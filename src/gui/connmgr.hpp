// Copyright (C) 2024 Ryan Bester

#ifndef __CANARY_GUI_CONNMGR__
#define __CANARY_GUI_CONNMGR__

#include <vector>
#include <memory>

#include "../config.hpp"

namespace canary::gui {

    class gui;

    struct connmgr_state {
        std::unique_ptr<const canary::config::connection> current_connection;
        int connection_mgr_selected_row = -1;
    };

    class connmgr {
    public:
        static connmgr_state m_state;

        static void show_conn_mgr(gui &gui);

        static void show_conn_mgr_edit_dlg(gui &gui);
    };

}

#endif
