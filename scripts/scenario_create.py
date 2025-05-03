import argparse
import copy
import os
import random
import sys
from multiprocessing import Manager, Pool, cpu_count

import numpy as np

kAgent = 0
kEmpty = 1
kDirt = 2
kDiamond = 5
kExitClosed = 7
kExitOpen = 8
kWallBrick = 18
kGateRedClosed = 27
kGateRedOpen = 28
kKeyRed = 29
kGateBlueClosed = 30
kGateBlueOpen = 31
kKeyBlue = 32
kGateGreenClosed = 33
kGateGreenOpen = 34
kKeyGreen = 35
kGateYellowClosed = 36
kGateYellowOpen = 37
kKeyYellow = 38

kBackgroundTiles = [kEmpty, kDirt]
kKeys = [kKeyRed, kKeyBlue, kKeyGreen, kKeyYellow]
kGatesOpen = [kGateRedOpen, kGateBlueOpen, kGateGreenOpen, kGateYellowOpen]
kGatesClosed = [kGateRedClosed, kGateBlueClosed, kGateGreenClosed, kGateYellowClosed]

wall_idxs_upper_left = [4, 18, 32, 51, 64, 65, 66]
wall_idxs_upper_right = [11, 27, 43, 60, 77, 78, 79]
wall_idxs_lower_left = [176, 177, 178, 195, 212, 228, 244]
wall_idxs_lower_right = [189, 190, 191, 204, 219, 235, 251]
wall_idxs_middle = [
    86,
    87,
    88,
    89,
    101,
    106,
    117,
    122,
    133,
    138,
    149,
    154,
    166,
    167,
    168,
    169,
]
wall_idxs_all = (
    wall_idxs_upper_left
    + wall_idxs_upper_right
    + wall_idxs_lower_left
    + wall_idxs_lower_right
    + wall_idxs_middle
)

door_idxs_upper_left = {4: (3, 5), 18: (19, 21), 64: (48, 80), 65: (49, 81)}
door_idxs_upper_right = {11: (10, 12), 27: (26, 28), 78: (62, 94), 79: (63, 95)}
door_idxs_lower_left = {
    176: (160, 192),
    177: (161, 193),
    228: (227, 229),
    244: (243, 245),
}
door_idxs_lower_right = {
    190: (174, 206),
    191: (175, 207),
    235: (234, 236),
    251: (250, 252),
}
door_idxs_middle = {
    87: (71, 103),
    88: (72, 104),
    117: (116, 118),
    122: (121, 123),
    133: (132, 134),
    138: (137, 139),
    167: (151, 183),
    168: (152, 184),
}


room_info = {
    "upper_left": {
        "wall_idxs": [4, 18, 32, 45, 56, 57, 58],
        "inner_idxs": [0, 1, 2, 14, 15, 16, 28, 29, 30, 31, 44],
        "door_blocked_idxs": {4: (3, 5), 18: (17, 19), 56: (42, 70), 57: (43, 71)},
    },
    "upper_right": {
        "wall_idxs": [9, 23, 37, 52, 67, 68, 69],
        "inner_idxs": [11, 12, 13, 25, 26, 27, 38, 39, 40, 41, 53],
        "door_blocked_idxs": {9: (8, 10), 23: (22, 24), 68: (54, 82), 69: (55, 83)},
    },
    "lower_left": {
        "wall_idxs": [126, 127, 128, 143, 158, 172, 186],
        "inner_idxs": [142, 154, 155, 156, 157, 168, 169, 170, 182, 183, 184],
        "door_blocked_idxs": {
            126: (112, 140),
            127: (113, 141),
            172: (171, 173),
            186: (185, 187),
        },
    },
    "lower_right": {
        "wall_idxs": [137, 138, 139, 150, 163, 177, 191],
        "inner_idxs": [151, 164, 165, 166, 167, 179, 180, 181, 193, 194, 195],
        "door_blocked_idxs": {
            138: (124, 152),
            139: (125, 153),
            177: (176, 178),
            191: (190, 192),
        },
    },
    "middle": {
        "wall_idxs": [62, 63, 75, 78, 88, 93, 102, 107, 117, 120, 132, 133],
        "inner_idxs": [90, 91, 104, 105],
        "door_blocked_idxs": {
            62: (48, 76),
            63: (49, 77),
            88: (87, 89),
            93: (92, 94),
            102: (101, 103),
            107: (106, 108),
            132: (118, 146),
            133: (119, 147),
        },
    },
}
room_ids = ["upper_left", "upper_right", "lower_left", "lower_right", "middle"]

