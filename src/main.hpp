// Copyright (C) 2024 Ryan Bester

#ifndef __CANARY_MAIN__

#define __CANARY_MAIN__

#include <glad/glad.h>

#include <GLFW/glfw3.h>

#include <imgui.h>
#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_opengl3.h>
#include <imgui_internal.h>

#include <unistd.h>
#include <arpa/inet.h>

#include <ImGuiFileDialog.h>

void draw_gauge(const char *label, float value, float min_value, float max_value, ImVec2 centre, float radius);

void error(const std::string &msg);

std::vector<std::string> split_string(std::string s, const std::string &delimiter);

void listen_for_packets();

std::vector<bool> hexStringToBitArray(const std::string &hex);

uint8_t extractFromBoolVectorInt(const std::vector<bool> &bitVector, size_t startIndex);

uint16_t extractFromBoolVector(const std::vector<bool> &bitVector, size_t startIndex);

uint16_t swap_endian_16(uint16_t value);

#endif
