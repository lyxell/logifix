#pragma once

#include <cstdlib>
#include <string>
#include <vector>
#include <chrono>

namespace timer {

extern std::vector<std::chrono::time_point<std::chrono::high_resolution_clock>> start_times;
extern std::vector<std::string> event_data;

extern std::vector<std::pair<size_t, std::string>> events;

auto create(const std::string& name) -> std::size_t;

auto stop(std::size_t id) -> void;

}
