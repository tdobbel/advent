package main

import (
    "bufio"
    "fmt"
    "os"
    "log"
    "strconv"
    "sort"
)

func check(e error) {
    if e != nil {
        panic(e)
    }
}

func main() {
    if len(os.Args) != 2 {
        fmt.Println("Please provide a filename")
        os.Exit(1)
    }
    filename := os.Args[1]
    file, err := os.Open(filename)
    check(err)
    defer file.Close()
    scanner := bufio.NewScanner(file)
    var calories []int
    var value int = 0
    for scanner.Scan() {
        line := scanner.Text()
        if len(line) == 0 {
            calories = append(calories, value)
            value = 0
        } else {
            lineValue, err := strconv.Atoi(line)
            check(err)
            value += lineValue
        }
    }
    calories = append(calories, value)
    sort.Ints(calories)
    n := len(calories)

    if err := scanner.Err(); err != nil {
        log.Fatal(err)
    }

    fmt.Printf("Part 1: %d\n", calories[n-1])
    fmt.Printf("Part 2: %d\n", calories[n-1] + calories[n-2] + calories[n-3])
}