room_reserved_indices = []
for _, room in room_info.items():
    room_reserved_indices += room["wall_idxs"]
    room_reserved_indices += room["inner_idxs"]
    for idxs in room["door_blocked_idxs"].values():
        room_reserved_indices += list(idxs)
room_reserved_indices = set(room_reserved_indices)
non_reserved_indices = set(
    [i for i in range(14 * 14) if i not in room_reserved_indices]
)


def create_map_scenario1(
    seed, m: list[int], blocked_indices: set[int], gen: np.random.RandomState
):
    # Diamond in locked room, exit in open
    room_indices = [i for i in range(len(room_ids))]
    gen.shuffle(room_indices)
    diamond_room = room_ids[room_indices[0]]
    key_door_ids = [i for i in range(len(kKeys))]
    gen.shuffle(key_door_ids)
    key_door_idx = key_door_ids.pop()

    # agent/exit cannot be inside diamond room
    for idx in room_info[diamond_room]["inner_idxs"]:
        blocked_indices.add(idx)
    for idxs in room_info[diamond_room]["door_blocked_idxs"].values():
        for idx in idxs:
            blocked_indices.add(idx)

    # Put locked door on selected room
    wall_idx = gen.choice(list(room_info[diamond_room]["door_blocked_idxs"].keys()))
    m[wall_idx] = kGatesClosed[key_door_idx]
    for idx in room_info[diamond_room]["door_blocked_idxs"][wall_idx]:
        m[idx] = kEmpty

    # Put diamond inside room
    gem_idx = gen.choice(room_info[diamond_room]["inner_idxs"])
    m[gem_idx] = kDiamond

    # Put key inside another room
    key_room = room_ids[room_indices[1]]
    key_idx = gen.choice(room_info[key_room]["inner_idxs"])
    m[key_idx] = kKeys[key_door_idx]
    blocked_indices.add(key_idx)

    # Open all other rooms
    for room in room_ids:
        if room == diamond_room:
            continue
        open_idx = gen.choice(list(room_info[room]["door_blocked_idxs"].keys()))
        for idx in room_info[room]["door_blocked_idxs"][open_idx]:
            m[idx] = kEmpty
            blocked_indices.add(idx)
        if gen.choice([0, 1], p=[0.25, 0.75]) == 0 or len(key_door_ids) == 0:
            m[open_idx] = kEmpty
        else:
            m[open_idx] = kGatesOpen[key_door_ids.pop()]

    # Place agent and exit, with possible decoy key
    agent_idx, goal_idx, key_idx = gen.choice(
        [i for i in range(14 * 14) if i not in blocked_indices], size=3, replace=False
    )
    m[agent_idx] = kAgent
    m[goal_idx] = kExitClosed
    if gen.choice([0, 1]) == 0:
        m[key_idx] = kKeys[
            gen.choice([i for i in range(len(kKeys)) if i != key_door_idx])
        ]

    return m


