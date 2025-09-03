package main

import (
	"bufio"
	"fmt"
	"log"
	"os"
	"slices"
)

const (
	Wall = iota
	Box
)

const (
	Up    rune = '^'
	Right      = '>'
	Down       = 'v'
	Left       = '<'
)

type ObjectBase struct {
	x, y, objectType int
}

type Object struct {
	x, y, objectType, width int
}

type Warehouse struct {
	objects []Object
	grid    [][]int
}

func (w *Warehouse) summedGpsScore() int {
	score := 0
	for _, obj := range w.objects {
		if obj.objectType == Box {
			score += 100*obj.y + obj.x
		}
	}
	return score
}

func (w *Warehouse) findMovedObjects(x, y, dx, dy int, moved *[]int, blocked *bool) {
	if *blocked {
		return
	}
	iobj := w.grid[y][x] - 1
	if iobj < 0 {
		return
	}
	object := w.objects[iobj]
	if object.objectType == Wall {
		*blocked = true
		return
	}
	if slices.Contains(*moved, iobj) {
		return
	}
	*moved = append(*moved, iobj)
	if dx == 0 {
		for iw := range object.width {
			w.findMovedObjects(object.x+iw, object.y+dy, dx, dy, moved, blocked)
		}
	} else if dx < 0 {
		w.findMovedObjects(object.x+dx, object.y, dx, dy, moved, blocked)
	} else {
		w.findMovedObjects(object.x+dx*object.width, object.y, dx, dy, moved, blocked)
	}
}

func (w *Warehouse) updateGrid(movedObjects []int, dx, dy int) {
	for _, iobj := range movedObjects {
		object := &w.objects[iobj]
		for iw := range object.width {
			w.grid[object.y][object.x+iw] = 0
		}
		object.x += dx
		object.y += dy
	}
	for _, iobj := range movedObjects {
		object := w.objects[iobj]
		for iw := range object.width {
			w.grid[object.y][object.x+iw] = iobj + 1
		}
	}
}

func (w *Warehouse) show(robotX, robotY int) {
	ny := len(w.grid)
	nx := len(w.grid[0])
	for y := range ny {
		for x := range nx {
			if x == robotX && y == robotY {
				fmt.Print("@")
				continue
			}
			iobj := w.grid[y][x] - 1
			if iobj < 0 {
				fmt.Print(".")
				continue
			}
			object := w.objects[iobj]
			if object.objectType == Wall {
				fmt.Print("#")
			} else if object.width == 1 {
				fmt.Print("O")
			} else if x == object.x {
				fmt.Print("[")
			} else {
				fmt.Print("]")
			}
		}
		fmt.Println()
	}
}

func (w *Warehouse) moveObjects(robotX, robotY *int, move rune) {
	dx, dy := 0, 0
	switch move {
	case Up:
		dy = -1
	case Right:
		dx = 1
	case Down:
		dy = 1
	default:
		dx = -1
	}
	x := *robotX + dx
	y := *robotY + dy
	var moved []int
	blocked := false
	w.findMovedObjects(x, y, dx, dy, &moved, &blocked)
	if blocked {
		return
	}
	w.updateGrid(moved, dx, dy)
	*robotX = x
	*robotY = y
}

func createWarehouse(objectPos []ObjectBase, nx, ny, width int) Warehouse {
	grid := make([][]int, ny)
	for i := range grid {
		grid[i] = make([]int, nx*width)
	}

	objects := make([]Object, len(objectPos))
	for i, obj := range objectPos {
		objects[i] = Object{x: obj.x * width, y: obj.y, objectType: obj.objectType, width: width}
		for w := range width {
			grid[obj.y][objects[i].x+w] = i + 1
		}
	}

	return Warehouse{objects: objects, grid: grid}
}

func solvePuzzle(startX, startY int, objectPos []ObjectBase, moves []rune, nx, ny, width int) int {
	robotX := startX * width
	robotY := startY
	warehouse := createWarehouse(objectPos, nx, ny, width)
	// warehouse.show(robotX, robotY)
	for _, move := range moves {
		// fmt.Printf("%c\n", move)
		warehouse.moveObjects(&robotX, &robotY, move)
		// warehouse.show(robotX, robotY)
	}
	return warehouse.summedGpsScore()
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

	parseGrid := true
	ny := 0
	nx := 0
	var objectPositions []ObjectBase
	var moves []rune
	robotX, robotY := 0, 0

	for fileScanner.Scan() {
		line := fileScanner.Text()
		if len(line) == 0 {
			parseGrid = false
			continue
		}
		if parseGrid {
			nx = len(line)
			for x, char := range line {
				switch char {
				case '.':
					continue
				case '#':
					wall := ObjectBase{x: x, y: ny, objectType: Wall}
					objectPositions = append(objectPositions, wall)
				case 'O':
					box := ObjectBase{x: x, y: ny, objectType: Box}
					objectPositions = append(objectPositions, box)
				case '@':
					robotX, robotY = x, ny

				}
			}
			ny++
		} else {
			for _, c := range line {
				if c == Up || c == Right || c == Down || c == Left {
					moves = append(moves, c)
				} else {
					log.Fatalf("Unknown move")
					return
				}
			}
		}

	}

	if err := fileScanner.Err(); err != nil {
		log.Fatal(err)
		return
	}

	part1 := solvePuzzle(robotX, robotY, objectPositions, moves, nx, ny, 1)
	fmt.Println("Part 1: ", part1)
	part2 := solvePuzzle(robotX, robotY, objectPositions, moves, nx, ny, 2)
	fmt.Println("Part 2: ", part2)

}
