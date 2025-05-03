#include <boulderdash/boulderdash.h>

#include <iostream>
#include <limits>
#include <string>
#include <unordered_map>

using namespace boulderdash;

namespace {
void test_play() {
    std::string board_str;
    std::cout << "Enter board str: ";
    std::cin >> board_str;
    BoulderDashGameState state(board_str);

    std::cout << state;
    std::cout << state.get_hash() << std::endl;

    int action = -1;
    while (!state.is_terminal()) {
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        action = std::cin.get();
        switch (action) {
            case 'w':
                state.apply_action(Action::kUp);
                break;
            case 'd':
                state.apply_action(Action::kRight);
                break;
            case 's':
                state.apply_action(Action::kDown);
                break;
            case 'a':
                state.apply_action(Action::kLeft);
                break;
            default:
                return;
        }
        std::cout << state;
        std::cout << state.get_hash() << std::endl;
        std::cout << state.agent_alive() << " " << state.agent_in_exit() << std::endl;
        std::cout << std::endl;
    }
}
}    // namespace

int main() {
    test_play();
}
