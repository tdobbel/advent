#!/usr/bin/python3
from __future__ import annotations
import sys

TOTAL_DISK_SPACE = 70_000_000
REQUIRED_SPACE = 30_000_000


class Directory:
    def __init__(self, name: str, parent: "Directory" | None) -> None:
        self.name = name
        self.children = {}
        self.fsize = 0
        self.parent = parent

    def add_child(self, name: str) -> None:
        self.children[name] = Directory(name, self)

    def get_size(self) -> int:
        return self.fsize + sum(child.get_size() for child in self.children.values())


def get_total_size_below_limit(dir: Directory, limit: int) -> int:
    dir_size = dir.get_size()
    result = 0
    if dir_size < limit:
        result += dir_size
    for child in dir.children.values():
        result += get_total_size_below_limit(child, limit)
    return result


def get_minimum_free(dir: Directory, target_size: int) -> int:
    dir_size = dir.get_size()
    if dir_size < target_size:
        return TOTAL_DISK_SPACE
    result = dir_size
    for child in dir.children.values():
        result = min(result, get_minimum_free(child, target_size))
    return result


def main() -> None:
    filename = sys.argv[1]
    root = Directory("/", None)
    current = root
    with open(filename, "r") as f:
        line = f.readline().strip()
        while line := f.readline().strip():
            split = line.split()
            match split[0]:
                case "$":
                    match split[1]:
                        case "cd":
                            dst = split[2]
                            current = (
                                current.parent if dst == ".." else current.children[dst]
                            )
                        case "ls":
                            continue
                        case _:
                            raise ValueError(f"Unknown command: {split[1]}")
                case "dir":
                    current.add_child(split[1])
                case _:
                    size = int(split[0])
                    current.fsize += size
    part1 = get_total_size_below_limit(root, 100_000)
    print(f"Part 1: {part1}")
    free_space = TOTAL_DISK_SPACE - root.get_size()
    target_size = REQUIRED_SPACE - free_space
    part2 = get_minimum_free(root, target_size)
    print(f"Part 2: {part2}")


if __name__ == "__main__":
    main()
