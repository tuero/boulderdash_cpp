#include <boulderdash/boulderdash.h>

#include <chrono>
#include <cstddef>
#include <cstdint>
#include <iostream>
#include <string>
#include <vector>

using namespace boulderdash;

using std::chrono::duration;
using std::chrono::high_resolution_clock;
using std::chrono::milliseconds;

namespace {
constexpr std::size_t NUM_STEPS = 10000000;
constexpr std::size_t MILLISECONDS_PER_SECOND = 1000;

void test_speed() {
    const std::string board_str =
        "14|14|1|18|18|18|18|18|18|18|18|18|18|18|18|18|18|18|07|01|01|18|01|01|01|01|18|02|02|05|18|18|02|01|01|18|"
        "02|02|02|02|18|02|32|01|18|18|01|01|02|36|02|02|02|01|18|01|01|02|18|18|18|18|18|18|01|01|01|01|18|34|18|18|"
        "18|18|01|02|02|01|01|02|02|02|01|02|02|02|18|18|02|02|02|35|02|01|02|02|02|02|01|01|18|18|01|01|02|02|01|02|"
        "02|01|02|02|01|01|18|18|02|02|02|01|02|01|01|02|01|01|02|02|18|18|18|18|18|18|00|02|01|01|18|18|18|18|18|18|"
        "01|01|29|18|02|01|02|02|18|02|01|02|18|18|02|01|02|18|02|01|02|02|18|02|02|01|18|18|01|01|01|31|01|01|02|01|"
        "28|01|38|02|18|18|18|18|18|18|18|18|18|18|18|18|18|18|18";
    BoulderDashGameState state(board_str);

    std::cout << "starting ..." << std::endl;

    const auto t1 = high_resolution_clock::now();
    for (std::size_t i = 0; i < NUM_STEPS; ++i) {
        state.apply_action(
            ALL_ACTIONS[i % BoulderDashGameState::action_space_size()]);    // NOLINT(*-bounds-constant-array-index)
        const std::vector<float> obs = state.get_observation();
        (void)obs;
        const uint64_t hash = state.get_hash();
        (void)hash;
    }
    const auto t2 = high_resolution_clock::now();
    const duration<double, std::milli> ms_double = t2 - t1;

    std::cout << "Total time for " << NUM_STEPS << " steps: " << ms_double.count() / MILLISECONDS_PER_SECOND
              << std::endl;
    std::cout << "Time per step :  " << ms_double.count() / MILLISECONDS_PER_SECOND / NUM_STEPS << std::endl;
}
}    // namespace

int main() {
    test_speed();
}
