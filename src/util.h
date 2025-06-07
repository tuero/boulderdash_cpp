#ifndef BOULDERDASH_UTIL_H_
#define BOULDERDASH_UTIL_H_

#include <array>
#include <cstddef>
#include <cstdint>
#include <string>
#include <type_traits>
#include <unordered_map>
#include <utility>

#include "definitions.h"

namespace boulderdash {

const std::unordered_map<HiddenCellType, uint8_t> kPointMap = {
    {HiddenCellType::kDiamond, 1},
    {HiddenCellType::kDiamondFalling, 1},
    {HiddenCellType::kAgentInExit, 10},
};

const std::unordered_map<HiddenCellType, RewardCodes> kElementToRewardMap = {
    {HiddenCellType::kDiamond, kRewardCollectDiamond},
    {HiddenCellType::kDiamondFalling, kRewardCollectDiamond},
    {HiddenCellType::kNut, kRewardNutToDiamond},
    {HiddenCellType::kNutFalling, kRewardNutToDiamond},
    {HiddenCellType::kExitOpen, kRewardWalkThroughExit},
    {HiddenCellType::kKeyRed, kRewardCollectKeyRed},
    {HiddenCellType::kKeyBlue, kRewardCollectKeyBlue},
    {HiddenCellType::kKeyGreen, kRewardCollectKeyGreen},
    {HiddenCellType::kKeyYellow, kRewardCollectKeyYellow},
    {HiddenCellType::kGateRedOpen, kRewardWalkThroughGateRed},
    {HiddenCellType::kGateBlueOpen, kRewardWalkThroughGateBlue},
    {HiddenCellType::kGateGreenOpen, kRewardWalkThroughGateGreen},
    {HiddenCellType::kGateYellowOpen, kRewardWalkThroughGateYellow},
};

// Property bit flags
enum ElementProperties {
    kNone = 0,
    kConsumable = 1 << 0,
    kCanExplode = 1 << 1,
    kRounded = 1 << 2,
    kTraversable = 1 << 3,
    kPushable = 1 << 4,
};

// Default base element
const Element kNullElement = {
    HiddenCellType::kNull,
    VisibleCellType::kNull,
    -1,
    0,
};

// All possible elements
const Element kElAgent{
    HiddenCellType::kAgent,
    VisibleCellType::kAgent,
    ElementProperties::kConsumable | ElementProperties::kCanExplode,
    '@',
};
const Element kElAgentInExit = {
    HiddenCellType::kAgentInExit,
    VisibleCellType::kAgentInExit,
    ElementProperties::kNone,
    '!',
};
const Element kElExitOpen = {
    HiddenCellType::kExitOpen,
    VisibleCellType::kExitOpen,
    ElementProperties::kTraversable,
    '#',
};
const Element kElExitClosed = {
    HiddenCellType::kExitClosed,
    VisibleCellType::kExitClosed,
    ElementProperties::kNone,
    'C',
};
const Element kElEmpty = {
    HiddenCellType::kEmpty,
    VisibleCellType::kEmpty,
    ElementProperties::kConsumable | ElementProperties::kTraversable,
    ' ',
};
const Element kElDirt = {
    HiddenCellType::kDirt,
    VisibleCellType::kDirt,
    ElementProperties::kConsumable | ElementProperties::kTraversable,
    '.',
};
const Element kElStone = {
    HiddenCellType::kStone,
    VisibleCellType::kStone,
    ElementProperties::kConsumable | ElementProperties::kRounded | ElementProperties::kPushable,
    'o',
};
const Element kElStoneFalling = {
    HiddenCellType::kStoneFalling,
    VisibleCellType::kStone,
    ElementProperties::kConsumable,
    'o',
};
const Element kElDiamond = {
    HiddenCellType::kDiamond,
    VisibleCellType::kDiamond,
    ElementProperties::kConsumable | ElementProperties::kRounded | ElementProperties::kTraversable,
    '*',
};
const Element kElDiamondFalling = {
    HiddenCellType::kDiamondFalling,
    VisibleCellType::kDiamond,
    ElementProperties::kConsumable,
    '*',
};
const Element kElFireflyUp = {
    HiddenCellType::kFireflyUp,
    VisibleCellType::kFirefly,
    ElementProperties::kConsumable | ElementProperties::kCanExplode,
    'F',
};
const Element kElFireflyLeft = {
    HiddenCellType::kFireflyLeft,
    VisibleCellType::kFirefly,
    ElementProperties::kConsumable | ElementProperties::kCanExplode,
    'F',
};
const Element kElFireflyDown = {
    HiddenCellType::kFireflyDown,
    VisibleCellType::kFirefly,
    ElementProperties::kConsumable | ElementProperties::kCanExplode,
    'F',
};
const Element kElFireflyRight = {
    HiddenCellType::kFireflyRight,
    VisibleCellType::kFirefly,
    ElementProperties::kConsumable | ElementProperties::kCanExplode,
    'F',
};
const Element kElButterflyUp = {
    HiddenCellType::kButterflyUp,
    VisibleCellType::kButterfly,
    ElementProperties::kConsumable | ElementProperties::kCanExplode,
    'U',
};
const Element kElButterflyLeft = {
    HiddenCellType::kButterflyLeft,
    VisibleCellType::kButterfly,
    ElementProperties::kConsumable | ElementProperties::kCanExplode,
    'U',
};
const Element kElButterflyDown = {
    HiddenCellType::kButterflyDown,
    VisibleCellType::kButterfly,
    ElementProperties::kConsumable | ElementProperties::kCanExplode,
    'U',
};
const Element kElButterflyRight = {
    HiddenCellType::kButterflyRight,
    VisibleCellType::kButterfly,
    ElementProperties::kConsumable | ElementProperties::kCanExplode,
    'U',
};
const Element kElBlob = {
    HiddenCellType::kBlob,
    VisibleCellType::kBlob,
    ElementProperties::kConsumable,
    'A',
};
const Element kElWallBrick = {
    HiddenCellType::kWallBrick,
    VisibleCellType::kWallBrick,
    ElementProperties::kConsumable | ElementProperties::kRounded,
    'H',
};
const Element kElWallSteel = {
    HiddenCellType::kWallSteel,
    VisibleCellType::kWallSteel,
    ElementProperties::kNone,
    'S',
};
const Element kElWallMagicOn = {
    HiddenCellType::kWallMagicOn,
    VisibleCellType::kWallMagicOn,
    ElementProperties::kConsumable,
    'M',
};
const Element kElWallMagicDormant = {
    HiddenCellType::kWallMagicDormant,
    VisibleCellType::kWallMagicOff,
    ElementProperties::kConsumable,
    'Q',
};
const Element kElWallMagicExpired = {
    HiddenCellType::kWallMagicExpired,
    VisibleCellType::kWallMagicOff,
    ElementProperties::kConsumable,
    'Q',
};
const Element kElExplosionDiamond = {
    HiddenCellType::kExplosionDiamond,
    VisibleCellType::kExplosion,
    ElementProperties::kNone,
    'E',
};
const Element kElExplosionBoulder = {
    HiddenCellType::kExplosionBoulder,
    VisibleCellType::kExplosion,
    ElementProperties::kNone,
    'E',
};
const Element kElExplosionEmpty = {
    HiddenCellType::kExplosionEmpty,
    VisibleCellType::kExplosion,
    ElementProperties::kNone,
    'E',
};
const Element kElGateRedClosed = {
    HiddenCellType::kGateRedClosed,
    VisibleCellType::kGateRedClosed,
    ElementProperties::kNone,
    'r',
};
const Element kElGateRedOpen = {
    HiddenCellType::kGateRedOpen,
    VisibleCellType::kGateRedOpen,
    ElementProperties::kNone,
    'R',
};
const Element kElKeyRed = {
    HiddenCellType::kKeyRed,
    VisibleCellType::kKeyRed,
    ElementProperties::kTraversable,
    '1',
};
const Element kElGateBlueClosed = {
    HiddenCellType::kGateBlueClosed,
    VisibleCellType::kGateBlueClosed,
    ElementProperties::kNone,
    'b',
};
const Element kElGateBlueOpen = {
    HiddenCellType::kGateBlueOpen,
    VisibleCellType::kGateBlueOpen,
    ElementProperties::kNone,
    'B',
};
const Element kElKeyBlue = {
    HiddenCellType::kKeyBlue,
    VisibleCellType::kKeyBlue,
    ElementProperties::kTraversable,
    '2',
};
const Element kElGateGreenClosed = {
    HiddenCellType::kGateGreenClosed,
    VisibleCellType::kGateGreenClosed,
    ElementProperties::kNone,
    'g',
};
const Element kElGateGreenOpen = {
    HiddenCellType::kGateGreenOpen,
    VisibleCellType::kGateGreenOpen,
    ElementProperties::kNone,
    'G',
};
const Element kElKeyGreen = {
    HiddenCellType::kKeyGreen,
    VisibleCellType::kKeyGreen,
    ElementProperties::kTraversable,
    '3',
};
const Element kElGateYellowClosed = {
    HiddenCellType::kGateYellowClosed,
    VisibleCellType::kGateYellowClosed,
    ElementProperties::kNone,
    'y',
};
const Element kElGateYellowOpen = {
    HiddenCellType::kGateYellowOpen,
    VisibleCellType::kGateYellowOpen,
    ElementProperties::kNone,
    'Y',
};
const Element kElKeyYellow = {
    HiddenCellType::kKeyYellow,
    VisibleCellType::kKeyYellow,
    ElementProperties::kTraversable,
    '4',
};
const Element kElNut = {
    HiddenCellType::kNut,
    VisibleCellType::kNut,
    ElementProperties::kRounded | ElementProperties::kConsumable | ElementProperties::kPushable,
    '+',
};
const Element kElNutFalling = {
    HiddenCellType::kNutFalling,
    VisibleCellType::kNut,
    ElementProperties::kRounded | ElementProperties::kConsumable,
    '+',
};
const Element kElBomb = {
    HiddenCellType::kBomb,
    VisibleCellType::kBomb,
    ElementProperties::kRounded | ElementProperties::kConsumable | ElementProperties::kCanExplode |
        ElementProperties::kPushable,
    '^',
};
const Element kElBombFalling = {
    HiddenCellType::kBombFalling,
    VisibleCellType::kBomb,
    ElementProperties::kRounded | ElementProperties::kConsumable | ElementProperties::kCanExplode,
    '^',
};
const Element kElOrangeUp = {
    HiddenCellType::kOrangeUp,
    VisibleCellType::kOrange,
    ElementProperties::kConsumable | ElementProperties::kCanExplode,
    'X',
};
const Element kElOrangeLeft = {
    HiddenCellType::kOrangeLeft,
    VisibleCellType::kOrange,
    ElementProperties::kConsumable | ElementProperties::kCanExplode,
    'X',
};
const Element kElOrangeDown = {
    HiddenCellType::kOrangeDown,
    VisibleCellType::kOrange,
    ElementProperties::kConsumable | ElementProperties::kCanExplode,
    'X',
};
const Element kElOrangeRight = {
    HiddenCellType::kOrangeRight,
    VisibleCellType::kOrange,
    ElementProperties::kConsumable | ElementProperties::kCanExplode,
    'X',
};

// Hash for Element, so we can use as a map key
struct ElementHash {
    auto operator()(const Element &e) const noexcept -> std::size_t {
        return static_cast<std::size_t>(to_underlying(e.cell_type) - to_underlying(HiddenCellType::kNull));
    }
};

// ----- Conversion maps -----
// Swap map for from cell type id to element
const std::array<Element, kNumHiddenCellType + 1> kCellTypeToElement{
    kNullElement,           // HiddenCellType::kNull
    kElAgent,               // HiddenCellType::kAgent
    kElEmpty,               // HiddenCellType::kEmpty
    kElDirt,                // HiddenCellType::kDirt
    kElStone,               // HiddenCellType::kStone
    kElStoneFalling,        // HiddenCellType::kStoneFalling
    kElDiamond,             // HiddenCellType::kDiamond
    kElDiamondFalling,      // HiddenCellType::kDiamondFalling
    kElExitClosed,          // HiddenCellType::kExitClosed
    kElExitOpen,            // HiddenCellType::kExitOpen
    kElAgentInExit,         // HiddenCellType::kAgentInExit
    kElFireflyUp,           // HiddenCellType::kAgentInExit
    kElFireflyLeft,         // HiddenCellType::kFireflyLeft
    kElFireflyDown,         // HiddenCellType::kFireflyDown
    kElFireflyRight,        // HiddenCellType::kFireflyRight
    kElButterflyUp,         // HiddenCellType::kButterflyUp
    kElButterflyLeft,       // HiddenCellType::kButterflyLeft
    kElButterflyDown,       // HiddenCellType::kButterflyDown
    kElButterflyRight,      // HiddenCellType::kButterflyRight
    kElWallBrick,           // HiddenCellType::kWallBrick
    kElWallSteel,           // HiddenCellType::kWallSteel
    kElWallMagicOn,         // HiddenCellType::kWallMagicOn
    kElWallMagicDormant,    // HiddenCellType::kWallMagicDormant
    kElWallMagicExpired,    // HiddenCellType::kWallMagicExpired
    kElBlob,                // HiddenCellType::kBlob
    kElExplosionDiamond,    // HiddenCellType::kExplosionDiamond
    kElExplosionBoulder,    // HiddenCellType::kExplosionBoulder
    kElExplosionEmpty,      // HiddenCellType::kExplosionEmpty
    kElGateRedClosed,       // HiddenCellType::kGateRedClosed
    kElGateRedOpen,         // HiddenCellType::kGateRedOpen
    kElKeyRed,              // HiddenCellType::kKeyRed
    kElGateBlueClosed,      // HiddenCellType::kGateBlueClosed
    kElGateBlueOpen,        // HiddenCellType::kGateBlueOpen
    kElKeyBlue,             // HiddenCellType::kKeyBlue
    kElGateGreenClosed,     // HiddenCellType::kGateGreenClosed
    kElGateGreenOpen,       // HiddenCellType::kGateGreenOpen
    kElKeyGreen,            // HiddenCellType::kKeyGreen
    kElGateYellowClosed,    // HiddenCellType::kGateYellowClosed
    kElGateYellowOpen,      // HiddenCellType::kGateYellowOpen
    kElKeyYellow,           // HiddenCellType::kKeyYellow
    kElNut,                 // HiddenCellType::kNut
    kElNutFalling,          // HiddenCellType::kNutFalling
    kElBomb,                // HiddenCellType::kBomb
    kElBombFalling,         // HiddenCellType::kBombFalling
    kElOrangeUp,            // HiddenCellType::kOrangeUp
    kElOrangeLeft,          // HiddenCellType::kOrangeLeft
    kElOrangeDown,          // HiddenCellType::kOrangeDown
    kElOrangeRight,         // HiddenCellType::kOrangeRight
    kNullElement,           // HiddenCellType::kPebbleInDirt
    kNullElement,           // HiddenCellType::kStoneInDirt
    kNullElement,           // HiddenCellType::kVoidInDirt
};

// Swap map for from cell type id to string for debugging
const std::unordered_map<int8_t, std::string> kCellTypeToString{
    {static_cast<int8_t>(HiddenCellType::kNull), "NullElement"},
    {static_cast<int8_t>(HiddenCellType::kAgent), "Agent"},
    {static_cast<int8_t>(HiddenCellType::kEmpty), "Empty"},
    {static_cast<int8_t>(HiddenCellType::kDirt), "Dirt"},
    {static_cast<int8_t>(HiddenCellType::kStone), "Stone"},
    {static_cast<int8_t>(HiddenCellType::kStoneFalling), "StoneFalling"},
    {static_cast<int8_t>(HiddenCellType::kDiamond), "Diamond"},
    {static_cast<int8_t>(HiddenCellType::kDiamondFalling), "DiamondFalling"},
    {static_cast<int8_t>(HiddenCellType::kExitClosed), "ExitClosed"},
    {static_cast<int8_t>(HiddenCellType::kExitOpen), "ExitOpen"},
    {static_cast<int8_t>(HiddenCellType::kAgentInExit), "AgentInExit"},
    {static_cast<int8_t>(HiddenCellType::kFireflyUp), "FireflyUp"},
    {static_cast<int8_t>(HiddenCellType::kFireflyLeft), "FireflyLeft"},
    {static_cast<int8_t>(HiddenCellType::kFireflyDown), "FireflyDown"},
    {static_cast<int8_t>(HiddenCellType::kFireflyRight), "FireflyRight"},
    {static_cast<int8_t>(HiddenCellType::kButterflyUp), "ButterflyUp"},
    {static_cast<int8_t>(HiddenCellType::kButterflyLeft), "ButterflyLeft"},
    {static_cast<int8_t>(HiddenCellType::kButterflyDown), "ButterflyDown"},
    {static_cast<int8_t>(HiddenCellType::kButterflyRight), "ButterflyRight"},
    {static_cast<int8_t>(HiddenCellType::kWallBrick), "WallBrick"},
    {static_cast<int8_t>(HiddenCellType::kWallSteel), "WallSteel"},
    {static_cast<int8_t>(HiddenCellType::kWallMagicOn), "WallMagicOn"},
    {static_cast<int8_t>(HiddenCellType::kWallMagicDormant), "WallMagicDormant"},
    {static_cast<int8_t>(HiddenCellType::kWallMagicExpired), "WallMagicExpired"},
    {static_cast<int8_t>(HiddenCellType::kBlob), "Blob"},
    {static_cast<int8_t>(HiddenCellType::kExplosionBoulder), "ExplosionBoulder"},
    {static_cast<int8_t>(HiddenCellType::kExplosionDiamond), "ExplosionDiamond"},
    {static_cast<int8_t>(HiddenCellType::kExplosionEmpty), "ExplosionEmpty"},
    {static_cast<int8_t>(HiddenCellType::kGateRedClosed), "GateRedClosed"},
    {static_cast<int8_t>(HiddenCellType::kGateRedOpen), "GateRedOpen"},
    {static_cast<int8_t>(HiddenCellType::kKeyRed), "KeyRed"},
    {static_cast<int8_t>(HiddenCellType::kGateBlueClosed), "GateBlueClosed"},
    {static_cast<int8_t>(HiddenCellType::kGateBlueOpen), "GateBlueOpen"},
    {static_cast<int8_t>(HiddenCellType::kKeyBlue), "KeyBlue"},
    {static_cast<int8_t>(HiddenCellType::kGateGreenClosed), "GateGreenClosed"},
    {static_cast<int8_t>(HiddenCellType::kGateGreenOpen), "GateGreenOpen"},
    {static_cast<int8_t>(HiddenCellType::kKeyGreen), "KeyGreen"},
    {static_cast<int8_t>(HiddenCellType::kGateYellowClosed), "GateYellowClosed"},
    {static_cast<int8_t>(HiddenCellType::kGateYellowOpen), "GateYellowOpen"},
    {static_cast<int8_t>(HiddenCellType::kKeyYellow), "KeyYellow"},
    {static_cast<int8_t>(HiddenCellType::kNut), "Nut"},
    {static_cast<int8_t>(HiddenCellType::kNutFalling), "NutFalling"},
    {static_cast<int8_t>(HiddenCellType::kBomb), "Bomb"},
    {static_cast<int8_t>(HiddenCellType::kBombFalling), "BombFalling"},
    {static_cast<int8_t>(HiddenCellType::kOrangeUp), "OrangeUp"},
    {static_cast<int8_t>(HiddenCellType::kOrangeLeft), "OrangeLeft"},
    {static_cast<int8_t>(HiddenCellType::kOrangeDown), "OrangeDown"},
    {static_cast<int8_t>(HiddenCellType::kOrangeRight), "OrangeRight"},
};

// Rotate actions right
const std::array<Direction, kNumActions> kRotateRight{Direction::kRight, Direction::kDown, Direction::kLeft,
                                                      Direction::kUp};

// Rotate actions left
const std::array<Direction, kNumActions> kRotateLeft{Direction::kLeft, Direction::kUp, Direction::kRight,
                                                     Direction::kDown};

// actions to strings
const std::unordered_map<Direction, std::string> kActionsToString{
    {Direction::kUp, "up"},
    {Direction::kLeft, "left"},
    {Direction::kDown, "down"},
    {Direction::kRight, "right"},
};

// typedefs
using Offset = std::pair<int, int>;

// directions to offsets (col, row)
const std::array<Offset, kNumDirections> kDirectionOffsets{{
    {0, -1},    // Direction::kUp
    {1, 0},     // Direction::kRight
    {0, 1},     // Direction::kDown
    {-1, 0},    // Direction::kLeft
    {0, 0},     // Direction::kNoop
    {1, -1},    // Direction::kUpRight
    {1, 1},     // Direction::kDownRight
    {-1, 1},    // Direction::kDownLeft
    {-1, -1}    // Direction::kUpLeft
}};

// Directions to fireflys
const std::array<Element, kNumActions> kDirectionToFirefly{
    kElFireflyUp,       // Direction::kUp
    kElFireflyRight,    // Direction::kRight
    kElFireflyDown,     // Direction::kDown
    kElFireflyLeft,     // Direction::kLeft
};

// Firefly to directions
const std::unordered_map<Element, Direction, ElementHash> kFireflyToDirection{
    {kElFireflyUp, Direction::kUp},
    {kElFireflyLeft, Direction::kLeft},
    {kElFireflyDown, Direction::kDown},
    {kElFireflyRight, Direction::kRight},
};

// Directions to butterflys
const std::array<Element, kNumActions> kDirectionToButterfly{
    kElButterflyUp,       // Direction::kUp
    kElButterflyRight,    // Direction::kRight
    kElButterflyDown,     // Direction::kDown
    kElButterflyLeft,     // Direction::kLeft
};

// Butterfly to directions
const std::unordered_map<Element, Direction, ElementHash> kButterflyToDirection{
    {kElButterflyUp, Direction::kUp},
    {kElButterflyLeft, Direction::kLeft},
    {kElButterflyDown, Direction::kDown},
    {kElButterflyRight, Direction::kRight},
};

// Orange to directions
const std::unordered_map<Element, Direction, ElementHash> kOrangeToDirection{
    {kElOrangeUp, Direction::kUp},
    {kElOrangeLeft, Direction::kLeft},
    {kElOrangeDown, Direction::kDown},
    {kElOrangeRight, Direction::kRight},
};

// Direction to Orange
const std::array<Element, kNumActions> kDirectionToOrange{
    kElOrangeUp,       // Direction::kUp
    kElOrangeRight,    // Direction::kRight
    kElOrangeDown,     // Direction::kDown
    kElOrangeLeft,     // Direction::kLeft
};

// Element explosion maps
const std::unordered_map<Element, Element, ElementHash> kElementToExplosion{
    {kElFireflyUp, kElExplosionEmpty},       {kElFireflyLeft, kElExplosionEmpty},
    {kElFireflyDown, kElExplosionEmpty},     {kElFireflyRight, kElExplosionEmpty},
    {kElButterflyUp, kElExplosionDiamond},   {kElButterflyLeft, kElExplosionDiamond},
    {kElButterflyDown, kElExplosionDiamond}, {kElButterflyRight, kElExplosionDiamond},
    {kElAgent, kElExplosionEmpty},           {kElBomb, kElExplosionEmpty},
    {kElBombFalling, kElExplosionEmpty},     {kElOrangeUp, kElExplosionEmpty},
    {kElOrangeLeft, kElExplosionEmpty},      {kElOrangeDown, kElExplosionEmpty},
    {kElOrangeRight, kElExplosionEmpty},
};

// Explosions back to elements
const std::unordered_map<Element, Element, ElementHash> kExplosionToElement{
    {kElExplosionDiamond, kElDiamond},
    {kElExplosionBoulder, kElStone},
    {kElExplosionEmpty, kElEmpty},
};
const std::unordered_map<Element, RewardCodes, ElementHash> kExplosionToReward{
    {kElExplosionDiamond, kRewardButterflyToDiamond},
    {kElExplosionBoulder, kRewardNone},
    {kElExplosionEmpty, kRewardNone},
};

// Magic wall conversion map
const std::unordered_map<Element, Element, ElementHash> kMagicWallConversion{
    {kElStoneFalling, kElDiamondFalling},
    {kElDiamondFalling, kElStoneFalling},
};

// Gem point maps
const std::unordered_map<Element, int, ElementHash> kGemPoints{
    {kElDiamond, 10},
    {kElDiamondFalling, 10},
    {kElAgentInExit, 100},
};

// Gate open conversion map
const std::unordered_map<Element, Element, ElementHash> kGateOpenMap{
    {kElGateRedClosed, kElGateRedOpen},
    {kElGateBlueClosed, kElGateBlueOpen},
    {kElGateGreenClosed, kElGateGreenOpen},
    {kElGateYellowClosed, kElGateYellowOpen},
};
// Gate key map
const std::unordered_map<Element, Element, ElementHash> kKeyToGate{
    {kElKeyRed, kElGateRedClosed},
    {kElKeyBlue, kElGateBlueClosed},
    {kElKeyGreen, kElGateGreenClosed},
    {kElKeyYellow, kElGateYellowClosed},
};
// Key signal map
const std::unordered_map<Element, RewardCodes, ElementHash> kKeyToSignal{
    {kElKeyRed, RewardCodes::kRewardCollectKeyRed},
    {kElKeyBlue, RewardCodes::kRewardCollectKeyBlue},
    {kElKeyGreen, RewardCodes::kRewardCollectKeyGreen},
    {kElKeyYellow, RewardCodes::kRewardCollectKeyYellow},
};
// Gate signal map
const std::unordered_map<Element, RewardCodes, ElementHash> kGateToSignal{
    {kElGateRedOpen, RewardCodes::kRewardWalkThroughGateRed},
    {kElGateBlueOpen, RewardCodes::kRewardWalkThroughGateBlue},
    {kElGateGreenOpen, RewardCodes::kRewardWalkThroughGateGreen},
    {kElGateYellowOpen, RewardCodes::kRewardWalkThroughGateYellow},
};

// Stationary to falling
const std::unordered_map<Element, Element, ElementHash> kElToFalling{
    {kElDiamond, kElDiamondFalling},
    {kElStone, kElStoneFalling},
    {kElNut, kElNutFalling},
    {kElBomb, kElBombFalling},
};

// Helper functions
inline auto IsDirectionHorz(Direction direction) noexcept -> bool {
    return direction == Direction::kLeft || direction == Direction::kRight;
}

inline auto IsFirefly(const Element &element) noexcept -> bool {
    return element == kElFireflyUp || element == kElFireflyLeft || element == kElFireflyDown ||
           element == kElFireflyRight;
}

inline auto IsButterfly(const Element &element) noexcept -> bool {
    return element == kElButterflyUp || element == kElButterflyLeft || element == kElButterflyDown ||
           element == kElButterflyRight;
}

inline auto IsOrange(const Element &element) noexcept -> bool {
    return element == kElOrangeUp || element == kElOrangeLeft || element == kElOrangeDown || element == kElOrangeRight;
}

inline auto IsExplosion(const Element &element) noexcept -> bool {
    return element == kElExplosionBoulder || element == kElExplosionDiamond || element == kElExplosionEmpty;
}

inline auto IsMagicWall(const Element &element) noexcept -> bool {
    return element == kElWallMagicDormant || element == kElWallMagicExpired || element == kElWallMagicOn;
}

inline auto IsOpenGate(const Element &element) noexcept -> bool {
    return element == kElGateRedOpen || element == kElGateBlueOpen || element == kElGateGreenOpen ||
           element == kElGateYellowOpen;
}

inline auto IsKey(const Element &element) noexcept -> bool {
    return element == kElKeyRed || element == kElKeyBlue || element == kElKeyGreen || element == kElKeyYellow;
}

inline auto ElementToItem(const Element &element) noexcept -> std::underlying_type_t<HiddenCellType> {
    return to_underlying(element.cell_type);
}

}    // namespace boulderdash

#endif    // BOULDERDASH_UTIL_H_