def create_map_3keys(
    seed, m: list[int], blocked_indices: set[int], gen: np.random.RandomState
):
    # Diamond in locked room, exit in open
    room_indices = [i for i in range(len(room_ids))]
    gen.shuffle(room_indices)
    diamond_room = room_ids[room_indices[0]]
    key_door_ids = [i for i in range(len(kKeys))][::-1]
    gen.shuffle(key_door_ids)
    key_door_idx1 = key_door_ids.pop()
    key_door_idx2 = key_door_ids.pop()
    key_door_idx3 = key_door_ids.pop()

    # agent/exit cannot be inside diamond room
    for idx in room_info[diamond_room]["inner_idxs"]:
        blocked_indices.add(idx)
    for idxs in room_info[diamond_room]["door_blocked_idxs"].values():
        for idx in idxs:
            blocked_indices.add(idx)

    # Put locked door on selected room
    wall_idx = gen.choice(list(room_info[diamond_room]["door_blocked_idxs"].keys()))
    m[wall_idx] = kGatesClosed[key_door_idx1]
    for idx in room_info[diamond_room]["door_blocked_idxs"][wall_idx]:
        m[idx] = kEmpty

    # Put diamond inside room
    gem_idx = gen.choice(room_info[diamond_room]["inner_idxs"])
    m[gem_idx] = kDiamond

    # Put 1st key inside another room
    key_room1 = room_ids[room_indices[1]]
    key_idx = gen.choice(room_info[key_room1]["inner_idxs"])
    m[key_idx] = kKeys[key_door_idx1]
    blocked_indices.add(key_idx)

    for idx in room_info[key_room1]["inner_idxs"]:
        blocked_indices.add(idx)
    for idxs in room_info[key_room1]["door_blocked_idxs"].values():
        for idx in idxs:
            blocked_indices.add(idx)

    wall_idx = gen.choice(list(room_info[key_room1]["door_blocked_idxs"].keys()))
    m[wall_idx] = kGatesClosed[key_door_idx2]
    for idx in room_info[key_room1]["door_blocked_idxs"][wall_idx]:
        m[idx] = kEmpty

    # Put 2nd key inside another room
    key_room2 = room_ids[room_indices[2]]
    key_idx = gen.choice(room_info[key_room2]["inner_idxs"])
    m[key_idx] = kKeys[key_door_idx2]
    blocked_indices.add(key_idx)

    for idx in room_info[key_room2]["inner_idxs"]:
        blocked_indices.add(idx)
    for idxs in room_info[key_room2]["door_blocked_idxs"].values():
        for idx in idxs:
            blocked_indices.add(idx)

    wall_idx = gen.choice(list(room_info[key_room2]["door_blocked_idxs"].keys()))
    m[wall_idx] = kGatesClosed[key_door_idx3]
    for idx in room_info[key_room2]["door_blocked_idxs"][wall_idx]:
        m[idx] = kEmpty

    # Put 3nd key inside another room
    key_room3 = room_ids[room_indices[3]]
    key_idx = gen.choice(room_info[key_room3]["inner_idxs"])
    m[key_idx] = kKeys[key_door_idx3]
    blocked_indices.add(key_idx)

    # Open all other rooms
    for room in room_ids:
        if room == diamond_room or room == key_room1 or room == key_room2:
            continue
        open_idx = gen.choice(list(room_info[room]["door_blocked_idxs"].keys()))
        for idx in room_info[room]["door_blocked_idxs"][open_idx]:
            m[idx] = kEmpty
            blocked_indices.add(idx)
        if gen.choice([0, 1], p=[0.25, 0.75]) == 0 or len(key_door_ids) == 0:
            m[open_idx] = kEmpty
        else:
            m[open_idx] = kGatesOpen[key_door_ids.pop()]

    # Place agent and exit
    agent_idx, goal_idx = gen.choice(
        [i for i in range(14 * 14) if i not in blocked_indices], size=2, replace=False
    )
    m[agent_idx] = kAgent
    m[goal_idx] = kExitClosed

    return m


