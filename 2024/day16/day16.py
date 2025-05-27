from typing import Optional
import sys
from dataclasses import dataclass
from enum import IntEnum
import heapq


class Direction(IntEnum):
    UP = 0
    RIGHT = 1
    DOWN = 2
    LEFT = 3


@dataclass
class Path:
    positions: list[tuple[int, int]]
    direction: Direction
    score: int
    distance: int

    @property
    def total_score(self) -> int:
        return self.score + self.distance

    def next_position(self, dir: Direction) -> tuple[int, int]:
        x, y = self.positions[-1]
        match dir:
            case Direction.UP:
                y -= 1
            case Direction.RIGHT:
                x += 1
            case Direction.DOWN:
                y += 1
            case Direction.LEFT:
                x -= 1
        return (x, y)

    def __lt__(self, other):
        return self.total_score < other.total_score

    def __gt__(self, other):
        return self.total_score > other.total_score

    def __ge__(self, other):
        return self.total_score >= other.total_score

    def __le__(self, other):
        return self.total_score <= other.total_score


class Maze:
    def __init__(self, filename: str):
        with open(filename, "r") as f:
            lines = f.read().splitlines()
        self.ny = 0
        self.walls = []
        for line in lines:
            line = line.strip()
            self.nx = len(line)
            row = [False] * self.nx
            for x, c in enumerate(line):
                pos = (x, self.ny)
                if c == "#":
                    row[x] = True
                elif c == "S":
                    self.start = pos
                elif c == "E":
                    self.end = pos
            self.ny += 1
            self.walls.append(row)

    def distance_to_end(self, x: int, y: int) -> int:
        return abs(x - self.end[0]) + abs(y - self.end[1])


def next_moves(maze: Maze, path: Path, queue: list[Path], visited: set[Path]) -> None:
    directions = ((path.direction + 3) % 4, path.direction, (path.direction + 1) % 4)
    for dir in directions:
        x, y = path.next_position(dir)
        if x < 0 or x >= maze.nx or y < 0 or y >= maze.ny or maze.walls[y][x]:
            continue
        if (x, y) in visited:
            continue
        visited.add((x, y))
        score = path.score + 1
        if dir != path.direction:
            score += 1000
        dist = maze.distance_to_end(x, y)
        positions = path.positions + [(x, y)]
        heapq.heappush(queue, Path(positions, dir, score, dist))


def solve_maze(maze: Maze, start_path: Path, visited: set[Path]) -> Optional[Path]:
    queue = [start_path]
    heapq.heapify(queue)
    while len(queue) > 0:
        path = heapq.heappop(queue)
        if path.positions[-1] == maze.end:
            return path
        next_moves(maze, path, queue, visited)
    return None


def solve_part1(maze: Maze) -> Path:
    path = Path([maze.start], Direction.RIGHT, 0, maze.distance_to_end(*maze.start))
    visited = {maze.start}
    return solve_maze(maze, path, visited)


def solve_part2(maze: Maze, ref_path: Path) -> int:
    n = len(ref_path.positions)
    path = Path([maze.start], Direction.RIGHT, 0, maze.distance_to_end(*maze.start))
    all_pos = set()
    for i in range(1, n - 1):
        visited = set()
        for j in range(i + 1):
            visited.add(ref_path.positions[j])
        new_path = solve_maze(maze, path, visited)
        if new_path is not None and new_path.score <= ref_path.score:
            _ = [all_pos.add(p) for p in new_path.positions]
        directions = [
            (path.direction + 3) % 4,
            path.direction,
            (path.direction + 1) % 4,
        ]
        for dir in directions:
            x, y = path.next_position(dir)
            if ref_path.positions[i] == (x, y):
                path.positions.append((x, y))
                path.score += 1 if dir == path.direction else 1001
                path.direction = dir
                path.distance = maze.distance_to_end(x, y)
                break
    return len(all_pos)


def main() -> None:
    maze = Maze(sys.argv[1])
    path = solve_part1(maze)
    print("Part 1:", path.score)
    part2 = solve_part2(maze, path)
    print("Part 2:", part2)


if __name__ == "__main__":
    main()
