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

func processFile(filename string) (int, int) {
    file, err := os.Open(filename)
    handleError(err)
    defer file.Close()

    scanner := bufio.NewScanner(file)
    totalPart1 := 0
    totalPart2 := 0

    for scanner.Scan() {
        var start1, end1, start2, end2 int
        line := scanner.Text()
        fmt.Sscanf(line, "%d-%d,%d-%d", &start1, &end1, &start2, &end2)

        if (start2 >= start1 && end2 <= end1) || (start2 <= start1 && end2 >= end1) {
            totalPart1++
        }

        if !(start2 > end1 || start1 > end2) {
            totalPart2++
        }
    }

    handleError(scanner.Err())
    return totalPart1, totalPart2
}

func main() {
    if len(os.Args) != 2 {
        fmt.Println("Usage: provide a filename")
        os.Exit(1)
    }

    filename := os.Args[1]
    totalPart1, totalPart2 := processFile(filename)

    fmt.Printf("Part 1: %d\n", totalPart1)
    fmt.Printf("Part 2: %d\n", totalPart2)
}
