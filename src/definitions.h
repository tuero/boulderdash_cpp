#ifndef BOULDERDASH_DEFS_H_
#define BOULDERDASH_DEFS_H_

#include <array>
#include <cassert>
#include <cstdint>
#include <type_traits>

namespace boulderdash {

template <class E>
constexpr inline auto to_underlying(E e) noexcept -> std::underlying_type_t<E> {
    return static_cast<std::underlying_type_t<E>>(e);
}

enum class HiddenCellType : int8_t {
    kNull = -1,
    kAgent = 0,
    kEmpty = 1,
    kDirt = 2,
    kStone = 3,
    kStoneFalling = 4,
    kDiamond = 5,
    kDiamondFalling = 6,
    kExitClosed = 7,
    kExitOpen = 8,
    kAgentInExit = 9,
    kFireflyUp = 10,
    kFireflyLeft = 11,
    kFireflyDown = 12,
    kFireflyRight = 13,
    kButterflyUp = 14,
    kButterflyLeft = 15,
    kButterflyDown = 16,
    kButterflyRight = 17,
    kWallBrick = 18,
    kWallSteel = 19,
    kWallMagicDormant = 20,
    kWallMagicOn = 21,
    kWallMagicExpired = 22,
    kBlob = 23,
    kExplosionDiamond = 24,
    kExplosionBoulder = 25,
    kExplosionEmpty = 26,
    kGateRedClosed = 27,
    kGateRedOpen = 28,
    kKeyRed = 29,
    kGateBlueClosed = 30,
    kGateBlueOpen = 31,
    kKeyBlue = 32,
    kGateGreenClosed = 33,
    kGateGreenOpen = 34,
    kKeyGreen = 35,
    kGateYellowClosed = 36,
    kGateYellowOpen = 37,
    kKeyYellow = 38,
    kNut = 39,
    kNutFalling = 40,
    kBomb = 41,
    kBombFalling = 42,
    kOrangeUp = 43,
    kOrangeLeft = 44,
    kOrangeDown = 45,
    kOrangeRight = 46,
    kPebbleInDirt = 47,
    kStoneInDirt = 48,
    kVoidInDirt = 49,
};
constexpr int kNumHiddenCellType = 50;

// Cell types which are observable
enum class VisibleCellType : int8_t {
    kNull = -1,
    kAgent = 0,
    kEmpty = 1,
    kDirt = 2,
    kStone = 3,
    kDiamond = 4,
    kExitClosed = 5,
    kExitOpen = 6,
    kAgentInExit = 7,
    kFirefly = 8,
    kButterfly = 9,
    kWallBrick = 10,
    kWallSteel = 11,
    kWallMagicOff = 12,
    kWallMagicOn = 13,
    kBlob = 14,
    kExplosion = 15,
    kGateRedClosed = 16,
    kGateRedOpen = 17,
    kKeyRed = 18,
    kGateBlueClosed = 19,
    kGateBlueOpen = 20,
    kKeyBlue = 21,
    kGateGreenClosed = 22,
    kGateGreenOpen = 23,
    kKeyGreen = 24,
    kGateYellowClosed = 25,
    kGateYellowOpen = 26,
    kKeyYellow = 27,
    kNut = 28,
    kBomb = 29,
    kOrange = 30,
    kPebbleInDirt = 31,
    kStoneInDirt = 32,
    kVoidInDirt = 33,
};
constexpr int kNumVisibleCellType = 34;

// Actions the agent can take
enum class Action {
    kUp = 0,
    kRight = 1,
    kDown = 2,
    kLeft = 3,
};
constexpr int kNumActions = 4;
const std::array<Action, kNumActions> ALL_ACTIONS{
    Action::kUp,
    Action::kRight,
    Action::kDown,
    Action::kLeft,
};

// Directions the interactions take place
enum class Direction {
    kUp = to_underlying(Action::kUp),
    kRight = to_underlying(Action::kRight),
    kDown = to_underlying(Action::kDown),
    kLeft = to_underlying(Action::kLeft),
    kNoop = to_underlying(Action::kLeft) + 1,
    kUpRight = to_underlying(Action::kLeft) + 2,
    kDownRight = to_underlying(Action::kLeft) + 3,
    kDownLeft = to_underlying(Action::kLeft) + 4,
    kUpLeft = to_underlying(Action::kLeft) + 5,
};
// Agent can only take a subset of all directions
constexpr int kNumDirections = 9;

[[nodiscard]] constexpr inline auto action_to_direction(Action action) noexcept -> Direction {
    return static_cast<Direction>(to_underlying(action));
}

enum RewardCodes : uint64_t {
    kRewardNone = 0,
    kRewardAgentDies = 1 << 0,
    kRewardCollectDiamond = 1 << 1,
    kRewardWalkThroughExit = 1 << 2,
    kRewardNutToDiamond = 1 << 3,
    kRewardButterflyToDiamond = 1 << 4,
    kRewardCollectKey = 1 << 5,
    kRewardCollectKeyRed = 1 << 6,
    kRewardCollectKeyBlue = 1 << 7,
    kRewardCollectKeyGreen = 1 << 8,
    kRewardCollectKeyYellow = 1 << 9,
    kRewardWalkThroughGate = 1 << 10,
    kRewardWalkThroughGateRed = 1 << 11,
    kRewardWalkThroughGateBlue = 1 << 12,
    kRewardWalkThroughGateGreen = 1 << 13,
    kRewardWalkThroughGateYellow = 1 << 14,
};

enum ButterflyExplosionVersion : int {
    kExplode = 1,    // Explode when being hit by stone
    kConvert = 2,    // Convert to diamong when being hit by stone
};
enum ButterflyMoveVersion : int {
    kDelay = 1,      // Delay a game tick between transitioning directions
    kInstant = 2,    // Move instantly after changing directions
};

// Element entities, along with properties
struct Element {
    // NOLINTBEGIN(misc-non-private-member-variables-in-classes)
    HiddenCellType cell_type;
    VisibleCellType visible_type;
    int properties;
    char id;
    bool has_updated;
    // NOLINTEND(misc-non-private-member-variables-in-classes)

    Element()
        : cell_type(HiddenCellType::kNull),
          visible_type(VisibleCellType::kNull),
          properties(0),
          id(0),
          has_updated(false) {}

    Element(HiddenCellType cell_type_, VisibleCellType visible_type_, int properties_, char id_)
        : cell_type(cell_type_), visible_type(visible_type_), properties(properties_), id(id_), has_updated(false) {}

    auto operator==(const Element &rhs) const -> bool {
        return this->cell_type == rhs.cell_type;
    }

    auto operator!=(const Element &rhs) const -> bool {
        return this->cell_type != rhs.cell_type;
    }
};

}    // namespace boulderdash

#endif    // BOULDERDASH_DEFS_H_
