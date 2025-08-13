package main

import (
	"bufio"
	"fmt"
	"log"
	"os"
	"strconv"
	// "strings"
)

type Pair struct {
	x, y int
}

func countTrails(grid [][]int, pos Pair, found map[Pair]bool, total *int) {
	height := grid[pos.y][pos.x]
	if height == 9 {
		*total++
		found[pos] = true
		return
	}
	ny := len(grid)
	nx := len(grid[0])
	shifts := [4]Pair{{1, 0}, {-1, 0}, {0, 1}, {0, -1}}
	var x, y int
	for _, shift := range shifts {
		x = pos.x + shift.x
		y = pos.y + shift.y
		if x < 0 || x >= nx || y < 0 || y >= ny {
			continue
		}
		if grid[y][x] == height+1 {
			countTrails(grid, Pair{x, y}, found, total)
		}
	}
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

	var grid [][]int
	var candidates []Pair

	ny := 0
	for fileScanner.Scan() {
		line := fileScanner.Text()
		row := make([]int, len(line))
		for x, char := range line {
			num, err := strconv.Atoi(string(char))
			if err != nil {
				log.Fatal(err)
				return
			}
			if num == 0 {
				candidates = append(candidates, Pair{x, ny})
			}
			row[x] = num
		}
		grid = append(grid, row)
		ny++
	}
	if err := fileScanner.Err(); err != nil {
		log.Fatal(err)
		return
	}

	part1 := 0
	part2 := 0

	var found map[Pair]bool
	var total int
	for _, pos := range candidates {
		found = make(map[Pair]bool)
		total = 0
		countTrails(grid, pos, found, &total)
		part1 += len(found)
		part2 += total
	}

	fmt.Println("Part 1:", part1)
	fmt.Println("Part 2:", part2)

}
