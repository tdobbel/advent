import argparse

parser = argparse.ArgumentParser()
parser.add_argument("input_file", type=str)
args = parser.parse_args()

x = y = aim = 0

with open(args.input_file, "r") as fp:
    while line := fp.readline():
        line = line.strip()
        heading, n = line.split()
        n = int(n)
        match heading:
            case "forward":
                x += n
                y += aim * n
            case "up":
                aim -= n
            case "down":
                aim += n
            case _:
                raise ValueError(f"Unknwon direction '{heading}'")

    print(f"Part 1: {x * aim}")
    print(f"Part 2: {x * y}")
