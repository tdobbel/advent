import sys


def main() -> None:
    antennas: dict[str, list[tuple[int, int]]] = {}
    nx = 0
    ny = 0
    with open(sys.argv[1], "r") as f:
        line = f.readline().strip()
        while line:
            nx = len(line)
            for x, c in enumerate(line):
                if c == ".":
                    continue
                if c in antennas:
                    antennas[c].append((x, ny))
                else:
                    antennas[c] = [(x, ny)]
            line = f.readline().strip()
            ny += 1

    def inside(x: int, y: int) -> bool:
        return 0 <= x < nx and 0 <= y < ny

    antinodes1: set(tuple[int, int]) = set()
    antinodes2: set(tuple[int, int]) = set()
    for a, coords in antennas.items():
        n = len(coords)
        for i in range(n - 1):
            xa, ya = coords[i]
            for j in range(i + 1, n):
                xb, yb = coords[j]
                dx = xb - xa
                dy = yb - ya
                if inside(xb + dx, yb + dy):
                    antinodes1.add((xb + dx, yb + dy))
                x, y = xb, yb
                while inside(x, y):
                    antinodes2.add((x, y))
                    x += dx
                    y += dy
                if inside(xa - dx, ya - dy):
                    antinodes1.add((xa - dx, ya - dy))
                x, y = xa, ya
                while inside(x, y):
                    antinodes2.add((x, y))
                    x -= dx
                    y -= dy
    print("Part 1:", len(antinodes1))
    print("Part 2:", len(antinodes2))


if __name__ == "__main__":
    main()
