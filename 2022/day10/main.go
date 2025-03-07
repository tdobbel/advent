package main

import (
    "bufio"
    "fmt"
    "log"
    "os"
)

func handleError(err error) {
    if err != nil {
        log.Fatalf("Error: %v", err)
    }
}

func processFile(filename string) []string {
    file, err := os.Open(filename)
    handleError(err)
    defer file.Close()

    scanner := bufio.NewScanner(file)
    lines := make([]string, 0)

    for scanner.Scan() {
        line := scanner.Text()
        lines = append(lines, line)
    }

    handleError(scanner.Err())

    return lines
}

func abs(x int) int {
    if x < 0 {
        return -x
    }
    return x
}

func updateSignalStrength(cycle, x int, cycle_numbers []int, r *int) {
    for _, cycle_ := range cycle_numbers {
        if cycle == cycle_ {
            *r += x * cycle_
            return
        }
    }
}

func executeProgram(program []string) int {
    x := 1
    r := 0
    cycle := 0
    cycle_numbers := []int{
        20, 60, 100, 140, 180, 220,
    }
    var n int
    for _, line := range program {
        cycle ++
        updateSignalStrength(cycle, x, cycle_numbers, &r)
        _, err := fmt.Sscanf(line, "addx %d", &n)
        if err == nil {
            cycle++
            updateSignalStrength(cycle, x, cycle_numbers, &r)
            x += n
        }
    }
    return r
}

func drawPixel(pixels *[6][40]rune, cycle, x int) {
    row := cycle / 40
    col := cycle % 40
    if abs(col - x) < 2 {
        pixels[row][col] = '#'
    } else {
        pixels[row][col] = '.'
    }
}

func generateImage(program []string) {
    x := 1
    var pixels [6][40]rune
    cycle := 0
    iline := 0
    var n int
    for cycle < 240 {
        line := program[iline]
        drawPixel(&pixels, cycle, x)
        _, err := fmt.Sscanf(line, "addx %d", &n)
        if err == nil {
            cycle++
            drawPixel(&pixels, cycle, x)
            x += n
        }
        cycle++
        iline++
    }
    for _, row := range pixels {
        fmt.Println(string(row[:]))
    }
}

func main() {
    if len(os.Args) != 2 {
        fmt.Println("Usage: provide a filename")
        os.Exit(1)
    }
    filename := os.Args[1]
    program := processFile(filename)
    part1 := executeProgram(program)
    fmt.Println("Part 1:", part1)
    generateImage(program)
}
