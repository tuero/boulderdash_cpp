#ifndef BOULDERDASH_BASE_H_
#define BOULDERDASH_BASE_H_

#include <array>
#include <cstdint>
#include <iostream>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>

#include "definitions.h"

namespace boulderdash {

constexpr int SPRITE_WIDTH = 32;
constexpr int SPRITE_HEIGHT = 32;
constexpr int SPRITE_CHANNELS = 3;
constexpr int SPRITE_DATA_LEN_PER_ROW = SPRITE_WIDTH * SPRITE_CHANNELS;
constexpr int SPRITE_DATA_LEN = SPRITE_WIDTH * SPRITE_HEIGHT * SPRITE_CHANNELS;

// Default game parameters
constexpr int DEFAULT_MAGIC_WALL_STEPS = 140;
constexpr int DEFAULT_BLOB_CHANCE = 20;
constexpr float DEFAULT_BLOB_MAX_PERCENTAGE = static_cast<float>(0.16);
constexpr bool DEFAULT_GRAVITY = false;
constexpr bool DEFAULT_DISABLE_EXPLOSIONS = false;
constexpr int DEFAULT_BUTTERFLY_EXPLOSION_VER = ButterflyExplosionVersion::kExplode;
constexpr int DEFAULT_BUTTERFLY_MOVE_VER = ButterflyMoveVersion::kDelay;

struct GameParameters {
    bool gravity = DEFAULT_GRAVITY;
    int magic_wall_steps = DEFAULT_MAGIC_WALL_STEPS;
    int blob_chance = DEFAULT_BLOB_CHANCE;
    float blob_max_percentage = DEFAULT_BLOB_MAX_PERCENTAGE;
    bool disable_explosions = DEFAULT_DISABLE_EXPLOSIONS;
    int butterfly_explosion_ver = DEFAULT_BUTTERFLY_EXPLOSION_VER;
    int butterfly_move_ver = DEFAULT_BUTTERFLY_MOVE_VER;
    friend auto operator<<(std::ostream &os, const GameParameters &params) -> std::ostream &;
};

// Game state
class BoulderDashGameState {
public:
    // Internal use for packing/unpacking with pybind11 pickle
    struct InternalState {
        int magic_wall_steps;
        int blob_max_size;
        int butterfly_explosion_ver;
        int butterfly_move_ver;
        int gems_collected;
        int magic_wall_steps_remaining;
        int blob_size;
        int rows;
        int cols;
        int agent_idx;
        int gems_required;
        uint64_t random_state;
        uint64_t reward_signal;
        uint64_t hash;
        uint8_t blob_chance;
        bool gravity;
        bool disable_explosions;
        bool magic_active;
        bool blob_enclosed;
        bool is_agent_alive;
        bool is_agent_in_exit;
        int8_t blob_swap;
        std::vector<int8_t> grid;
        std::vector<bool> has_updated;
    };

    using Position = std::pair<int, int>;

    BoulderDashGameState() = delete;
    BoulderDashGameState(const std::string &board_str, const GameParameters &params = {});
    BoulderDashGameState(InternalState &&internal_state);

    auto operator==(const BoulderDashGameState &other) const -> bool = default;
    auto operator!=(const BoulderDashGameState &other) const -> bool = default;

    static inline std::string name = "boulderdash";

    /**
     * Check if the given visible element is valid.
     * @param element Element to check
     * @return True if element is valid, false otherwise
     */
    [[nodiscard]] constexpr static auto is_valid_visible_element(VisibleCellType element) -> bool {
        return static_cast<int>(element) >= 0 && static_cast<int>(element) < static_cast<int>(kNumVisibleCellType);
    }

    /**
     * Check if the given hidden element is valid.
     * @param element Element to check
     * @return True if element is valid, false otherwise
     */
    [[nodiscard]] constexpr static auto is_valid_hidden_element(HiddenCellType element) -> bool {
        return static_cast<int>(element) >= 0 && static_cast<int>(element) < static_cast<int>(kNumVisibleCellType);
    }

    /**
     * Check if the given action is valid.
     * @param action Action to check
     * @return True if action is valid, false otherwise
     */
    [[nodiscard]] constexpr static auto is_valid_action(Action action) -> bool {
        return static_cast<int>(action) >= 0 && static_cast<int>(action) < static_cast<int>(kNumActions);
    }

