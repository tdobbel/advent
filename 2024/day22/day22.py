import sys


def mix(x: int, y: int) -> int:
    return x ^ y


def prune(x: int) -> int:
    return x & ((1 << 24) - 1)


def next_number(x: int) -> int:
    res = prune(mix(x << 6, x))
    res = prune(mix(res >> 5, res))
    res = prune(mix(res << 11, res))
    return res


def main() -> None:
    with open(sys.argv[1], "r") as f:
        lines = f.read().splitlines()
    prev = None
    part1 = 0
    part2 = 0
    best_seq = None
    gains = {}
    for line in lines:
        seq = tuple()
        secret = int(line.strip())
        prev = secret % 10
        gains_buyer = {}
        for i in range(2000):
            secret = next_number(secret)
            curr = secret % 10
            seq += (curr - prev,)
            prev = curr
            if i < 3:
                continue
            if seq not in gains_buyer:
                gains_buyer[seq] = curr
            seq = seq[1:]
        for key, value in gains_buyer.items():
            if key not in gains:
                gains[key] = 0
            gains[key] += value
            if gains[key] > part2:
                part2 = gains[key]
                best_seq = key
        part1 += secret
    print(f"Part 1: {part1}")
    print(f"Part 2: {part2} with sequence {best_seq}")


if __name__ == "__main__":
    main()
