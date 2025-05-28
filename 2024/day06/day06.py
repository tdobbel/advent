from typing import Optional
import sys
import itertools
from enum import IntEnum

State = tuple[int, int, int]


class Direction(IntEnum):
    UP = 0
    RIGHT = 1
    DOWN = 2
    LEFT = 3


def simulate_guard_path(
    obstacles: list[list[bool]],
    pos: tuple[int, int],
    dir: Direction,
) -> Optional[list[list[bool]]]:
    states: set[State] = set()
    x, y = pos
    nx = len(obstacles[0])
    ny = len(obstacles)
    visited = [[False] * nx for _ in range(ny)]
    while True:
        visited[y][x] = True
        x_next, y_next = x, y
        match dir:
            case Direction.UP:
                y_next -= 1
            case Direction.RIGHT:
                x_next += 1
            case Direction.DOWN:
                y_next += 1
            case Direction.LEFT:
                x_next -= 1
        if x_next < 0 or x_next >= nx or y_next < 0 or y_next >= ny:
            return visited
        if obstacles[y_next][x_next]:
            dir = (dir + 1) % 4
        else:
            x, y = x_next, y_next
        if (x, y, dir) in states:
            return None
        states.add((x, y, dir))


def main() -> None:
    obstacles = []
    start_pos = None
    with open(sys.argv[1], "r") as f:
        line = f.readline().strip()
        while line:
            row = []
            for x, c in enumerate(line):
                row.append(c == "#")
                if c == "^":
                    start_pos = (x, len(obstacles))
            obstacles.append(row)
            line = f.readline().strip()
    nx = len(obstacles[0])
    ny = len(obstacles)
    visited = simulate_guard_path(obstacles, start_pos, Direction.UP)
    part1 = sum(map(lambda x: sum(x), visited))
    print(f"Part 1: {part1}")
    part2 = 0
    for y, x in itertools.product(range(ny), range(nx)):
        if (x, y) == start_pos or not visited[y][x]:
            continue
        obstacles[y][x] = True
        if simulate_guard_path(obstacles, start_pos, Direction.UP) is None:
            part2 += 1
        obstacles[y][x] = False
    print(f"Part 2: {part2}")


if __name__ == "__main__":
    main()
