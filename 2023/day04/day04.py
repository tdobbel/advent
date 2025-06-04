import sys


def main() -> None:
    part1 = 0
    counter = {}
    with open(sys.argv[1], "r") as f:
        lines = f.read().splitlines()
    for i, line in enumerate(lines):
        counter[i] = counter.get(i, 0) + 1
        start = line.index(":")
        line = line[start + 2 :].strip()
        lhs, rhs = line.split(" | ")
        winning = [int(x) for x in lhs.split()]
        hand = [int(x) for x in rhs.split()]
        n_win = 0
        for card in hand:
            if card in winning:
                n_win += 1
                counter[i + n_win] = counter.get(i + n_win, 0) + counter[i]
        if n_win > 0:
            part1 += 1 << (n_win - 1)

    part2 = sum(counter.values())

    print("Part 1", part1)
    print("Part 2", part2)


if __name__ == "__main__":
    main()
