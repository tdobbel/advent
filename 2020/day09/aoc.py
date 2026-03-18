import argparse


def isvalid(nums: list[int], num: int) -> bool:
    for i, x in enumerate(nums[:-1]):
        if x > num:
            continue
        for y in nums[i + 1 :]:
            if x + y == num:
                return True
    return False


def solve_part2(nums: list[int]) -> int:
    target = nums[-1]
    for i in range(len(nums) - 2):
        sum = nums[i]
        for j in range(i + 1, len(nums) - 1):
            sum += nums[j]
            if sum == target:
                vmin = min(nums[i : j + 1])
                vmax = max(nums[i : j + 1])
                return vmin + vmax
            if sum > target:
                break
    return -1


def solve(input_file: str, npre: int) -> None:
    nums = []
    with open(input_file, "r") as fp:
        for _ in range(npre):
            nums.append(int(fp.readline().strip()))
        while line := fp.readline():
            num = int(line.strip())
            nums.append(num)
            if not isvalid(nums[-npre - 1 : -1], num):
                break
    print(f"Part 1: {nums[-1]}")
    part2 = solve_part2(nums)
    assert part2 >= 0
    print(f"Part 2: {part2}")


def main() -> None:
    parser = argparse.ArgumentParser()
    parser.add_argument("input_file", type=str)
    parser.add_argument("npre", type=int)
    args = parser.parse_args()

    solve(args.input_file, args.npre)


if __name__ == "__main__":
    main()
