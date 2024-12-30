// Copyright (C) 2024 Ryan Bester

#include <iostream>
#include <memory>
#include <vector>
#include <mutex>
#include <atomic>
#include <thread>
#include <cmath>

#include "main.hpp"
#include "config.hpp"
#include "dbc.hpp"
#include "gui/gui.hpp"
#include "can/packetprovider.hpp"
#include "cmd/commanddispatcher.hpp"
#include "cmd/helpcmd.hpp"

#include <nlohmann/json.hpp>

#include "socket.hpp"


#if defined(WIN32)

#include <io.h>

#else

#include <unistd.h>
#include <arpa/inet.h>

#endif

#define SOCKETCAND_PORT 29536
#define SOCKETCAND_IP "192.168.0.31"
#define SOCKETCAND_INTERFACE "can0"

std::vector<std::string> received_packets;
std::mutex packets_mutex;
std::atomic<bool> is_running(false);

std::mutex exit_status_mutex;
std::condition_variable exit_status;


int speed = 0;

bool paused(false);


void draw_gauge(const char *label, float value, float min_value, float max_value, ImVec2 centre, float radius) {
    ImDrawList *draw_list = ImGui::GetWindowDrawList();
    const float start_angle = 3.13159f * 0.75f;
    const float end_angle = 3.14159f * 2.25f;

    float t = (value - min_value) / (max_value - min_value);
    float angle = start_angle + t * (end_angle - start_angle);

    draw_list->AddCircle(centre, radius, IM_COL32(200, 200, 200, 255), 64, 3.0f);

    draw_list->PathArcTo(centre, radius - 10, start_angle, angle, 64);
    draw_list->PathStroke(IM_COL32(100, 200, 100, 255), false, 6.0f);

    ImVec2 needle_end(
            centre.x + cosf(angle) * (radius - 20),
            centre.y + sinf(angle) * (radius - 20)
    );
    draw_list->AddLine(centre, needle_end, IM_COL32(255, 0, 0, 255), 3.0f);

    char text[32];
    snprintf(text, sizeof(text), "%.1f", value);
    ImVec2 text_size = ImGui::CalcTextSize(text);
    draw_list->AddText(ImVec2(centre.x - text_size.x * 0.5f, centre.y - radius * 0.5f), IM_COL32(255, 255, 255, 255),
                       text);

    ImVec2 label_size = ImGui::CalcTextSize(label);
    draw_list->AddText(ImVec2(centre.x - label_size.x * 0.5f, centre.y + radius + 10), IM_COL32(255, 255, 255, 255),
                       label);
}

void error(const std::string &msg) {
    perror(msg.c_str());
    is_running = false;
}

std::vector<std::string> split_string(std::string s, const std::string &delimiter) {
    std::vector<std::string> res;
    int pos = 0;
    std::string part;
    while ((pos = s.find(delimiter)) != std::string::npos) {
        part = s.substr(0, pos);
        res.push_back(part);
        s.erase(0, pos + delimiter.length());
    }
    res.push_back(s);
    return res;
}

canary::socketcand socketcand(SOCKETCAND_IP, SOCKETCAND_PORT, SOCKETCAND_INTERFACE);