    /**
     * Apply the action to the current state, and set the reward and signals.
     * @param action The action to apply, should be one of the legal actions
     */
    void apply_action(Action action);

    /**
     * Get the number of possible actions
     * @return Count of possible actions
     */
    [[nodiscard]] constexpr static auto action_space_size() noexcept -> int {
        return kNumActions;
    }

    /**
     * Check if the state is terminal, meaning either solution, timeout, or agent dies.
     * @return True if terminal, false otherwise
     */
    [[nodiscard]] auto is_terminal() const noexcept -> bool;

    /**
     * Check if the state is in the solution state (agent inside exit).
     * @return True if terminal, false otherwise
     */
    [[nodiscard]] auto is_solution() const noexcept -> bool;

    /**
     * Get the shape the observations should be viewed as.
     * @return vector indicating observation CHW
     */
    [[nodiscard]] auto observation_shape() const noexcept -> std::array<int, 3>;

    /**
     * Get a flat representation of the current state observation.
     * The observation should be viewed as the shape given by observation_shape().
     * @return vector where 1 represents object at position
     */
    [[nodiscard]] auto get_observation() const noexcept -> std::vector<float>;

    /**
     * Get the index corresponding to the given position
     * @return the flat index
     */
    [[nodiscard]] auto position_to_index(const Position &position) const -> int;

    /**
     * Get the position corresponding to the given index
     * @return The position
     */
    [[nodiscard]] auto index_to_position(int index) const -> Position;

    /**
     * Get the shape the image should be viewed as.
     * @return array indicating observation HWC
     */
    [[nodiscard]] auto image_shape() const noexcept -> std::array<int, 3>;

    /**
     * Get the flat (HWC) image representation of the current state
     * @return flattened byte vector represending RGB values (HWC)
     */
    [[nodiscard]] auto to_image() const noexcept -> std::vector<uint8_t>;

    /**
     * Get the current reward signal as a result of the previous action taken.
     * @return bit field representing events that occured
     */
    [[nodiscard]] auto get_reward_signal() const noexcept -> uint64_t;

    /**
     * Get the hash representation for the current state.
     * @return hash value
     */
    [[nodiscard]] auto get_hash() const noexcept -> uint64_t;

    /**
     * Get all positions for a given element type
     * @param element The hidden cell type of the element to search for
     * @return pair of positions for each instance of element
     */
    [[nodiscard]] auto get_positions(HiddenCellType element) const noexcept -> std::vector<Position>;

    /**
     * Get all indices for a given element type
     * @param element The hidden cell type of the element to search for
     * @return flat indices for each instance of element
     */
    [[nodiscard]] auto get_indices(HiddenCellType element) const noexcept -> std::vector<int>;

    /**
     * Check if a given position is in bounds
     * @param position The position to check
     * @return True if the position is in bounds, false otherwise
     */
    [[nodiscard]] auto is_pos_in_bounds(const Position &position) const noexcept -> bool;

    /**
     * Check if the agent is alive
     */
    [[nodiscard]] auto agent_alive() const noexcept -> bool;

    /**
     * Check if the agent is in the exit
     */
    [[nodiscard]] auto agent_in_exit() const noexcept -> bool;

    /**
     * Get the agent index position, even if in exit or just died
     * @return Agent index
     */
    [[nodiscard]] auto get_agent_index() const noexcept -> int;

    /**
     * Get the hidden cell item at the given index
     */
    [[nodiscard]] auto get_hidden_item(int index) const -> HiddenCellType;

    friend auto operator<<(std::ostream &os, const BoulderDashGameState &state) -> std::ostream &;

