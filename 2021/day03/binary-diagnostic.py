import argparse

parser = argparse.ArgumentParser()
parser.add_argument("input_file", type=str)
args = parser.parse_args()


def filter(report: list[str], take_max: bool) -> str:
    num_size = len(report[0])
    for i in range(num_size):
        ones = []
        zeros = []
        for number in report:
            if number[i] == "1":
                ones.append(number)
            else:
                zeros.append(number)
        if take_max ^ (len(ones) < len(zeros)):
            report = ones
        else:
            report = zeros
        if len(report) == 1:
            return report[0]
    raise ValueError("This should not be happening")


def main() -> None:
    reports: list[str] = []
    counter = []
    num_size = 0
    with open(args.input_file, "r") as fp:
        while line := fp.readline():
            line = line.strip()
            if len(counter) == 0:
                num_size = len(line)
                counter = [0] * num_size
            for i, char in enumerate(line):
                counter[i] += int(char)
            reports.append(line)
    gamma = 0
    epsilon = 0
    for cntr in counter:
        gamma <<= 1
        epsilon <<= 1
        if cntr > len(reports) / 2:
            gamma += 1
        else:
            epsilon += 1
    print(f"Part 1: {epsilon * gamma}")
    oxygen = filter(reports, True)
    co2 = filter(reports, False)
    print(f"Part 2: {int(oxygen, 2) * int(co2, 2)}")


if __name__ == "__main__":
    main()
