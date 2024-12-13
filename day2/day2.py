import sys

def issafe(numbers: list[int]) -> bool:
    assert len(numbers) >= 2
    increasing = numbers[1] > numbers[0]
    for i in range(len(numbers)-1):
        diff = numbers[i+1] - numbers[i]
        if not increasing:
            diff *= -1
        if not 1 <= diff <= 3:
            return False
    return True

def issafe_dampened(numbers: list[int]) -> bool:
    if issafe(numbers):
        return True
    for i in range(len(numbers)):
        if issafe(numbers[:i] + numbers[i+1:]):
            return True
    return False

def count_safe(file: str) -> None:
    total1 = 0
    total2 = 0
    with open(file,"r") as f:
        line = f.readline().strip()
        while line:
            numbers = list(map(int, line.split()))
            total1 += issafe(numbers)
            total2 += issafe_dampened(numbers)
            line = f.readline().strip()
    print("Total safe lines (part 1):", total1)
    print("Total safe lines (part 2):", total2)

if __name__ == "__main__":
    count_safe(sys.argv[1])
