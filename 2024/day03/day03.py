import sys
import re


def main(filename: str) -> None:
    part1 = part2 = 0
    do = True
    pattern = re.compile(r"mul\((\d+),(\d+)\)")
    with open(filename, "r") as f:
        line = f.readline().strip()
        while line:
            for entry in line.split("()"):
                for x, y in pattern.findall(entry):
                    product = int(x) * int(y)
                    part1 += product
                    if do:
                        part2 += product
                if entry.endswith("don't"):
                    do = False
                elif entry.endswith("do"):
                    do = True
            line = f.readline().strip()
    print(f"Part 1: {part1}")
    print(f"Part 2: {part2}")


if __name__ == "__main__":
    main(sys.argv[1])
