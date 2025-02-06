package main

import (
    "bufio"
    "fmt"
    "log"
    "os"
    "strconv"
)

func handleError(err error) {
    if err != nil {
        log.Fatalf("Error: %v", err)
    }
}

func readGrid(filename string) [][]int {
    grid := make([][]int, 0)
    file, err := os.Open(filename)
    handleError(err)
    defer file.Close()

    scanner := bufio.NewScanner(file)
    for scanner.Scan() {
        line := scanner.Text()
        row := make([]int, len(line))
        for i,c := range(line) {
            row[i], _ = strconv.Atoi(string(c))
        }
        grid = append(grid, row)
    }

    handleError(scanner.Err())

    return grid
}

func isVisible(trees []int) []bool {
    visible := make([]bool, len(trees))
    maxHeight := -1
    for i,height := range(trees) {
        visible[i] = height > maxHeight
        maxHeight = max(maxHeight, height)
    }
    return visible
}

func reverse(array []int) []int {
    n := len(array)
    reversed := make([]int, n)
    for i := 0; i < len(array); i++ {
        reversed[i] = array[n-1-i]
    }
    return reversed
}

func part1(trees [][]int) int {
    visible := make([][]bool, len(trees))
    n_rows := len(trees)
    n_cols := len(trees[0])
    for i,rowTrees := range(trees) {
        visibleLeft := isVisible(rowTrees)
        visibleRight := isVisible(reverse(rowTrees))
        visible[i] = make([]bool, len(rowTrees))
        for j := 0; j < n_cols; j++ {
            visible[i][j] = visibleLeft[j] || visibleRight[n_cols-1-j]
        }
    }
    for col := 0; col < n_cols; col++ {
        colTrees := make([]int, len(trees))
        for row := 0; row < n_rows; row++ {
            colTrees[row] = trees[row][col]
        }
        visibleTop := isVisible(colTrees)
        visibleBottom := isVisible(reverse(colTrees))
        for row := 0; row < n_rows; row++ {
            visible[row][col] = visible[row][col] || visibleTop[row] || visibleBottom[n_rows-1-row]
        }
    }
    n_visible := 0
    for _,row := range(visible) {
        for _, v := range(row) {
            if v {
                n_visible++
            }
        }
    }
    return n_visible
}

func scenicScore(trees [][] int, i, j int) int {
    score := 1
    ny := len(trees)
    nx := len(trees[0])
    var x, y int
    dx := []int{0, 1, 0, -1}
    dy := []int{-1, 0, 1, 0}
    for k := 0; k < 4; k++ {
        factor := 0
        x = j + dx[k]
        y = i + dy[k]
        for {
            factor ++
            if trees[y][x] >= trees[i][j] {
                break
            }
            x += dx[k]
            y += dy[k]
            if y < 0 || y >= ny || x < 0 || x >= nx {
                break
            }
        }
        score *= factor
    }
    return score
}

func part2(trees [][]int) int {
    maxScore := 0
    for i := 1; i < len(trees)-1; i++ {
        for j := 1; j < len(trees[0])-1; j++ {
            maxScore = max(maxScore, scenicScore(trees, i, j))
        }
    }
    return maxScore
}

func main() {
    if len(os.Args) != 2 {
        fmt.Println("Usage: provide a filename")
        os.Exit(1)
    }

    filename := os.Args[1]
    grid := readGrid(filename)
    fmt.Println("Part 1:", part1(grid))
    fmt.Println("Part 2:", part2(grid))
}
