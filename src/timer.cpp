#include "timer.h"
#include <mutex>

namespace timer {

using std::chrono::duration_cast;
using std::chrono::high_resolution_clock;
using std::chrono::microseconds;

std::vector<std::chrono::time_point<std::chrono::high_resolution_clock>> start_times;
std::vector<std::string> event_data;
std::vector<std::pair<size_t, std::string>> events;

namespace {
std::mutex timer_mutex;
} // namespace

auto create(const std::string& name) -> size_t {
    auto lock = std::unique_lock{timer_mutex};
    auto id = start_times.size();
    start_times.emplace_back(high_resolution_clock::now());
    event_data.emplace_back(name);
    return id;
}

auto stop(size_t id) -> void {
    auto lock = std::unique_lock{timer_mutex};
    auto end = high_resolution_clock::now();
    auto diff = duration_cast<microseconds>(end - start_times[id]).count();
    events.emplace_back(diff, event_data[id]);
}

} // namespace timer
