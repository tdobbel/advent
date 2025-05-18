#!/usr/bin/env python3
import sys

def find_start_of_packet(line: str, size: int) -> int:
    for i in range(len(line)-size):
        ok = True
        for j in range(1, size):
            if line[i+j] in line[i:i+j]:
                ok = False
                break
        if ok:
            return i+size
    raise ValueError("No start of packet found")

def main() -> None:
    with open(sys.argv[1]) as f:
        line = f.read().strip()
    part1 = find_start_of_packet(line, 4)
    part2 = find_start_of_packet(line, 14)
    print(f"Part 1: {part1}")
    print(f"Part 2: {part2}")

if __name__ == "__main__":
    main()
