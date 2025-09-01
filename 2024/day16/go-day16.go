package main

import (
	"bufio"
	"fmt"
	"log"
	"os"
	"slices"
)

const (
	North = iota
	East
	South
	West
)

type Position struct {
	x, y int
}

type Path struct {
	positions                []Position
	heading, score, distance int
}

func (p *Path) getCurrentPosition() Position {
	return p.positions[len(p.positions)-1]
}

func (p *Path) getCost() int {
	return p.score + p.distance
}

func (p *Path) getSize() int {
	return len(p.positions)
}

func abs(a int) int {
	if a < 0 {
		return -a
	}
	return a
}

func distance(a, b Position) int {
	return abs(a.x-b.x) + abs(a.y-b.y)
}

func getNextPos(pos Position, dir int) Position {
	var newPos Position
	switch dir {
	case North:
		newPos = Position{pos.x, pos.y - 1}
	case East:
		newPos = Position{pos.x + 1, pos.y}
	case South:
		newPos = Position{pos.x, pos.y + 1}
	case West:
		newPos = Position{pos.x - 1, pos.y}
	}
	return newPos
}

func directionFromPositions(posA, posB Position) int {
	if posB.y < posA.y {
		return North
	}
	if posB.y > posA.y {
		return South
	}
	if posB.x > posA.x {
		return East
	}
	return West
}

func createPositions(positions []Position, pos Position) []Position {
	n := len(positions)
	var res = make([]Position, n+1)
	for i := range n {
		res[i] = positions[i]
	}
	res[n] = pos
	return res
}

func showPath(path Path, isWall [][]bool, end Position) {
	ny := len(isWall)
	nx := len(isWall[0])
	var isPath = make([][]bool, ny)
	for y := range ny {
		isPath[y] = make([]bool, nx)
	}
	for _, pos := range path.positions {
		isPath[pos.y][pos.x] = true
	}
	for y := range ny {
		for x := range nx {
			if isWall[y][x] {
				fmt.Print("#")
			} else if isPath[y][x] {
				fmt.Print("x")
			} else if y == end.y && x == end.x {
				fmt.Print("S")
			} else {
				fmt.Print(".")
			}
		}
		fmt.Println()
	}
}

func findShortestPath(start, end Position, heading int, isWall [][]bool) (Path, error) {
	ny := len(isWall)
	nx := len(isWall[0])
	var visited = make([][]bool, ny)
	for i := range visited {
		visited[i] = make([]bool, nx)
	}
	visited[start.y][start.x] = true
	startPath := Path{
		positions: []Position{start},
		heading:   heading,
		score:     0,
		distance:  distance(start, end),
	}
	var shortestPath Path
	var newHeading, newScore int
	var pos, newPos Position
	queue := []Path{startPath}
	shifts := [3]int{0, 1, 3} // left, forward, right
	for len(queue) > 0 {
		minCost := queue[0].getCost()
		minCostIndex := 0
		var cost int
		for i, p := range queue[1:] {
			cost = p.getCost()
			if cost < minCost {
				minCost = cost
				minCostIndex = i + 1
			}
		}
		shortestPath = queue[minCostIndex]
		pos = shortestPath.getCurrentPosition()
		// showPath(shortestPath, isWall, end)
		queue = slices.Delete(queue, minCostIndex, minCostIndex+1)
		if pos == end {
			return shortestPath, nil
		}
		for _, shift := range shifts {
			newHeading = (shortestPath.heading + shift) % 4
			newPos = getNextPos(pos, newHeading)
			if isWall[newPos.y][newPos.x] || visited[newPos.y][newPos.x] {
				continue
			}
			visited[newPos.y][newPos.x] = true
			if newHeading == shortestPath.heading {
				newScore = shortestPath.score + 1
			} else {
				newScore = shortestPath.score + 1001
			}
			newPath := Path{
				positions: createPositions(shortestPath.positions, newPos),
				heading:   newHeading,
				score:     newScore,
				distance:  distance(newPos, end),
			}
			queue = append(queue, newPath)
		}
	}
	return shortestPath, fmt.Errorf("Could not reach end position")
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

	var isWall [][]bool
	var startPos, endPos Position
	ny := 0

	for fileScanner.Scan() {
		line := fileScanner.Text()
		row := make([]bool, len(line))
		for x, char := range line {
			switch char {
			case '#':
				row[x] = true
			case '.':
				continue
			case 'S':
				startPos = Position{x, ny}
			case 'E':
				endPos = Position{x, ny}
			default:
				log.Fatalf("Unknown character: %c", char)
			}
		}
		isWall = append(isWall, row)
		ny++
	}

	if err := fileScanner.Err(); err != nil {
		log.Fatal(err)
		return
	}

	shortestPath, err := findShortestPath(startPos, endPos, East, isWall)
	if err != nil {
		log.Fatal(err)
		return
	}
	fmt.Println("Part1:", shortestPath.score)

	var isShortest = make([][]bool, ny)
	nx := len(isWall[0])
	for i := range ny {
		isShortest[i] = make([]bool, nx)
	}
	for _, pos := range shortestPath.positions {
		isShortest[pos.y][pos.x] = true
	}

	heading := East
	minScore := shortestPath.score
	baseScore := 0
	start := startPos
	for i := range shortestPath.getSize() - 2 {
		pos := shortestPath.positions[i+1]
		isWall[pos.y][pos.x] = true
		shortest, err := findShortestPath(start, endPos, heading, isWall)
		if err == nil && baseScore+shortest.score == minScore {
			for _, p := range shortest.positions {
				isShortest[p.y][p.x] = true
			}
		}
		isWall[pos.y][pos.x] = false
		nextHeading := directionFromPositions(start, pos)
		baseScore++
		if nextHeading != heading {
			baseScore += 1000
		}
		heading = nextHeading
		start = pos
	}

	part2 := 0
	for y := range ny {
		for x := range nx {
			if isShortest[y][x] {
				part2++
			}
		}
	}
	fmt.Println("Part 2: ", part2)
}
