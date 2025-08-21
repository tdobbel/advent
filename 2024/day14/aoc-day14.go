package main

import (
	"bufio"
	"fmt"
	"log"
	"os"
	"strconv"
	"strings"
)

type Robot struct {
	x, y, u, v int
}

const NX = 101
const NY = 103

func buildGrid(robots []Robot) [NY][NX]int {
	var grid [NY][NX]int
	for _, robot := range robots {
		grid[robot.y][robot.x] += 1
	}
	return grid
}

func plotGrid(grid [NY][NX]int) {
	for y := range NY {
		for x := range NX {
			if grid[y][x] == 0 {
				fmt.Print(".")
			} else {
				fmt.Printf("%d", grid[y][x])
			}
		}
		fmt.Println()
	}
}

func iterate(grid *[NY][NX]int, robots []Robot) {
	for i := range robots {
		robot := &robots[i]
		grid[robot.y][robot.x]--
		robot.x = (robot.x + robot.u) % NX
		robot.y = (robot.y + robot.v) % NY
		if robot.x < 0 {
			robot.x += NX
		}
		if robot.y < 0 {
			robot.y += NY
		}
		grid[robot.y][robot.x]++
	}
}

func computeStabilityScore(robots []Robot) int {
	xMid := NX / 2
	yMid := NY / 2
	topLeft := 0
	topRight := 0
	botLeft := 0
	botRight := 0
	for _, robot := range robots {
		if robot.x == xMid || robot.y == yMid {
			continue
		}
		if robot.x < xMid {
			if robot.y < yMid {
				topLeft++
			} else {
				botLeft++
			}
		} else {
			if robot.y < yMid {
				topRight++
			} else {
				botRight++
			}
		}
	}
	return botLeft * botRight * topLeft * topRight
}

func computeMaxTreeHeight(grid [NY][NX]int, robots []Robot) int {
	hmax := 0
	var h, y, x_start, x_end int
	for _, robot := range robots {
		h = 1
		y = robot.y
		x_start = robot.x - 1
		x_end = robot.x + 1
		for y < NY && x_start >= 0 && x_end < NX {
			valid := true
			for x := x_start; x <= x_end; x++ {
				if grid[y][x] == 0 {
					valid = false
					break
				}
			}
			if !valid {
				break
			}
			x_start--
			x_end++
			y++
			h++
		} 
		if h > hmax {
			hmax = h
		}
	}
	return hmax
}

func main() {

	if len(os.Args) != 2 {
		fmt.Println("Please provide a filename")
		os.Exit(1)
	}

	filePath := os.Args[1]
	readFile, err := os.Open(filePath)

	if err != nil {
		fmt.Println(err)
		return
	}
	defer readFile.Close()
	fileScanner := bufio.NewScanner(readFile)
	fileScanner.Split(bufio.ScanLines)

	var robots []Robot

	for fileScanner.Scan() {
		line := fileScanner.Text()
		parts := strings.Fields(line)
		xy := strings.Split(parts[0][2:], ",")
		x, err := strconv.Atoi(xy[0])
		if err != nil {
			log.Fatal(err)
			return
		}
		y, err := strconv.Atoi(xy[1])
		if err != nil {
			log.Fatal(err)
			return
		}
		uv := strings.Split(parts[1][2:], ",")
		u, err := strconv.Atoi(uv[0])
		if err != nil {
			log.Fatal(err)
			return
		}
		v, err := strconv.Atoi(uv[1])
		if err != nil {
			log.Fatal(err)
			return
		}
		robots = append(robots, Robot{x: x, y: y, u: u, v: v})
	}
	if err := fileScanner.Err(); err != nil {
		log.Fatal(err)
		return
	}
	grid := buildGrid(robots)
	for range 100 {
		iterate(&grid, robots)
	}
	part1 := computeStabilityScore(robots)
	fmt.Println("Part 1:", part1)

	part2 := 100
	treeSize := computeMaxTreeHeight(grid, robots)
	fmt.Println("Tree size:", treeSize)
	for treeSize < 8 {
		iterate(&grid, robots)
		part2++
		treeSize = computeMaxTreeHeight(grid, robots)
	}
	fmt.Println("Part 2:", part2)
	plotGrid(grid)
}
