import sys
from enum import Enum, auto


class Operation(Enum):
    ADD = auto()
    MULTIPLY = auto()
    CONCATENATE = auto()


def apply_operation(op: Operation, x: int, y: int) -> int:
    match op:
        case Operation.ADD:
            return x + y
        case Operation.MULTIPLY:
            return x * y
        case Operation.CONCATENATE:
            return int(f"{x}{y}")


def ispossible(
    result: int, x: int, numbers: list[int], operations: list[Operation]
) -> bool:
    if len(numbers) == 0:
        return x == result
    y = numbers[0]
    for op in operations:
        x_new = apply_operation(op, x, y)
        if x_new > result:
            continue
        if ispossible(result, x_new, numbers[1:], operations):
            return True
    return False


def main() -> None:
    part1 = 0
    part2 = 0
    op1 = [Operation.ADD, Operation.MULTIPLY]
    op2 = [Operation.ADD, Operation.MULTIPLY, Operation.CONCATENATE]
    with open(sys.argv[1]) as f:
        line = f.readline().strip()
        while line:
            isep = line.index(":")
            result = int(line[:isep])
            numbers = list(map(int, line[isep + 1 :].split()))
            if ispossible(result, numbers[0], numbers[1:], op1):
                part1 += result
            elif ispossible(result, numbers[0], numbers[1:], op2):
                part2 += result
            line = f.readline().strip()
    print(f"Part 1: {part1}")
    print(f"Part 2: {part2}")


if __name__ == "__main__":
    main()
