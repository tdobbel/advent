import sys
from functools import cache

@cache
def count_possibilities(puzzle: str, target: tuple, value: int) -> int:
    if len(puzzle) == 0:
        if value == 0:
            return 1 if len(target) == 0 else 0
        elif len(target) != 1:
            return 0
        else:
            return 1 if value == target[0] else 0
    if len(target) == 0 and "#" in puzzle:
        return 0
    if puzzle[0] == "?":
        return count_possibilities('#'+puzzle[1:], target, value) +  \
            count_possibilities('.'+puzzle[1:], target, value)
    elif puzzle[0] == ".":
        if value == 0:
            return count_possibilities(puzzle[1:], target, 0)
        if len(target) == 0 or target[0] != value:
            return 0
        return count_possibilities(puzzle[1:], target[1:], 0)
    else:
        return count_possibilities(puzzle[1:], target, value+1)

def main(input_file: str) -> None:
    with open(input_file, 'r') as f:
        lines = f.read().splitlines()
    total1 = 0
    total2 = 0
    for line in lines:
        puzzle, target = line.split()
        target = tuple(map(int, target.split(',')))
        total1 += count_possibilities(puzzle, target, 0)
        new_puzzle = '?'.join([puzzle]*5)
        new_target = tuple()
        for _ in range(5):
            new_target += target
        total2 += count_possibilities(new_puzzle, new_target, 0)
    print(f"Part 1: {total1}")
    print(f"Part 2: {total2}")

if __name__ == '__main__':
    main(sys.argv[1])

