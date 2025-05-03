// pyboulderdash.cpp
// Python bindings

#include <pybind11/numpy.h>
#include <pybind11/operators.h>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include <cstdint>
#include <sstream>
#include <vector>

#include "boulderdash/boulderdash.h"

namespace py = pybind11;

PYBIND11_MODULE(pyboulderdash, m) {
    m.doc() = "BoulderDash environment module docs.";
    using T = boulderdash::BoulderDashGameState;
    py::enum_<boulderdash::HiddenCellType>(m, "HiddenCellType")
        .value("kAgent", boulderdash::HiddenCellType::kAgent)
        .value("kEmpty", boulderdash::HiddenCellType::kEmpty)
        .value("kDirt", boulderdash::HiddenCellType::kDirt)
        .value("kStone", boulderdash::HiddenCellType::kStone)
        .value("kStoneFalling", boulderdash::HiddenCellType::kStoneFalling)
        .value("kDiamond", boulderdash::HiddenCellType::kDiamond)
        .value("kDiamondFalling", boulderdash::HiddenCellType::kDiamondFalling)
        .value("kExitClosed", boulderdash::HiddenCellType::kExitClosed)
        .value("kExitOpen", boulderdash::HiddenCellType::kExitOpen)
        .value("kAgentInExit", boulderdash::HiddenCellType::kAgentInExit)
        .value("kFireflyUp", boulderdash::HiddenCellType::kFireflyUp)
        .value("kFireflyLeft", boulderdash::HiddenCellType::kFireflyLeft)
        .value("kFireflyDown", boulderdash::HiddenCellType::kFireflyDown)
        .value("kFireflyRight", boulderdash::HiddenCellType::kFireflyRight)
        .value("kButterflyUp", boulderdash::HiddenCellType::kButterflyUp)
        .value("kButterflyLeft", boulderdash::HiddenCellType::kButterflyLeft)
        .value("kButterflyDown", boulderdash::HiddenCellType::kButterflyDown)
        .value("kButterflyRight", boulderdash::HiddenCellType::kButterflyRight)
        .value("kWallBrick", boulderdash::HiddenCellType::kWallBrick)
        .value("kWallSteel", boulderdash::HiddenCellType::kWallSteel)
        .value("kWallMagicDormant", boulderdash::HiddenCellType::kWallMagicDormant)
        .value("kWallMagicOn", boulderdash::HiddenCellType::kWallMagicOn)
        .value("kWallMagicExpired", boulderdash::HiddenCellType::kWallMagicExpired)
        .value("kBlob", boulderdash::HiddenCellType::kBlob)
        .value("kExplosionDiamond", boulderdash::HiddenCellType::kExplosionDiamond)
        .value("kExplosionBoulder", boulderdash::HiddenCellType::kExplosionBoulder)
        .value("kExplosionEmpty", boulderdash::HiddenCellType::kExplosionEmpty)
        .value("kGateRedClosed", boulderdash::HiddenCellType::kGateRedClosed)
        .value("kGateRedOpen", boulderdash::HiddenCellType::kGateRedOpen)
        .value("kKeyRed", boulderdash::HiddenCellType::kKeyRed)
        .value("kGateBlueClosed", boulderdash::HiddenCellType::kGateBlueClosed)
        .value("kGateBlueOpen", boulderdash::HiddenCellType::kGateBlueOpen)
        .value("kKeyBlue", boulderdash::HiddenCellType::kKeyBlue)
        .value("kGateGreenClosed", boulderdash::HiddenCellType::kGateGreenClosed)
        .value("kGateGreenOpen", boulderdash::HiddenCellType::kGateGreenOpen)
        .value("kKeyGreen", boulderdash::HiddenCellType::kKeyGreen)
        .value("kGateYellowClosed", boulderdash::HiddenCellType::kGateYellowClosed)
        .value("kGateYellowOpen", boulderdash::HiddenCellType::kGateYellowOpen)
        .value("kKeyYellow", boulderdash::HiddenCellType::kKeyYellow)
        .value("kNut", boulderdash::HiddenCellType::kNut)
        .value("kNutFalling", boulderdash::HiddenCellType::kNutFalling)
        .value("kBomb", boulderdash::HiddenCellType::kBomb)
        .value("kBombFalling", boulderdash::HiddenCellType::kBombFalling)
        .value("kOrangeUp", boulderdash::HiddenCellType::kOrangeUp)
        .value("kOrangeLeft", boulderdash::HiddenCellType::kOrangeLeft)
        .value("kOrangeDown", boulderdash::HiddenCellType::kOrangeDown)
        .value("kOrangeRight", boulderdash::HiddenCellType::kOrangeRight)
        .value("kPebbleInDirt", boulderdash::HiddenCellType::kPebbleInDirt)
        .value("kStoneInDirt", boulderdash::HiddenCellType::kStoneInDirt)
        .value("kVoidInDirt", boulderdash::HiddenCellType::kVoidInDirt);
    py::enum_<boulderdash::RewardCodes>(m, "RewardCode")
        .value("kRewardAgentDies", boulderdash::RewardCodes::kRewardAgentDies)
        .value("kRewardCollectDiamond", boulderdash::RewardCodes::kRewardCollectDiamond)
        .value("kRewardWalkThroughExit", boulderdash::RewardCodes::kRewardWalkThroughExit)
        .value("kRewardNutToDiamond", boulderdash::RewardCodes::kRewardNutToDiamond)
        .value("kRewardCollectKey", boulderdash::RewardCodes::kRewardCollectKey)
        .value("kRewardCollectKeyRed", boulderdash::RewardCodes::kRewardCollectKeyRed)
        .value("kRewardCollectKeyBlue", boulderdash::RewardCodes::kRewardCollectKeyBlue)
        .value("kRewardCollectKeyGreen", boulderdash::RewardCodes::kRewardCollectKeyGreen)
        .value("kRewardCollectKeyYellow", boulderdash::RewardCodes::kRewardCollectKeyYellow)
        .value("kRewardWalkThroughGate", boulderdash::RewardCodes::kRewardWalkThroughGate)
        .value("kRewardWalkThroughGateRed", boulderdash::RewardCodes::kRewardWalkThroughGateRed)
        .value("kRewardWalkThroughGateBlue", boulderdash::RewardCodes::kRewardWalkThroughGateBlue)
        .value("kRewardWalkThroughGateGreen", boulderdash::RewardCodes::kRewardWalkThroughGateGreen)
        .value("kRewardWalkThroughGateYellow", boulderdash::RewardCodes::kRewardWalkThroughGateYellow);

    using GP = boulderdash::GameParameters;
    py::class_<GP>(m, "GameParameters")
        .def(py::init<>())
        .def("__repr__",
             [](const GP &self) {
                 std::stringstream stream;
                 stream << self;
                 return stream.str();
             })
        .def_readwrite("gravity", &GP::gravity)
        .def_readwrite("magic_wall_steps", &GP::magic_wall_steps)
        .def_readwrite("blob_chance", &GP::blob_chance)
        .def_readwrite("blob_max_percentage", &GP::blob_max_percentage)
        .def_readwrite("disable_explosions", &GP::disable_explosions)
        .def_readwrite("butterfly_explosion_ver", &GP::butterfly_explosion_ver)
        .def_readwrite("butterfly_move_ver", &GP::butterfly_move_ver);

    py::class_<T>(m, "BoulderDashGameState")
        .def(py::init<const std::string &>())
        .def(py::init<const std::string &, const GP &>())
        .def_readonly_static("name", &T::name)
        .def_readonly_static("num_actions", &boulderdash::kNumActions)
        .def(py::self == py::self)    // NOLINT (misc-redundant-expression)
        .def(py::self != py::self)    // NOLINT (misc-redundant-expression)
        .def("__hash__", [](const T &self) { return self.get_hash(); })
        .def("__copy__", [](const T &self) { return T(self); })
        .def("__deepcopy__", [](const T &self, py::dict) { return T(self); })
        .def("__repr__",
             [](const T &self) {
                 std::stringstream stream;
                 stream << self;
                 return stream.str();
             })
        .def(py::pickle(
            [](const T &self) {    // __getstate__
                auto s = self.pack();
                return py::make_tuple(s.magic_wall_steps, s.blob_max_size, s.butterfly_explosion_ver,
                                      s.butterfly_move_ver, s.gems_collected, s.magic_wall_steps_remaining, s.blob_size,
                                      s.rows, s.cols, s.agent_idx, s.gems_required, s.random_state, s.reward_signal,
                                      s.hash, s.blob_chance, s.gravity, s.disable_explosions, s.magic_active,
                                      s.blob_enclosed, s.is_agent_alive, s.is_agent_in_exit, s.blob_swap, s.grid,
                                      s.has_updated);
            },
            [](py::tuple t) -> T {    // __setstate__
                if (t.size() != 24) {
                    throw std::runtime_error("Invalid state");
                }
                T::InternalState s;
                s.magic_wall_steps = t[0].cast<int>();              // NOLINT(*-magic-numbers)
                s.blob_max_size = t[1].cast<int>();                 // NOLINT(*-magic-numbers)
                s.butterfly_explosion_ver = t[2].cast<int>();       // NOLINT(*-magic-numbers)
                s.butterfly_move_ver = t[3].cast<int>();            // NOLINT(*-magic-numbers)
                s.gems_collected = t[4].cast<int>();                // NOLINT(*-magic-numbers)
                s.magic_wall_steps_remaining = t[5].cast<int>();    // NOLINT(*-magic-numbers)
                s.blob_size = t[6].cast<int>();                     // NOLINT(*-magic-numbers)
                s.rows = t[7].cast<int>();                          // NOLINT(*-magic-numbers)
                s.cols = t[8].cast<int>();                          // NOLINT(*-magic-numbers)
                s.agent_idx = t[9].cast<int>();                     // NOLINT(*-magic-numbers)
                s.gems_required = t[10].cast<int>();                // NOLINT(*-magic-numbers)
                s.random_state = t[11].cast<uint64_t>();            // NOLINT(*-magic-numbers)
                s.reward_signal = t[12].cast<uint64_t>();           // NOLINT(*-magic-numbers)
                s.hash = t[13].cast<uint64_t>();                    // NOLINT(*-magic-numbers)
                s.blob_chance = t[14].cast<uint8_t>();              // NOLINT(*-magic-numbers)
                s.gravity = t[15].cast<bool>();                     // NOLINT(*-magic-numbers)
                s.disable_explosions = t[16].cast<bool>();          // NOLINT(*-magic-numbers)
                s.magic_active = t[17].cast<bool>();                // NOLINT(*-magic-numbers)
                s.blob_enclosed = t[18].cast<bool>();               // NOLINT(*-magic-numbers)
                s.is_agent_alive = t[19].cast<bool>();              // NOLINT(*-magic-numbers)
                s.is_agent_in_exit = t[20].cast<bool>();            // NOLINT(*-magic-numbers)
                s.blob_swap = t[21].cast<int8_t>();                 // NOLINT(*-magic-numbers)
                s.grid = t[22].cast<std::vector<int8_t>>();         // NOLINT(*-magic-numbers)
                s.has_updated = t[23].cast<std::vector<bool>>();    // NOLINT(*-magic-numbers)
                return {std::move(s)};
            }))
        .def("apply_action",
             [](T &self, int action) {
                 if (action < 0 || action >= T::action_space_size()) {
                     throw std::invalid_argument("Invalid action.");
                 }
                 self.apply_action(static_cast<boulderdash::Action>(action));
             })
        .def("is_solution", &T::is_solution)
        .def("observation_shape", &T::observation_shape)
        .def("get_observation",
             [](const T &self) {
                 py::array_t<float> out = py::cast(self.get_observation());
                 return out.reshape(self.observation_shape());
             })
        .def("image_shape", &T::image_shape)
        .def("to_image",
             [](T &self) {
                 py::array_t<uint8_t> out = py::cast(self.to_image());
                 const auto obs_shape = self.observation_shape();
                 return out.reshape({static_cast<py::ssize_t>(obs_shape[1] * boulderdash::SPRITE_HEIGHT),
                                     static_cast<py::ssize_t>(obs_shape[2] * boulderdash::SPRITE_WIDTH),
                                     static_cast<py::ssize_t>(boulderdash::SPRITE_CHANNELS)});
             })
        .def("get_reward_signal", &T::get_reward_signal)
        .def("get_agent_index", &T::get_agent_index)
        .def("agent_alive", &T::agent_alive)
        .def("agent_in_exit", &T::agent_in_exit)
        .def("get_hidden_item", &T::get_hidden_item);
}