void listen_for_packets() {
    char buffer[1024] = {0};

    socketcand.set_error_handler([](const std::string &msg) {
        std::cout << "Error: " << msg << std::endl;
    });

    int connect_res = socketcand.connect();
    if (connect_res != 0) {
        std::cout << "Error connecting to socketcand" << std::endl;
        return;
    }

    std::cout << "Connected to socketcand" << std::endl;

    is_running = true;
    int i = 0;
    bool flag = false;
    while (is_running && !paused) {
        i++;
        if (i % 200 == 0) {
            if (flag) {
                const char *pid_rpm_msg = "< send 7df 8 02 01 0c 00 00 00 00 00 >\n";
                socketcand.send_when_ready(pid_rpm_msg, strlen(pid_rpm_msg));
            } else {
                const char *pid_speed_msg = "< send 7df 8 02 01 0d 00 00 00 00 00 >\n";
                socketcand.send_when_ready(pid_speed_msg, strlen(pid_speed_msg));
            }
            flag = !flag;

        }

        bool read, write = false;
        int activity = socketcand.select(false, read, write);
        if (activity == 0) {
            // No activity
            continue;
        }

        if (!read) {
            continue;
        }

        memset(buffer, 0, sizeof(buffer));
        int n = socketcand.recv(buffer, sizeof(buffer) - 1);
        if (n < 0) {
            if (is_running) error("Error reading from socket");
            break;
        } else if (n == 0) {
            std::cerr << "Connection closed by socketcand" << std::endl;
            break;
        } else {
            // Split data with multiple packets
            std::lock_guard<std::mutex> lock(packets_mutex);
            auto partial_buffer = std::string(buffer);

            size_t start = 0;
            size_t end;
            while ((end = partial_buffer.find('>', start)) != std::string::npos) {
                std::string packet = partial_buffer.substr(start, end - start + 1);
                received_packets.emplace_back(packet);
                start = end + 1;

//                if (received_packets.size() > 50) {
//                    received_packets.erase(received_packets.begin());
//                }

                std::vector<std::string> parts = split_string(packet, std::string(" "));
                if (parts.size() != 6) {
                    // Probably < ok > or malformed packet, ignore
                    continue;
                }

                if (parts[2] == "7E8") {
                    if (parts[4].substr(4, 2) == "0C") {
                        // RPM response
                        auto input = parts[4].substr(6, 4);
                        // std::cout << "Got RPM response: " << parts[4].substr(6, 4) << std::endl;

                        std::string firstPart = input.substr(0, 2); // "0B"
                        std::string secondPart = input.substr(2, 2); // "AB"

                        int firstValue = std::stoi(firstPart, nullptr, 16);
                        int secondValue = std::stoi(secondPart, nullptr, 16);

                        //rpm = ((256 * firstValue) + secondValue) / 4;
                    }

                    if (parts[4].substr(4, 2) == "0D") {
                        // Speed response
                        auto input = parts[4].substr(6, 2);

                        std::string firstPart = input.substr(0, 2); // "0B"

                        int firstValue = std::stoi(firstPart, nullptr, 16);

                        speed = firstValue;
                    }
                }


            }
        }
    }

    socketcand.close();

    exit_status.notify_one();
}

std::vector<bool> hexStringToBitArray(const std::string &hex) {
    std::vector<bool> bitArray;

    for (char hexChar: hex) {
        // Convert hex character to its integer equivalent
        int value = (hexChar >= '0' && hexChar <= '9') ? (hexChar - '0') :
                    (hexChar >= 'A' && hexChar <= 'F') ? (hexChar - 'A' + 10) :
                    (hexChar >= 'a' && hexChar <= 'f') ? (hexChar - 'a' + 10) : -1;

        if (value == -1) {
            throw std::invalid_argument("Invalid hexadecimal character");
        }

        // Convert the integer to binary and append to the bit array
        for (int i = 3; i >= 0; --i) {
            bitArray.push_back((value >> i) & 1);
        }
    }

    return bitArray;
}

uint16_t swap_endian_16(uint16_t value) {
    return (value >> 8) | (value << 8);
}

uint8_t extractFromBoolVectorInt(const std::vector<bool> &bitVector, size_t startIndex) {
    if (startIndex + 8 > bitVector.size()) {
        throw std::out_of_range("Not enough bits to extract a 1-byte integer");
    }

    uint16_t result = 0;

    for (size_t i = 0; i < 8; ++i) {
        if (bitVector[startIndex + i]) {
            result |= (1 << (7 - i)); // Set the corresponding bit in the 16-bit result
        }
    }

    return result;
}

uint16_t extractFromBoolVector(const std::vector<bool> &bitVector, size_t startIndex) {
    if (startIndex + 16 > bitVector.size()) {
        throw std::out_of_range("Not enough bits to extract a 2-byte integer");
    }

    uint16_t result = 0;

    for (size_t i = 0; i < 16; ++i) {
        if (bitVector[startIndex + i]) {
            result |= (1 << (15 - i)); // Set the corresponding bit in the 16-bit result
        }
    }

    return result;
}

void register_commands(canary::command::command_dispatcher &cmd_dispatcher) {
    auto help = std::make_shared<canary::command::help_cmd>(cmd_dispatcher);
    cmd_dispatcher.register_command(help);
}