def create_map_scenario_hard(
    seed,
    m: list[int],
    blocked_indices: set[int],
    num_diamonds: int,
    gen: np.random.RandomState,
):
    assert num_diamonds > 0
    # Diamond in locked room, exit in open
    room_indices = [i for i in range(len(room_ids))]
    gen.shuffle(room_indices)
    diamond_room = room_ids[room_indices[0]]
    key_door_ids = [i for i in range(len(kKeys))]
    gen.shuffle(key_door_ids)
    key_door_idx = key_door_ids.pop()

    # agent/exit cannot be inside diamond room
    for idx in room_info[diamond_room]["inner_idxs"]:
        blocked_indices.add(idx)
    for idxs in room_info[diamond_room]["door_blocked_idxs"].values():
        for idx in idxs:
            blocked_indices.add(idx)

    # Put locked door on selected room
    wall_idx = gen.choice(list(room_info[diamond_room]["door_blocked_idxs"].keys()))
    m[wall_idx] = kGatesClosed[key_door_idx]
    for idx in room_info[diamond_room]["door_blocked_idxs"][wall_idx]:
        m[idx] = kEmpty

    # Put diamond inside room
    gem_idx = gen.choice(room_info[diamond_room]["inner_idxs"])
    m[gem_idx] = kDiamond

    # Put key inside another room
    key_room = room_ids[room_indices[1]]
    key_idx = gen.choice(room_info[key_room]["inner_idxs"])
    m[key_idx] = kKeys[key_door_idx]
    blocked_indices.add(key_idx)

    # Open all other rooms
    for room in room_ids:
        if room == diamond_room:
            continue
        open_idx = gen.choice(list(room_info[room]["door_blocked_idxs"].keys()))
        for idx in room_info[room]["door_blocked_idxs"][open_idx]:
            m[idx] = kEmpty
            blocked_indices.add(idx)
        if gen.choice([0, 1], p=[0.25, 0.75]) == 0 or len(key_door_ids) == 0:
            m[open_idx] = kEmpty
        else:
            m[open_idx] = kGatesOpen[key_door_ids.pop()]

    # Place remaining diamonds
    if num_diamonds - 1 > 0:
        diamond_indices = gen.choice(
            [i for i in range(14 * 14) if i not in blocked_indices],
            size=num_diamonds - 1,
            replace=False,
        )
        for idx in diamond_indices:
            m[idx] = kDiamond
            blocked_indices.add(idx)

    # Place agent and exit, with possible decoy key
    agent_idx, goal_idx, key_idx = gen.choice(
        [i for i in range(14 * 14) if i not in blocked_indices], size=3, replace=False
    )
    m[agent_idx] = kAgent
    m[goal_idx] = kExitClosed
    if gen.choice([0, 1]) == 0:
        m[key_idx] = kKeys[
            gen.choice([i for i in range(len(kKeys)) if i != key_door_idx])
        ]

    return m


def runner(args):
    manager_dict, i, seed, use_hard = args
    DIRT_PERCENTAGE = 0.1
    # Create base map with rooms

    random.seed(seed)
    rng = np.random.default_rng(seed)
    m = [kEmpty for _ in range(14 * 14)]
    blocked_indices = set()
    for _, room in room_info.items():
        for idx in room["wall_idxs"]:
            m[idx] = kWallBrick
            blocked_indices.add(idx)

    corridor_idxs = [
        46,
        47,
        59,
        60,
        61,
        73,
        74,
        50,
        51,
        64,
        65,
        66,
        79,
        80,
        115,
        116,
        129,
        130,
        131,
        144,
        145,
        121,
        122,
        134,
        135,
        136,
        148,
        149,
    ]
    for idx in corridor_idxs:
        blocked_indices.add(idx)

    for idx in range(14 * 14):
        if m[idx] != kWallBrick:
            m[idx] = rng.choice(
                [kDirt, kEmpty], p=[DIRT_PERCENTAGE, 1.0 - DIRT_PERCENTAGE]
            )

    if not use_hard:
        num_diamonds = 1
        m = create_map_scenario1(seed, m, blocked_indices, rng)
        # m = create_map_3keys(seed, m, blocked_indices, rng)
    else:
        num_diamonds = 4
        m = create_map_scenario_hard(seed, m, blocked_indices, num_diamonds, rng)

    output_str = "{}|{}|{}|".format(14, 14, num_diamonds)
    for item in m:
        output_str += "{:02d}|".format(item)
    manager_dict[i] = output_str[:-1]


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument(
        "--num_samples",
        help="Number of total samples",
        required=False,
        type=int,
        default=10000,
    )
    parser.add_argument(
        "--export_path", help="Export path for file", required=True, type=str
    )
    parser.add_argument(
        "--seed", help="Start seed", type=int, required=False, default=0
    )
    parser.add_argument(
        "--hard",
        help="Only hard problems",
        required=False,
        type=bool,
        default=False,
    )
    args = parser.parse_args()

    manager = Manager()
    data = manager.dict()
    random.seed(args.seed)

    with Pool(cpu_count()) as p:
        p.map(
            runner,
            [(data, i, i + args.seed, args.hard) for i in range(args.num_samples)],
        )

    # Parse and save to file
    export_dir = os.path.dirname(args.export_path)
    if not os.path.exists(export_dir):
        os.makedirs(export_dir)

    with open(args.export_path, "w") as file:
        for i in range(len(data)):
            file.write(data[i])
            file.write("\n")


if __name__ == "__main__":
    main()
