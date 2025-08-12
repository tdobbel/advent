package main

import (
	"bufio"
	"fmt"
	"log"
	"os"
)

type position struct {
	x, y int
}

func addAntinode(antennas map[position]bool, x, y, nx, ny int) bool {
	if x < 0 || x >= nx || y < 0 || y >= ny {
		return false
	}
	antennas[position{x, y}] = true
	return true
}

func countAntinodes(antennas map[rune][]position, nx, ny int) int {
	antinodes := make(map[position]bool)
	for _, positions := range antennas {
		for i := range len(positions) - 1 {
			posA := positions[i]
			for j := i + 1; j < len(positions); j++ {
				posB := positions[j]
				dx := posB.x - posA.x
				dy := posB.y - posA.y
				addAntinode(antinodes, posB.x+dx, posB.y+dy, nx, ny)
				addAntinode(antinodes, posA.x-dx, posA.y-dy, nx, ny)
			}
		}
	}
	return len(antinodes)
}

func countAntinodes2(antennas map[rune][]position, nx, ny int) int {
	antinodes := make(map[position]bool)
	var x, y int
	for _, positions := range antennas {
		for i := range len(positions) - 1 {
			posA := positions[i]
			for j := i + 1; j < len(positions); j++ {
				posB := positions[j]
				dx := posB.x - posA.x
				dy := posB.y - posA.y
				x = posB.x
				y = posB.y
				for addAntinode(antinodes, x, y, nx, ny) {
					x += dx
					y += dy
				}
				x = posA.x
				y = posA.y
				for addAntinode(antinodes, x, y, nx, ny) {
					x -= dx
					y -= dy
				}
			}
		}
	}
	return len(antinodes)
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

	antennas := make(map[rune][]position)
	ny := 0
	nx := 0
	for fileScanner.Scan() {
		line := fileScanner.Text()
		nx = len(line)
		for x, char := range line {
			if char != '.' {
				antennas[char] = append(antennas[char], position{x, ny})
			}
		}
		ny++
	}

	if err := fileScanner.Err(); err != nil {
		log.Fatal(err)
		return
	}

	part1 := countAntinodes(antennas, nx, ny)
	fmt.Println("Part 1:", part1)
	part2 := countAntinodes2(antennas, nx, ny)
	fmt.Println("Part 2:", part2)
}
