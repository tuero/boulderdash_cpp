#include "boulderdash_base.h"

#include <algorithm>
#include <array>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <format>
#include <iostream>
#include <ranges>
#include <sstream>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

#include "definitions.h"
#include "util.h"

namespace boulderdash {

namespace {

[[noreturn]] inline void unreachable() {
    // Uses compiler specific extensions if possible.
    // Even if no extension is used, undefined behavior is still raised by
    // an empty function body and the noreturn attribute.
#if defined(_MSC_VER) && !defined(__clang__)    // MSVC
    __assume(false);
#else    // GCC, Clang
    __builtin_unreachable();
#endif
}

constexpr uint64_t SPLIT64_S1 = 30;
constexpr uint64_t SPLIT64_S2 = 27;
constexpr uint64_t SPLIT64_S3 = 31;
constexpr uint64_t SPLIT64_C1 = 0x9E3779B97f4A7C15;
constexpr uint64_t SPLIT64_C2 = 0xBF58476D1CE4E5B9;
constexpr uint64_t SPLIT64_C3 = 0x94D049BB133111EB;

// https://en.wikipedia.org/wiki/Xorshift
// Portable RNG Seed
// NOLINTBEGIN
auto splitmix64(uint64_t seed) noexcept -> uint64_t {
    uint64_t result = seed + SPLIT64_C1;
    result = (result ^ (result >> SPLIT64_S1)) * SPLIT64_C2;
    result = (result ^ (result >> SPLIT64_S2)) * SPLIT64_C3;
    return result ^ (result >> SPLIT64_S3);
}
// NOLINTEND

// Portable RNG
// NOLINTBEGIN
auto xorshift64(uint64_t &s) noexcept -> uint64_t {
    uint64_t x = s;
    x ^= x << 13;
    x ^= x >> 7;
    x ^= x << 17;
    s = x;
    return x;
}
// NOLINTEND

auto to_local_hash(int flat_size, HiddenCellType el, int offset) noexcept -> uint64_t {
    uint64_t seed = (flat_size * static_cast<int>(to_underlying(el))) + offset;
    uint64_t result = seed + SPLIT64_C1;
    result = (result ^ (result >> SPLIT64_S1)) * SPLIT64_C2;
    result = (result ^ (result >> SPLIT64_S2)) * SPLIT64_C3;
    return result ^ (result >> SPLIT64_S3);
}
}    // namespace

void BoulderDashGameState::parse_board_str(const std::string &board_str) {
    std::stringstream board_ss(board_str);
    std::string segment;
    std::vector<std::string> seglist;
    // string split on |
    while (std::getline(board_ss, segment, '|')) {
        seglist.push_back(segment);
    }

    assert(seglist.size() >= 4);

    // Get general info
    rows = std::stoi(seglist[0]);
    cols = std::stoi(seglist[1]);
    assert(seglist.size() == static_cast<std::size_t>(rows * cols) + 3);
    gems_required = std::stoi(seglist[2]);

    // Parse grid
    int agent_counter = 0;
    for (std::size_t i = 3; i < seglist.size(); ++i) {
        const auto hidden_type = std::stoi(seglist[i]);
        if (hidden_type < 0 || hidden_type >= kNumHiddenCellType) {
            throw std::invalid_argument(std::format("Unknown element type: {:d}", hidden_type));
        }
        const auto el = static_cast<HiddenCellType>(hidden_type);
        grid.push_back(el);
        has_updated.push_back(false);
        // Really shouldn't be creating a state with the agent in the exit
        if (el == HiddenCellType::kAgent || el == HiddenCellType::kAgentInExit) {
            agent_idx = static_cast<int>(i) - 3;
            is_agent_alive = true;
            is_agent_in_exit = el == HiddenCellType::kAgentInExit;
            ++agent_counter;
        }
    }

    if (agent_counter == 0) {
        throw std::invalid_argument("Agent element not found");
    } else if (agent_counter > 1) {
        throw std::invalid_argument("Too many agent elements, expected only one");
    }
}

BoulderDashGameState::BoulderDashGameState(const std::string &board_str, const GameParameters &params)
    : magic_wall_steps(params.magic_wall_steps),
      butterfly_explosion_ver(params.butterfly_explosion_ver),
      butterfly_move_ver(params.butterfly_move_ver),
      random_state(splitmix64(0)),
      blob_chance(params.blob_chance),
      gravity(params.gravity),
      disable_explosions(params.disable_explosions) {
    parse_board_str(board_str);

    blob_max_size = static_cast<int>(static_cast<float>(cols * rows) * params.blob_max_percentage);

    // Initial hash
    int flat_size = rows * cols;
    for (int i : std::views::iota(0, flat_size)) {
        hash ^= to_local_hash(flat_size, grid[static_cast<std::size_t>(i)], i);
    }
}

BoulderDashGameState::BoulderDashGameState(InternalState &&internal_state)
    : magic_wall_steps(internal_state.magic_wall_steps),
      blob_max_size(internal_state.blob_max_size),
      butterfly_explosion_ver(internal_state.butterfly_explosion_ver),
      butterfly_move_ver(internal_state.butterfly_move_ver),
      gems_collected(internal_state.gems_collected),
      magic_wall_steps_remaining(internal_state.magic_wall_steps_remaining),
      blob_size(internal_state.blob_size),
      rows(internal_state.rows),
      cols(internal_state.cols),
      agent_idx(internal_state.agent_idx),
      gems_required(internal_state.gems_required),
      random_state(internal_state.random_state),
      reward_signal(internal_state.reward_signal),
      hash(internal_state.hash),
      blob_chance(internal_state.blob_chance),
      gravity(internal_state.gravity),
      disable_explosions(internal_state.disable_explosions),
      magic_active(internal_state.magic_active),
      blob_enclosed(internal_state.blob_enclosed),
      is_agent_alive(internal_state.is_agent_alive),
      is_agent_in_exit(internal_state.is_agent_in_exit),
      blob_swap(static_cast<HiddenCellType>(internal_state.blob_swap)),
      has_updated(std::move(internal_state).has_updated) {
    grid.clear();
    grid.reserve(internal_state.grid.size());
    for (const auto &el : internal_state.grid) {
        grid.push_back(static_cast<HiddenCellType>(el));
    }
}

const std::vector<Action> BoulderDashGameState::ALL_ACTIONS{
    Action::kNoop, Action::kUp, Action::kRight, Action::kDown, Action::kLeft,
};

// ---------------------------------------------------------------------------

void BoulderDashGameState::apply_action(Action action) {
    assert(is_valid_action(action));
    StartScan();

    // Handle agent first
    const Direction action_direction = action_to_direction(action);
    UpdateAgent(agent_idx, action_direction);

    // Handle all other items
    for (int i : std::views::iota(0, rows * cols)) {
        if (has_updated[static_cast<std::size_t>(i)]) {    // Item already updated
            continue;
        }
        const auto &hidden_el = grid[static_cast<std::size_t>(i)];
        switch (hidden_el) {
            // Handle non-compound types
            case HiddenCellType::kStone:
                UpdateStone(i);
                break;
            case HiddenCellType::kStoneFalling:
                UpdateStoneFalling(i);
                break;
            case HiddenCellType::kDiamond:
                UpdateDiamond(i);
                break;
            case HiddenCellType::kDiamondFalling:
                UpdateDiamondFalling(i);
                break;
            case HiddenCellType::kNut:
                UpdateNut(i);
                break;
            case HiddenCellType::kNutFalling:
                UpdateNutFalling(i);
                break;
            case HiddenCellType::kBomb:
                UpdateBomb(i);
                break;
            case HiddenCellType::kBombFalling:
                UpdateBombFalling(i);
                break;
            case HiddenCellType::kExitClosed:
                UpdateExit(i);
                break;
            case HiddenCellType::kBlob:
                UpdateBlob(i);
                break;
            default:
                // Handle compound types
                // NOLINTNEXTLINE(*-bounds-constant-array-index)
                const Element &element = kCellTypeToElement[static_cast<std::size_t>(hidden_el) + 1];
                if (IsButterfly(element)) {
                    UpdateButterfly(i, kButterflyToDirection.at(element));
                } else if (IsFirefly(element)) {
                    UpdateFirefly(i, kFireflyToDirection.at(element));
                } else if (IsOrange(element)) {
                    UpdateOrange(i, kOrangeToDirection.at(element));
                } else if (IsMagicWall(element)) {
                    UpdateMagicWall(i);
                } else if (IsExplosion(element)) {
                    UpdateExplosions(i);
                }
                break;
        }
    }

    EndScan();
}

auto BoulderDashGameState::is_terminal() const noexcept -> bool {
    // Terminal if agent not alive or agent is in exit
    return !is_agent_alive || is_agent_in_exit;
}

auto BoulderDashGameState::is_solution() const noexcept -> bool {
    // Solution if agent is in exit
    return is_agent_in_exit;
}

auto BoulderDashGameState::observation_shape() const noexcept -> std::array<int, 3> {
    return {kNumVisibleCellType, rows, cols};
}

auto BoulderDashGameState::get_observation() const noexcept -> std::vector<float> {
    auto channel_length = cols * rows;
    std::vector<float> obs(kNumVisibleCellType * channel_length, 0);
    for (int i : std::views::iota(0, channel_length)) {
        obs[static_cast<std::size_t>(GetItem(i).visible_type) * channel_length + i] = 1;
    }
    return obs;
}

// VisibleCellType to image binary data
#include "assets_all.inc"

auto BoulderDashGameState::image_shape() const noexcept -> std::array<int, 3> {
    return {rows * SPRITE_HEIGHT, cols * SPRITE_WIDTH, SPRITE_CHANNELS};
}

auto BoulderDashGameState::to_image() const noexcept -> std::vector<uint8_t> {
    const std::size_t flat_size = cols * rows;
    std::vector<uint8_t> img(flat_size * SPRITE_DATA_LEN, 0);
    for (int h : std::views::iota(0, rows)) {
        for (int w : std::views::iota(0, cols)) {
            const std::size_t img_idx_top_left = h * (SPRITE_DATA_LEN * cols) + (w * SPRITE_DATA_LEN_PER_ROW);
            const std::vector<uint8_t> &data = img_asset_map.at(GetItem(h * cols + w).visible_type);
            for (int r : std::views::iota(0, SPRITE_HEIGHT)) {
                for (int c : std::views::iota(0, SPRITE_WIDTH)) {
                    const std::size_t data_idx = (r * SPRITE_DATA_LEN_PER_ROW) + (3 * c);
                    const std::size_t img_idx =
                        (r * SPRITE_DATA_LEN_PER_ROW * cols) + (SPRITE_CHANNELS * c) + img_idx_top_left;
                    img[img_idx + 0] = data[data_idx + 0];
                    img[img_idx + 1] = data[data_idx + 1];
                    img[img_idx + 2] = data[data_idx + 2];
                }
            }
        }
    }
    return img;
}

auto BoulderDashGameState::get_reward_signal() const noexcept -> uint64_t {
    return reward_signal;
}

auto BoulderDashGameState::get_hash() const noexcept -> uint64_t {
    return hash;
}

auto BoulderDashGameState::get_positions(HiddenCellType element) const noexcept -> std::vector<Position> {
    assert(is_valid_hidden_element(element));
    std::vector<Position> positions;
    for (int idx : std::views::iota(0, rows * cols)) {
        if (grid[static_cast<std::size_t>(idx)] == element) {
            positions.emplace_back(idx / cols, idx % cols);
        }
    }
    return positions;
}

auto BoulderDashGameState::position_to_index(const Position &position) const -> int {
    if (position.first < 0 || position.first >= rows || position.second < 0 || position.second >= cols) {
        throw std::invalid_argument(std::format("Invalid positiom ({:d}, {:d}) for map size ({:d}, {:d})",
                                                position.first, position.second, rows, cols));
    }
    return position.first * cols + position.second;
}

auto BoulderDashGameState::index_to_position(int index) const -> Position {
    if (index < 0 || index >= rows * cols) {
        throw std::invalid_argument(std::format("Invalid index {:d} for map size ({:d}, {:d})", index, rows, cols));
    }
    return {index / cols, index % cols};
}

auto BoulderDashGameState::get_indices(HiddenCellType element) const noexcept -> std::vector<int> {
    assert(is_valid_hidden_element(element));
    std::vector<int> indices;
    for (int idx : std::views::iota(0, rows * cols)) {
        if (grid[static_cast<std::size_t>(idx)] == element) {
            indices.push_back(idx);
        }
    }
    return indices;
}

auto BoulderDashGameState::is_pos_in_bounds(const Position &position) const noexcept -> bool {
    return position.first >= 0 && position.first < rows && position.second >= 0 && position.second < cols;
}

auto BoulderDashGameState::agent_alive() const noexcept -> bool {
    return is_agent_alive;
}

auto BoulderDashGameState::agent_in_exit() const noexcept -> bool {
    return is_agent_in_exit;
}

auto BoulderDashGameState::get_agent_index() const noexcept -> int {
    return agent_idx;
}

auto BoulderDashGameState::get_hidden_item(int index) const -> HiddenCellType {
    if (index < 0 || index >= rows * cols) {
        throw std::invalid_argument(std::format("Invalid index {:d} for map size ({:d}, {:d})", index, rows, cols));
    }
    return grid[static_cast<std::size_t>(index)];
}

auto operator<<(std::ostream &os, const GameParameters &params) -> std::ostream & {
    os << "{\n";
    os << std::format("  gravity: {}\n", params.gravity);
    os << std::format("  magic_wall_steps: {:d}\n", params.magic_wall_steps);
    os << std::format("  blob_chance: {:d}\n", params.blob_chance);
    os << std::format("  blob_max_percentage: {:f}\n", params.blob_max_percentage);
    os << std::format("  disable_explosions: {}\n", params.disable_explosions);
    os << std::format("  butterfly_explosion_ver: {:d}\n", params.butterfly_explosion_ver);
    os << std::format("  butterfly_move_ver: {:d}\n", params.butterfly_move_ver);
    os << "}";
    return os;
}

auto operator<<(std::ostream &os, const BoulderDashGameState &state) -> std::ostream & {
    const auto print_horz_boarder = [&]() {
        for (int w = 0; w < state.cols + 2; ++w) {
            os << "-";
        }
        os << std::endl;
    };
    print_horz_boarder();
    for (int h : std::views::iota(0, state.rows)) {
        os << "|";
        for (int w : std::views::iota(0, state.cols)) {
            // NOLINTNEXTLINE(*-bounds-constant-array-index)
            os << kCellTypeToElement[static_cast<std::size_t>(state.grid[h * state.cols + w]) + 1].id;
        }
        os << "|" << std::endl;
    }
    print_horz_boarder();
    return os;
}

// ---------------------------------------------------------------------------

// Not safe, assumes InBounds has been called (or used in conjunction)
auto BoulderDashGameState::IndexFromDirection(int index, Direction direction) const noexcept -> int {
    switch (direction) {
        case Direction::kNoop:
            return index;
        case Direction::kUp:
            return index - cols;
        case Direction::kRight:
            return index + 1;
        case Direction::kDown:
            return index + cols;
        case Direction::kLeft:
            return index - 1;
        case Direction::kUpRight:
            return index - cols + 1;
        case Direction::kDownRight:
            return index + cols + 1;
        case Direction::kUpLeft:
            return index - cols - 1;
        case Direction::kDownLeft:
            return index + cols - 1;
        default:
            unreachable();
    }
}

auto BoulderDashGameState::InBounds(int index, Direction direction) const noexcept -> bool {
    int col = index % cols;
    int row = (index - col) / cols;
    const auto &offsets = kDirectionOffsets[static_cast<std::size_t>(direction)];    // NOLINT(*-array-index)
    col += offsets.first;
    row += offsets.second;
    return col >= 0 && col < static_cast<int>(cols) && row >= 0 && row < static_cast<int>(rows);
}

auto BoulderDashGameState::IsType(int index, const Element &element, Direction direction) const noexcept -> bool {
    auto new_index = IndexFromDirection(index, direction);
    return InBounds(index, direction) && GetItem(new_index) == element;
}

auto BoulderDashGameState::HasProperty(int index, int property, Direction direction) const noexcept -> bool {
    auto new_index = IndexFromDirection(index, direction);
    return InBounds(index, direction) && ((GetItem(new_index).properties & property) > 0);
}

void BoulderDashGameState::MoveItem(int index, Direction direction) noexcept {
    auto new_index = IndexFromDirection(index, direction);
    auto flat_size = rows * cols;
    hash ^= to_local_hash(flat_size, grid[static_cast<std::size_t>(new_index)], new_index);
    grid[static_cast<std::size_t>(new_index)] = grid[static_cast<std::size_t>(index)];
    hash ^= to_local_hash(flat_size, grid[static_cast<std::size_t>(new_index)], new_index);

    hash ^= to_local_hash(flat_size, grid[static_cast<std::size_t>(index)], index);
    grid[static_cast<std::size_t>(index)] = kElEmpty.cell_type;
    hash ^= to_local_hash(flat_size, kElEmpty.cell_type, index);

    has_updated[new_index] = true;
}

void BoulderDashGameState::SetItem(int index, const Element &element, Direction direction) noexcept {
    auto new_index = IndexFromDirection(index, direction);
    auto flat_size = rows * cols;
    hash ^= to_local_hash(flat_size, grid[static_cast<std::size_t>(new_index)], new_index);
    grid[static_cast<std::size_t>(new_index)] = element.cell_type;
    hash ^= to_local_hash(flat_size, element.cell_type, new_index);
    has_updated[new_index] = true;
}

auto BoulderDashGameState::GetItem(int index, Direction direction) const noexcept -> const Element & {
    auto new_index = static_cast<std::size_t>(IndexFromDirection(index, direction));
    // NOLINTNEXTLINE(*-bounds-constant-array-index)
    return kCellTypeToElement[static_cast<std::size_t>(grid[new_index]) + 1];
}

auto BoulderDashGameState::IsTypeAdjacent(int index, const Element &element) const noexcept -> bool {
    return IsType(index, element, Direction::kUp) || IsType(index, element, Direction::kLeft) ||
           IsType(index, element, Direction::kDown) || IsType(index, element, Direction::kRight);
}

// ---------------------------------------------------------------------------

auto BoulderDashGameState::CanRollLeft(int index) const noexcept -> bool {
    return HasProperty(index, ElementProperties::kRounded, Direction::kDown) &&
           IsType(index, kElEmpty, Direction::kLeft) && IsType(index, kElEmpty, Direction::kDownLeft);
}

auto BoulderDashGameState::CanRollRight(int index) const noexcept -> bool {
    return HasProperty(index, ElementProperties::kRounded, Direction::kDown) &&
           IsType(index, kElEmpty, Direction::kRight) && IsType(index, kElEmpty, Direction::kDownRight);
}

void BoulderDashGameState::RollLeft(int index, const Element &element) noexcept {
    SetItem(index, element);
    MoveItem(index, Direction::kLeft);
}

void BoulderDashGameState::RollRight(int index, const Element &element) noexcept {
    SetItem(index, element);
    MoveItem(index, Direction::kRight);
}

void BoulderDashGameState::Push(int index, const Element &stationary, const Element &falling,
                                Direction direction) noexcept {
    auto new_index = IndexFromDirection(index, direction);
    // Check if same direction past element is empty so that theres room to push
    if (IsType(new_index, kElEmpty, direction)) {
        // Check if the element will become stationary or falling
        auto next_index = IndexFromDirection(new_index, direction);
        bool is_empty = IsType(next_index, kElEmpty, Direction::kDown);
        // Move item and set as falling or stationary
        MoveItem(new_index, direction);
        SetItem(next_index, is_empty ? falling : stationary);
        // Move the agent
        MoveItem(index, direction);
        agent_idx = IndexFromDirection(index, direction);    // Assume only agent is pushing?
    }
}

void BoulderDashGameState::MoveThroughMagic(int index, const Element &element) noexcept {
    // Check if magic wall is still active
    if (magic_wall_steps <= 0) {
        return;
    }
    magic_active = true;
    auto index_wall = IndexFromDirection(index, Direction::kDown);
    auto index_under_wall = IndexFromDirection(index_wall, Direction::kDown);
    // Need to ensure cell below magic wall is empty (so item can pass through)
    if (IsType(index_under_wall, kElEmpty)) {
        SetItem(index, kElEmpty);
        SetItem(index_under_wall, element);
    }
}

// NOLINTNEXTLINE (mi-no-recursion)
void BoulderDashGameState::Explode(int index, const Element &element, Direction direction) noexcept {
    auto new_index = IndexFromDirection(index, direction);
    const auto &it = kElementToExplosion.find(GetItem(new_index));
    const Element &ex = (it == kElementToExplosion.end()) ? kElExplosionEmpty : it->second;
    if (GetItem(new_index) == kElAgent) {
        is_agent_alive = false;
    }
    SetItem(new_index, element);
    // Recursively check all directions for chain explosions
    for (int dir_index : std::views::iota(0, kNumDirections)) {
        auto dir = static_cast<Direction>(dir_index);
        if (dir == Direction::kNoop || !InBounds(new_index, dir)) {
            continue;
        }
        if (HasProperty(new_index, ElementProperties::kCanExplode, dir)) {
            Explode(new_index, ex, dir);
        } else if (HasProperty(new_index, ElementProperties::kConsumable, dir)) {
            SetItem(new_index, ex, dir);
            if (GetItem(new_index, dir) == kElAgent) {
                is_agent_alive = false;
            }
        }
    }
}

// ---------------------------------------------------------------------------

void BoulderDashGameState::UpdateStone(int index) noexcept {
    // If no gravity, do nothing
    if (!gravity) {
        return;
    }

    // Boulder falls if empty below
    if (IsType(index, kElEmpty, Direction::kDown)) {
        SetItem(index, kElStoneFalling);
        UpdateStoneFalling(index);
    } else if (CanRollLeft(index)) {    // Roll left/right if possible
        RollLeft(index, kElStoneFalling);
    } else if (CanRollRight(index)) {
        RollRight(index, kElStoneFalling);
    }
}

void BoulderDashGameState::UpdateStoneFalling(int index) noexcept {
    // Continue to fall as normal
    if (IsType(index, kElEmpty, Direction::kDown)) {
        MoveItem(index, Direction::kDown);
    } else if (butterfly_explosion_ver == ButterflyExplosionVersion::kConvert &&
               IsButterfly(GetItem(index, Direction::kDown))) {
        // Falling on a butterfly, destroy it open to reveal a diamond!
        SetItem(index, kElEmpty);
        SetItem(index, kElDiamond, Direction::kDown);
        reward_signal |= RewardCodes::kRewardButterflyToDiamond;
    } else if (HasProperty(index, ElementProperties::kCanExplode, Direction::kDown)) {
        // Falling stones can cause elements to explode
        const auto it = kElementToExplosion.find(GetItem(index, Direction::kDown));
        Explode(index, (it == kElementToExplosion.end()) ? kElExplosionEmpty : it->second, Direction::kDown);
    } else if (IsType(index, kElWallMagicOn, Direction::kDown) ||
               IsType(index, kElWallMagicDormant, Direction::kDown)) {
        MoveThroughMagic(index, kMagicWallConversion.at(GetItem(index)));
    } else if (IsType(index, kElNut, Direction::kDown)) {
        // Falling on a nut, crack it open to reveal a diamond!
        SetItem(index, kElDiamond, Direction::kDown);
        reward_signal |= RewardCodes::kRewardNutToDiamond;
    } else if (IsType(index, kElBomb, Direction::kDown)) {
        // Falling on a bomb, explode!
        const auto it = kElementToExplosion.find(GetItem(index));
        Explode(index, (it == kElementToExplosion.end()) ? kElExplosionEmpty : it->second);
    } else if (CanRollLeft(index)) {    // Roll left/right
        RollLeft(index, kElStoneFalling);
    } else if (CanRollRight(index)) {
        RollRight(index, kElStoneFalling);
    } else {
        // Default options is for falling stones to become stationary
        SetItem(index, kElStone);
    }
}

void BoulderDashGameState::UpdateDiamond(int index) noexcept {
    // If no gravity, do nothing
    if (!gravity) {
        return;
    }

    // Diamond falls if empty below
    if (IsType(index, kElEmpty, Direction::kDown)) {
        SetItem(index, kElDiamondFalling);
        UpdateDiamondFalling(index);
    } else if (CanRollLeft(index)) {    // Roll left/right if possible
        RollLeft(index, kElDiamondFalling);
    } else if (CanRollRight(index)) {
        RollRight(index, kElDiamondFalling);
    }
}

void BoulderDashGameState::UpdateDiamondFalling(int index) noexcept {
    // Continue to fall as normal
    if (IsType(index, kElEmpty, Direction::kDown)) {
        MoveItem(index, Direction::kDown);
    } else if (HasProperty(index, ElementProperties::kCanExplode, Direction::kDown) &&
               !IsType(index, kElBomb, Direction::kDown) && !IsType(index, kElBombFalling, Direction::kDown)) {
        // Falling diamonds can cause elements to explode (but not bombs)
        const auto it = kElementToExplosion.find(GetItem(index, Direction::kDown));
        Explode(index, (it == kElementToExplosion.end()) ? kElExplosionEmpty : it->second, Direction::kDown);
    } else if (IsType(index, kElWallMagicOn, Direction::kDown) ||
               IsType(index, kElWallMagicDormant, Direction::kDown)) {
        MoveThroughMagic(index, kMagicWallConversion.at(GetItem(index)));
    } else if (CanRollLeft(index)) {    // Roll left/right
        RollLeft(index, kElDiamondFalling);
    } else if (CanRollRight(index)) {
        RollRight(index, kElDiamondFalling);
    } else {
        // Default options is for falling diamond to become stationary
        SetItem(index, kElDiamond);
    }
}

void BoulderDashGameState::UpdateNut(int index) noexcept {
    // If no gravity, do nothing
    if (!gravity) {
        return;
    }

    // Nut falls if empty below
    if (IsType(index, kElEmpty, Direction::kDown)) {
        SetItem(index, kElNutFalling);
        UpdateNutFalling(index);
    } else if (CanRollLeft(index)) {    // Roll left/right
        RollLeft(index, kElNutFalling);
    } else if (CanRollRight(index)) {
        RollRight(index, kElNutFalling);
    }
}

void BoulderDashGameState::UpdateNutFalling(int index) noexcept {
    // Continue to fall as normal
    if (IsType(index, kElEmpty, Direction::kDown)) {
        MoveItem(index, Direction::kDown);
    } else if (CanRollLeft(index)) {    // Roll left/right
        RollLeft(index, kElNutFalling);
    } else if (CanRollRight(index)) {
        RollRight(index, kElNutFalling);
    } else {
        // Default options is for falling nut to become stationary
        SetItem(index, kElNut);
    }
}

void BoulderDashGameState::UpdateBomb(int index) noexcept {
    // If no gravity, do nothing
    if (!gravity) {
        return;
    }

    // Bomb falls if empty below
    if (IsType(index, kElEmpty, Direction::kDown)) {
        SetItem(index, kElBombFalling);
        UpdateBombFalling(index);
    } else if (CanRollLeft(index)) {    // Roll left/right
        RollLeft(index, kElBomb);
    } else if (CanRollRight(index)) {
        RollRight(index, kElBomb);
    }
}

void BoulderDashGameState::UpdateBombFalling(int index) noexcept {
    // Continue to fall as normal
    if (IsType(index, kElEmpty, Direction::kDown)) {
        MoveItem(index, Direction::kDown);
    } else if (CanRollLeft(index)) {    // Roll left/right
        RollLeft(index, kElBombFalling);
    } else if (CanRollRight(index)) {
        RollRight(index, kElBombFalling);
    } else if (!disable_explosions) {
        // Default options is for bomb to explode if stopped falling
        const auto it = kElementToExplosion.find(GetItem(index));
        Explode(index, (it == kElementToExplosion.end()) ? kElExplosionEmpty : it->second);
    }
}

void BoulderDashGameState::UpdateExit(int index) noexcept {
    // Open exit if enough gems collected
    if (gems_collected >= gems_required) {
        SetItem(index, kElExitOpen);
    }
}

void BoulderDashGameState::UpdateAgent(int index, Direction direction) noexcept {
    // If action results not in bounds, don't do anything
    if (!InBounds(index, direction)) {
        return;
    }

    if (IsType(index, kElEmpty, direction) || IsType(index, kElDirt, direction)) {    // Move if empty/dirt
        MoveItem(index, direction);
        agent_idx = IndexFromDirection(index, direction);
    } else if (IsType(index, kElDiamond, direction) || IsType(index, kElDiamondFalling, direction)) {    // Collect gems
        ++gems_collected;
        reward_signal |= RewardCodes::kRewardCollectDiamond;
        MoveItem(index, direction);
        agent_idx = IndexFromDirection(index, direction);
    } else if (IsDirectionHorz(direction) && HasProperty(index, ElementProperties::kPushable, direction)) {
        // Push stone, nut, or bomb if action is horizontal
        Push(index, GetItem(index, direction), kElToFalling.at(GetItem(index, direction)), direction);
    } else if (IsKey(GetItem(index, direction))) {
        // Collecting key, set gate open
        const Element &key_type = GetItem(index, direction);
        OpenGate(kKeyToGate.at(key_type));
        MoveItem(index, direction);
        agent_idx = IndexFromDirection(index, direction);
        reward_signal |= RewardCodes::kRewardCollectKey;
        reward_signal |= static_cast<uint64_t>(kKeyToSignal.at(key_type));
    } else if (IsOpenGate(GetItem(index, direction))) {
        // Walking through an open gate, with traversable element on other side
        auto index_gate = IndexFromDirection(index, direction);
        if (HasProperty(index_gate, ElementProperties::kTraversable, direction)) {
            // Correct for landing on traversable elements
            if (IsType(index_gate, kElDiamond, direction) || IsType(index_gate, kElDiamondFalling, direction)) {
                ++gems_collected;
                reward_signal |= RewardCodes::kRewardCollectDiamond;
            } else if (IsKey(GetItem(index_gate, direction))) {
                const Element &key_type = GetItem(index_gate, direction);
                OpenGate(kKeyToGate.at(key_type));
                reward_signal |= RewardCodes::kRewardCollectKey;
                reward_signal |= static_cast<uint64_t>(kKeyToSignal.at(key_type));
            }
            // Move agent through gate
            SetItem(index_gate, kElAgent, direction);
            SetItem(index, kElEmpty);
            agent_idx = IndexFromDirection(index_gate, direction);
            reward_signal |= RewardCodes::kRewardWalkThroughGate;
            reward_signal |= static_cast<uint64_t>(kGateToSignal.at(GetItem(index_gate)));
        }
    } else if (IsType(index, kElExitOpen, direction)) {
        // Walking into exit after collecting enough gems
        MoveItem(index, direction);
        SetItem(index, kElAgentInExit, direction);
        agent_idx = IndexFromDirection(index, direction);
        is_agent_in_exit = true;
        reward_signal |= RewardCodes::kRewardWalkThroughExit;
    }
}

void BoulderDashGameState::UpdateFirefly(int index, Direction direction) noexcept {
    // NOLINTBEGIN(*-bounds-constant-array-index)
    const Direction new_dir = kRotateLeft[static_cast<std::size_t>(direction)];
    if (IsTypeAdjacent(index, kElAgent) || IsTypeAdjacent(index, kElBlob)) {
        // Explode if touching the agent/blob
        const auto it = kElementToExplosion.find(GetItem(index));
        Explode(index, (it == kElementToExplosion.end()) ? kElExplosionEmpty : it->second);
    } else if (IsType(index, kElEmpty, new_dir)) {
        // Fireflies always try to rotate left, otherwise continue forward
        SetItem(index, kDirectionToFirefly[static_cast<std::size_t>(new_dir)]);
        MoveItem(index, new_dir);
    } else if (IsType(index, kElEmpty, direction)) {
        SetItem(index, kDirectionToFirefly[static_cast<std::size_t>(direction)]);
        MoveItem(index, direction);
    } else {
        // No other options, rotate right
        SetItem(index,
                kDirectionToFirefly[static_cast<std::size_t>(kRotateRight[static_cast<std::size_t>(direction)])]);
    }
    // NOLINTEND(*-bounds-constant-array-index)
}

void BoulderDashGameState::UpdateButterfly(int index, Direction direction) noexcept {
    // NOLINTBEGIN(*-bounds-constant-array-index)
    const Direction new_dir = kRotateRight[static_cast<std::size_t>(direction)];
    if (IsTypeAdjacent(index, kElAgent) || IsTypeAdjacent(index, kElBlob)) {
        // Explode if touching the agent/blob
        const auto it = kElementToExplosion.find(GetItem(index));
        Explode(index, (it == kElementToExplosion.end()) ? kElExplosionEmpty : it->second);
    } else if (IsType(index, kElEmpty, new_dir)) {
        // Butterflies always try to rotate right, otherwise continue forward
        SetItem(index, kDirectionToButterfly[static_cast<std::size_t>(new_dir)]);
        MoveItem(index, new_dir);
    } else if (IsType(index, kElEmpty, direction)) {
        SetItem(index, kDirectionToButterfly[static_cast<std::size_t>(direction)]);
        MoveItem(index, direction);
    } else {
        // No other options, rotate left
        SetItem(index,
                kDirectionToButterfly[static_cast<std::size_t>(kRotateLeft[static_cast<std::size_t>(direction)])]);
        if (butterfly_move_ver == ButterflyMoveVersion::kInstant) {
            const auto new_dir = kRotateLeft[static_cast<std::size_t>(direction)];
            MoveItem(index, new_dir);
        }
    }
    // NOLINTEND(*-bounds-constant-array-index)
}

void BoulderDashGameState::UpdateOrange(int index, Direction direction) noexcept {
    if (IsType(index, kElEmpty, direction)) {
        // Continue moving in direction
        MoveItem(index, direction);
    } else if (IsTypeAdjacent(index, kElAgent)) {
        // Run into the agent, explode!
        const auto it = kElementToExplosion.find(GetItem(index));
        Explode(index, (it == kElementToExplosion.end()) ? kElExplosionEmpty : it->second);
    } else {
        // Blocked, roll for new direction
        std::vector<Direction> open_dirs;
        for (int dir_index = 0; dir_index < kNumActions; ++dir_index) {
            const auto dir = static_cast<Direction>(dir_index);
            if (dir == Direction::kNoop || !InBounds(index, dir)) {
                continue;
            }
            if (IsType(index, kElEmpty, dir)) {
                open_dirs.push_back(dir);
            }
        }
        // Roll available directions
        if (!open_dirs.empty()) {
            const Direction new_dir = open_dirs[xorshift64(random_state) % open_dirs.size()];
            // NOLINTNEXTLINE(*-bounds-constant-array-index)
            SetItem(index, kDirectionToOrange[static_cast<std::size_t>(new_dir)]);
        }
    }
}

void BoulderDashGameState::UpdateMagicWall(int index) noexcept {
    // Dorminant, active, then expired once time runs out
    if (magic_active) {
        SetItem(index, kElWallMagicOn);
    } else if (magic_wall_steps > 0) {
        SetItem(index, kElWallMagicDormant);
    } else {
        SetItem(index, kElWallMagicExpired);
    }
}

constexpr int BASE_CHANCE = 256;

void BoulderDashGameState::UpdateBlob(int index) noexcept {
    // Replace blobs if swap element set
    if (blob_swap != kNullElement.cell_type) {
        // NOLINTNEXTLINE(*-bounds-constant-array-index)
        SetItem(index, kCellTypeToElement[static_cast<std::size_t>(blob_swap) + 1]);
        return;
    }
    ++blob_size;
    // Check if at least one tile blob can grow to
    if (IsTypeAdjacent(index, kElEmpty) || IsTypeAdjacent(index, kElDirt)) {
        blob_enclosed = false;
    }
    // Roll if to grow and direction
    bool will_grow = (xorshift64(random_state) % BASE_CHANCE) < blob_chance;
    auto grow_dir = static_cast<Direction>(xorshift64(random_state) % kNumActions);
    if (will_grow && (IsType(index, kElEmpty, grow_dir) || IsType(index, kElDirt, grow_dir))) {
        SetItem(index, kElBlob, grow_dir);
    }
}

void BoulderDashGameState::UpdateExplosions(int index) noexcept {
    reward_signal |= kExplosionToReward.at(GetItem(index));
    SetItem(index, kExplosionToElement.at(GetItem(index)));
}

void BoulderDashGameState::OpenGate(const Element &element) noexcept {
    for (int index : std::views::iota(0, rows * cols)) {
        if (grid[static_cast<std::size_t>(index)] == element.cell_type) {
            SetItem(index, kGateOpenMap.at(GetItem(index)));
        }
    }
}

// ---------------------------------------------------------------------------

void BoulderDashGameState::StartScan() noexcept {
    blob_size = 0;
    blob_enclosed = true;
    reward_signal = 0;
    for (int i : std::views::iota(0, rows * cols)) {
        has_updated[static_cast<std::size_t>(i)] = false;
    }
}

void BoulderDashGameState::EndScan() noexcept {
    if (blob_swap == kNullElement.cell_type) {
        if (blob_enclosed) {
            blob_swap = kElDiamond.cell_type;
        }
        if (blob_size > blob_max_size) {
            blob_swap = kElStone.cell_type;
        }
    }
    if (magic_active) {
        magic_wall_steps = static_cast<decltype(magic_wall_steps)>(std::max(magic_wall_steps - 1, 0));
    }
    magic_active = magic_active && (magic_wall_steps > 0);
}

}    // namespace boulderdash
