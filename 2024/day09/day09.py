import sys
import itertools
from dataclasses import dataclass


@dataclass
class Block:
    values: list[int]
    size: int
    counter: int


def blocks_to_list(blocks: list[Block]) -> list[int]:
    return list(itertools.chain(*(block.values for block in blocks)))


def disk_sum(disk: list[int]) -> int:
    return sum(i * d if d >= 0 else 0 for i, d in enumerate(disk))


def main() -> None:
    with open(sys.argv[1], "r") as f:
        line = f.readline().strip()
    blocks = []
    for i, c in enumerate(line):
        size = int(c)
        if i % 2 == 0:
            values = [i // 2] * size
            counter = size
        else:
            values = [-1] * size
            counter = 0
        blocks.append(Block(values, size, counter))
    disk = blocks_to_list(blocks)
    ileft = 0
    iright = len(disk) - 1
    while True:
        while disk[ileft] >= 0:
            ileft += 1
        while disk[iright] < 0:
            iright -= 1
        if ileft > iright:
            break
        disk[ileft], disk[iright] = disk[iright], disk[ileft]
    part1 = disk_sum(disk)
    print(f"Part 1: {part1}")
    iright = len(blocks) - 1
    while True:
        ileft = 1
        required = blocks[iright].size
        while ileft < iright:
            free = blocks[ileft].size - blocks[ileft].counter
            if free >= required:
                for i in range(required):
                    j = blocks[ileft].counter
                    blocks[ileft].values[j] = blocks[iright].values[i]
                    blocks[iright].values[i] = -1
                    blocks[ileft].counter += 1
                break
            ileft += 2
        iright -= 2
        if iright == 0:
            break
    part2 = disk_sum(blocks_to_list(blocks))
    print(f"Part 2: {part2}")


if __name__ == "__main__":
    main()
