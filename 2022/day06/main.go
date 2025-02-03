package main

import (
	"bufio"
	"fmt"
	"log"
	"os"
	"strings"
)

func handleError(err error) {
    if err != nil {
        log.Fatalf("Error: %v", err)
    }
}

func findStartOfPacked(line string, size int, index *int) {
    for i := 0; i < len(line)-size; i++ {
        ok := true
        for j := 1; j < size; j++ {
            if strings.ContainsRune(line[i:i+j], rune(line[i+j])) {
                ok = false
                break
            }
        }
        if ok {
            *index = i+size
            return
        }
    }
    log.Fatal("Could not find start of packed string in line")
}

func main() {
    if len(os.Args) != 2 {
        fmt.Println("Usage: provide a filename")
        os.Exit(1)
    }

    filename := os.Args[1]
    file, err := os.Open(filename)
    handleError(err)
    defer file.Close()

    scanner := bufio.NewScanner(file)
    var part1 = -1
    var part2 = -1
    for scanner.Scan() {
        line := scanner.Text()
        findStartOfPacked(line, 4, &part1)
        findStartOfPacked(line, 14, &part2)
    }

    handleError(scanner.Err())

    fmt.Printf("Part 1: %d\n", part1)
    fmt.Printf("Part 2: %d\n", part2)
}
