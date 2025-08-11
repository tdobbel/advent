package main

import (
	"bufio"
	"fmt"
	"log"
	"os"
	// "strings"
)

const (
	UP    = 0
	RIGHT = 1
	DOWN  = 2
	LEFT  = 3
)

func moveGuard(startX int, startY int, grid [][]rune, visited []bool, historic [4][]bool) bool {
	heading := UP
	x := startX
	y := startY
	nx := len(grid[0])
	ny := len(grid)
	var nextX, nextY int
	for {
		visited[y*nx+x] = true
		switch heading {
		case UP:
			nextX = x
			nextY = y - 1
		case RIGHT:
			nextX = x + 1
			nextY = y
		case DOWN:
			nextX = x
			nextY = y + 1
		case LEFT:
			nextX = x - 1
			nextY = y
		}
		if nextX < 0 || nextX >= nx || nextY < 0 || nextY >= ny {
			return true
		}
		if grid[nextY][nextX] == '#' {
			heading = (heading + 1) % 4 // turn right
		} else {
			x = nextX
			y = nextY
		}
		if historic[heading][y*nx+x] {
			return false
		}
		historic[heading][y*nx+x] = true
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

	var grid [][]rune
	var visited []bool
	var historic [4][]bool
	var startX, startY int

	ny := 0
	nx := 0
	for fileScanner.Scan() {
		line := fileScanner.Text()
		nx = len(line)
		var row []rune
		for x, c := range line {
			row = append(row, c)
			if c == '^' {
				startX = x
				startY = ny
			}
			visited = append(visited, false)
		}
		grid = append(grid, row)
		ny++
	}

	for i := range 4 {
		historic[i] = make([]bool, nx*ny, nx*ny)
	}

	if err := fileScanner.Err(); err != nil {
		log.Fatal(err)
		return
	}

	part1 := 0
	part2 := 0

	moveGuard(startX, startY, grid, visited, historic)

	var extraWalls []int
	for i, v := range visited {
		if v {
			part1++
			if i != startY*nx+startX {
				extraWalls = append(extraWalls, i)
			}
		}
	}
	fmt.Println("Part 1:", part1)

	var x, y int
	for _, pos := range extraWalls {
		x = pos % nx
		y = pos / nx
		for i := range nx * ny {
			visited[i] = false
		}
		for d := range 4 {
			for i := range ny * nx {
				historic[d][i] = false
			}
		}
		grid[y][x] = '#'
		success := moveGuard(startX, startY, grid, visited, historic)
		if !success {
			part2++
		}
		grid[y][x] = '.'
	}
	fmt.Println("Part 2:", part2)
}