    [[nodiscard]] auto pack() const -> InternalState {
        std::vector<decltype(to_underlying(grid[0]))> _grid;
        _grid.reserve(grid.size());
        for (const auto &el : grid) {
            _grid.push_back(to_underlying(el));
        }
        return {
            .magic_wall_steps = magic_wall_steps,
            .blob_max_size = blob_max_size,
            .butterfly_explosion_ver = butterfly_explosion_ver,
            .butterfly_move_ver = butterfly_move_ver,
            .gems_collected = gems_collected,
            .magic_wall_steps_remaining = magic_wall_steps_remaining,
            .blob_size = blob_size,
            .rows = rows,
            .cols = cols,
            .agent_idx = agent_idx,
            .gems_required = gems_required,
            .random_state = random_state,
            .reward_signal = reward_signal,
            .hash = hash,
            .blob_chance = blob_chance,
            .gravity = gravity,
            .disable_explosions = disable_explosions,
            .magic_active = magic_active,
            .blob_enclosed = blob_enclosed,
            .is_agent_alive = is_agent_alive,
            .is_agent_in_exit = is_agent_in_exit,
            .blob_swap = to_underlying(blob_swap),
            .grid = std::move(_grid),
            .has_updated = has_updated,
        };
    }

private:
    [[nodiscard]] auto IndexFromDirection(int index, Direction direction) const noexcept -> int;
    [[nodiscard]] auto InBounds(int index, Direction direction = Direction::kNoop) const noexcept -> bool;
    [[nodiscard]] auto IsType(int index, const Element &element, Direction direction = Direction::kNoop) const noexcept
        -> bool;
    [[nodiscard]] auto HasProperty(int index, int property, Direction direction = Direction::kNoop) const noexcept
        -> bool;
    void MoveItem(int index, Direction direction) noexcept;
    void SetItem(int index, const Element &element, Direction direction = Direction::kNoop) noexcept;
    [[nodiscard]] auto GetItem(int index, Direction direction = Direction::kNoop) const noexcept -> const Element &;
    [[nodiscard]] auto IsTypeAdjacent(int index, const Element &element) const noexcept -> bool;

    [[nodiscard]] auto CanRollLeft(int index) const noexcept -> bool;
    [[nodiscard]] auto CanRollRight(int index) const noexcept -> bool;
    void RollLeft(int index, const Element &element) noexcept;
    void RollRight(int index, const Element &element) noexcept;
    void Push(int index, const Element &stationary, const Element &falling, Direction direction) noexcept;
    void MoveThroughMagic(int index, const Element &element) noexcept;
    void Explode(int index, const Element &element, Direction direction = Direction::kNoop) noexcept;

    void UpdateStone(int index) noexcept;
    void UpdateStoneFalling(int index) noexcept;
    void UpdateDiamond(int index) noexcept;
    void UpdateDiamondFalling(int index) noexcept;
    void UpdateNut(int index) noexcept;
    void UpdateNutFalling(int index) noexcept;
    void UpdateBomb(int index) noexcept;
    void UpdateBombFalling(int index) noexcept;
    void UpdateExit(int index) noexcept;
    void UpdateAgent(int index, Direction direction) noexcept;
    void UpdateFirefly(int index, Direction direction) noexcept;
    void UpdateButterfly(int index, Direction direction) noexcept;
    void UpdateOrange(int index, Direction direction) noexcept;
    void UpdateMagicWall(int index) noexcept;
    void UpdateBlob(int index) noexcept;
    void UpdateExplosions(int index) noexcept;
    void OpenGate(const Element &element) noexcept;

    void StartScan() noexcept;
    void EndScan() noexcept;

    void parse_board_str(const std::string &board_str);

    int magic_wall_steps;                  // params: Number of steps the magic wall stays active for
    int blob_max_size;                     // Max blob size in terms of grid spaces
    int butterfly_explosion_ver;           // params:
    int butterfly_move_ver;                // params:
    int gems_collected = 0;                // Number of gems collected
    int magic_wall_steps_remaining = 0;    // Number of steps remaining for the magic wall
    int blob_size = 0;                     // Current size of the blob
    int rows = -1;
    int cols = -1;
    int agent_idx = -1;
    int gems_required = 0;
    uint64_t random_state;         // State of Xorshift rng
    uint64_t reward_signal = 0;    // Signal for external information about events
    uint64_t hash = 0;
    uint8_t blob_chance{};              // Chance (out of 256) for blob to spawn
    bool gravity{};                     // Flag if gravity is on, affects stones/gems
    bool disable_explosions = false;    // Flag if explosions are disabled, affects bombs
    bool magic_active = false;          // Flag if magic wall is currently active
    bool blob_enclosed = true;          // Flag if blob is enclosed
    bool is_agent_alive = false;
    bool is_agent_in_exit = false;
    HiddenCellType blob_swap = HiddenCellType::kNull;

    // Board
    std::vector<HiddenCellType> grid;
    std::vector<bool> has_updated;
};

}    // namespace boulderdash

#endif    // BOULDERDASH_BASE_H_
