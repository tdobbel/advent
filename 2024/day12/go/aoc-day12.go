package main

import (
	"bufio"
	"fmt"
	"log"
	"os"
)

const (
	Right = iota
	Up
	Left
	Down
)

type Edge struct {
	xa, ya int
	xb, yb int
}

func getEdge(x, y, direction int) (Edge, error) {
	var edge Edge
	switch direction {
	case Right:
		edge = Edge{xa: x + 1, ya: y + 1, xb: x + 1, yb: y}
		return edge, nil
	case Up:
		edge = Edge{xa: x + 1, ya: y, xb: x, yb: y}
		return edge, nil
	case Left:
		edge = Edge{xa: x, ya: y, xb: x, yb: y + 1}
		return edge, nil
	case Down:
		edge = Edge{xa: x, ya: y + 1, xb: x + 1, yb: y + 1}
		return edge, nil
	default:
		return Edge{}, fmt.Errorf("invalid direction: %d", direction)
	}
}

func countCorners(edges []Edge) int {
	u0 := edges[0].xb - edges[0].xa
	v0 := edges[0].yb - edges[0].ya
	n := len(edges)
	var u1, v1 int
	var edge Edge
	var crnrs int
	for i := range n {
		edge = edges[(i+1)%n]
		u1 = edge.xb - edge.xa
		v1 = edge.yb - edge.ya
		if u1 != u0 || v1 != v0 {
			crnrs++
		}
		u0 = u1
		v0 = v1
	}
	return crnrs
}

func sortEdges(edges []Edge) (int, error) {
	i := 0
	var found bool
	start := 0
	ncrnrs := 0
	for i < len(edges)-1 {
		x0, y0 := edges[i].xb, edges[i].yb
		found = false
		for j := i + 1; j < len(edges); j++ {
			x1, y1 := edges[j].xa, edges[j].ya
			if x0 == x1 && y0 == y1 {
				edges[i+1], edges[j] = edges[j], edges[i+1]
				found = true
				break
			}
		}
		if !found {
			return 0, nil
		}
		i++
		if edges[start].xa == edges[i].xb && edges[start].ya == edges[i].yb {
			ncrnrs += countCorners(edges[start : i+1])
			i++
			start = i

		}
	}
	return ncrnrs, nil
}

func computeAreaPerimeter(grid [][]rune, visited [][]bool, crop rune, x, y int, area, perimeter *int, edges *[]Edge) {
	if visited[y][x] || grid[y][x] != crop {
		return
	}
	ny := len(grid)
	nx := len(grid[0])
	visited[y][x] = true
	*area++
	shiftX := []int{1, 0, -1, 0} // Right, Up, Left, Down
	shiftY := []int{0, -1, 0, 1}
	var xnew, ynew int
	for i := range 4 {
		xnew = x + shiftX[i]
		ynew = y + shiftY[i]
		if xnew < 0 || xnew >= nx || ynew < 0 || ynew >= ny || grid[ynew][xnew] != crop {
			*perimeter++
			edge, _ := getEdge(x, y, i)
			*edges = append(*edges, edge)
			continue
		}
		computeAreaPerimeter(grid, visited, crop, xnew, ynew, area, perimeter, edges)
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

	var plots [][]rune
	var visited [][]bool
	ny := 0

	for fileScanner.Scan() {
		line := fileScanner.Text()
		row := make([]rune, len(line))
		for x, c := range line {
			row[x] = c
		}
		plots = append(plots, row)
		visited = append(visited, make([]bool, len(row)))
		ny++
	}
	if err := fileScanner.Err(); err != nil {
		log.Fatal(err)
		return
	}

	nx := len(plots[0])
	var area, perimeter int
	var edges []Edge
	part1 := 0
	part2 := 0
	for y := range ny {
		for x := range nx {
			if visited[y][x] {
				continue
			}
			area = 0
			perimeter = 0
			edges = make([]Edge, 0)
			computeAreaPerimeter(plots, visited, plots[y][x], x, y, &area, &perimeter, &edges)
			ncrnrs, err := sortEdges(edges)
			if err != nil {
				fmt.Println("Error sorting edges:", err)
				return
			}
			part1 += area * perimeter
			part2 += ncrnrs * area
		}
	}

	fmt.Println("Part 1:", part1)
	fmt.Println("Part 2:", part2)
}
