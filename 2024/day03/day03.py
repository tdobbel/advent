import sys
import re

def parse_line(line: str) -> int:
    res = 0
    matches = re.findall(r"mul\((\d+),(\d+)\)", line)
    for x, y in matches:
        res += int(x)*int(y)
    return res

def main(filename: str) -> None:
    part1 = 0
    part2 = 0
    do = True
    with open(filename, 'r') as f:
        line = f.readline().strip()
        while line:
            part1 += parse_line(line)
            split = line.split("()")
            for entry in split:
                if do:
                    part2 += parse_line(entry)
                if entry.endswith("don't"):
                    do = False
                elif entry.endswith("do"):
                    do = True
            line = f.readline().strip()
    print(f"Part 1: {part1}")
    print(f"Part 2: {part2}")

if __name__ == '__main__':
    main(sys.argv[1])
