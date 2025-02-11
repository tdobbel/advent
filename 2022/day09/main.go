package main

import (
    "bufio"
    "fmt"
    "log"
    "os"
    "strings"
    "strconv"
)

type Motion struct {
    direction rune
    steps int
}

type Position struct {
    x, y int
}

type Rope struct {
    knots []Position
}

func abs(x int) int {
    if x < 0 { return -x }
    return x
}

func createRope(n int) Rope {
    return Rope{ make([]Position, n) }
}

func handleError(err error) {
    if err != nil {
        log.Fatalf("Error: %v", err)
    }
}

func distance(p1, p2 Position) int {
    return max(abs(p1.x - p2.x), abs(p1.y - p2.y))
}

func directionToVec(direction rune) (int, int) {
    dx := 0
    dy := 0
    if direction == 'U' {
        dy = -1
    } else if direction == 'D' {
        dy = 1
    } else if direction == 'L' {
        dx = -1
    } else if direction == 'R' {
        dx = 1
    } else {
        panic("Unknown direction")
    }
    return dx, dy
}

func (r *Rope) move(motion Motion, visited map[Position]int) {
    for i := 0; i < motion.steps; i++ {
        dx, dy := directionToVec(motion.direction)
        r.knots[0].x += dx
        r.knots[0].y += dy
        for j := 1; j < len(r.knots); j++ {
            if distance(r.knots[j-1], r.knots[j]) <= 1 {
                break
            }
            dx := r.knots[j-1].x - r.knots[j].x
            dy := r.knots[j-1].y - r.knots[j].y
            if abs(dx) > 0 {
                dx /= abs(dx)
            }
            if abs(dy) > 0 {
                dy /= abs(dy)
            }
            r.knots[j].x += dx
            r.knots[j].y += dy
        }
        tail := r.knots[len(r.knots)-1]
        visited[tail]++
    }
}

func processFile(filename string) []Motion {
    file, err := os.Open(filename)
    handleError(err)
    defer file.Close()

    scanner := bufio.NewScanner(file)
    motions := make([]Motion, 0)

    for scanner.Scan() {
        line := scanner.Text()
        split := strings.Split(line, " ")
        steps, err := strconv.Atoi(split[1])
        handleError(err)
        motion := Motion{
            direction: rune(split[0][0]),
            steps: steps,
        }
        motions = append(motions, motion)
    }

    handleError(scanner.Err())

    return motions
}

func main() {
    if len(os.Args) != 2 {
        fmt.Println("Usage: provide a filename")
        os.Exit(1)
    }
    filename := os.Args[1]
    rope1 := createRope(2)
    rope2 := createRope(10)
    visited1 := make(map[Position]int)
    visited2 := make(map[Position]int)
    motions := processFile(filename)
    for _, motion := range(motions) {
        rope1.move(motion, visited1)
        rope2.move(motion, visited2)
    }
    fmt.Println("Part 1:", len(visited1))
    fmt.Println("Part 2:", len(visited2))
}
