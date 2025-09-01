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


def decode_sequence(seq: int) -> list[int]:
    decoded = [0] * 4
    for i in range(4):
        decoded[3 - i] = (seq & 0x1F) - 9
        seq >>= 5
    return decoded


def main() -> None:
    with open(sys.argv[1], "r") as f:
        lines = f.read().splitlines()
    prev = None
    part1 = 0
    part2 = 0
    best_seq = None
    gains = {}
    for line in lines:
        seq = 0
        secret = int(line.strip())
        prev = secret % 10
        gains_buyer = {}
        for i in range(2000):
            secret = next_number(secret)
            price = secret % 10
            seq = ((seq << 5) | (price - prev + 9)) & 0xFFFFF
            prev = price
            if i < 3:
                continue
            if seq not in gains_buyer:
                gains_buyer[seq] = price
        for key, value in gains_buyer.items():
            gains.setdefault(key, 0)
            gains[key] += value
            if gains[key] > part2:
                part2 = gains[key]
                best_seq = key
        part1 += secret
    print(f"Part 1: {part1}")
    print(f"Part 2: {part2} with sequence {decode_sequence(best_seq)}")


if __name__ == "__main__":
    main()