int main(int argc, char **argv) {
    canary::config::config_loader::load_config();

    // Options
    bool no_gui{false};
    std::vector<std::string> commands{};
    bool show_help{false};

    if (argc > 1) {
        // Arguments specified
        for (int i = 1; i < argc; i++) {
            std::string arg = argv[i];
            std::string arg_name, arg_val;

            auto sep_pos = std::find(arg.begin(), arg.end(), '=');
            if (sep_pos != arg.end()) {
                // key=value argument
                arg_name = std::string(arg.begin(), sep_pos);
                arg_val = std::string(sep_pos + 1, arg.end());
            } else {
                // flag argument
                arg_name = arg;
                arg_val = "true";
            }

            // Strip - or -- from beginning
            auto name_start = arg_name.find_first_not_of('-');
            if (name_start != std::string::npos) {
                arg_name.erase(0, name_start);
            }

            if (arg_name == "help") {
                std::istringstream(arg_val) >> std::boolalpha >> show_help;
            } else if (arg_name == "nogui") {
                std::istringstream(arg_val) >> std::boolalpha >> no_gui;
            } else if (arg_name == "cmds") {
                commands = split_string(arg_val, ";");
            } else {
                std::cout << "Unrecognised option: " << arg_name << std::endl;
            }
        }
    }

    if (show_help) {
        std::cout << "CANary help" << std::endl << std::endl;
        std::cout << "--help\tShow this help message" << std::endl;
        std::cout << "--nogui\tDon't render the GUI and don't start the connection loop" << std::endl;
        std::cout << "--cmds\tSemicolon-separated list of commands to execute" << std::endl;
        return 0;
    }

    std::thread listener_thread(listen_for_packets);

    if (!glfwInit())
        return 1;

        glfwWindowHint(GLFW_SAMPLES, 4);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
        glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

        win = glfwCreateWindow(1200, 1000, "CANary", NULL, NULL);
        glfwMakeContextCurrent(win);

        glfwMaximizeWindow(win);

        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGuiIO &io = ImGui::GetIO();
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;
        io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

        ImGui_ImplGlfw_InitForOpenGL(win, true);
        ImGui_ImplOpenGL3_Init("#version 330");

        if (!gladLoadGLLoader((GLADloadproc) glfwGetProcAddress)) {
            std::cout << "Failed to initialize GLAD" << std::endl;
            return -1;
        }
    }

    bool delete_ini_file(false);

    std::unique_ptr<const canary::config::connection> current_connection;

    canary::can::packetprovider provider;
    canary::command::command_dispatcher cmd_dispatcher;

    register_commands(cmd_dispatcher);

    int res = 0;
    for (const auto &cmd: commands) {
        res = cmd_dispatcher.execute_command(cmd);
        if (res != 0) {
            break;
        }
    }

    for (const auto &line: cmd_dispatcher.get_command_line().get_lines()) {
        std::cout << line << std::endl;
    }

    std::cout << "Commands executed with code: " << res << std::endl;

    std::shared_ptr<canary::gui::gui> gui = nullptr;
    if (!no_gui) {
        gui = std::make_shared<canary::gui::gui>(win, provider, cmd_dispatcher);
        gui->load_options();
        auto scale = canary::gui::gui::get_monitor_scale();
        ImGuiIO &io = ImGui::GetIO();
        gui->set_scale(io, 13.0f, scale);

        provider.add_packet("Test");

        while (!glfwWindowShouldClose(win)) {
            ImGui_ImplOpenGL3_NewFrame();
            ImGui_ImplGlfw_NewFrame();
            ImGui::NewFrame();

            gui->render_frame();

            glClearColor(0, 0, 0, 0);
            glClear(GL_COLOR_BUFFER_BIT);

            ImGui::Render();
            ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

            glfwSwapBuffers(win);
            glfwPollEvents();
        }
    }

    // Force close if still in connection phase
    if (!socketcand.m_connected) {
        socketcand.close();
    }

    if (is_running) {
        is_running = false;
        std::cout << "Waiting for listener thread to quit..." << std::endl;
        {
            std::unique_lock lk(exit_status_mutex);
            exit_status.wait(lk);
        }
        std::cout << "Listener thread terminated" << std::endl;
    }

    listener_thread.join();

    if (!no_gui) {
        ImGui_ImplOpenGL3_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyContext();

        glfwTerminate();

        // Remove ImGui ini file if Reset Window Positions options is selected
        if (delete_ini_file) std::remove("imgui.ini");

        gui->save_options();
    }

    canary::config::config_loader::save_config();

    return 0;
}
