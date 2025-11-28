import argparse

parser = argparse.ArgumentParser()
parser.add_argument("input_file", type=str)

args = parser.parse_args()

prev = 0
part1 = 0
part2 = 0

values = [0] * 3
sum1 = sum2 = 0

with open(args.input_file, "r") as fp:
    ln = 0
    while line := fp.readline():
        depth = int(line)
        ln += 1
        if ln > 1 and depth > prev:
            part1 += 1
        prev = depth
        for i in range(3):
            if ln > i:
                values[i] += depth
        if ln < 3:
            continue
        sum1 = sum2
        sum2 = values[0]
        values = values[1:] + [0]
        if ln > 3 and sum2 > sum1:
            part2 += 1

print("Part 1", part1)
print("Part 2", part2)
