package main

import (
    "bufio"
    "fmt"
    "log"
    "os"
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
    total1 := 0
    total2 := 0
    for scanner.Scan() {
        line := scanner.Text()
        var x0, y0, x1, y1 int
        fmt.Sscanf(line, "%d-%d,%d-%d", &x0, &y0, &x1, &y1)
        if (x1 >= x0 && y1 <= y0) || (x1 <= x0 && y1 >= y0) {
            total1 += 1
        }
        no_intersection := x1 > y0 || x0 > y1
        if !no_intersection {
            total2 += 1
        }
    }

    if err := scanner.Err(); err != nil {
        log.Fatal(err)
    }

    fmt.Println("Part 1:", total1)
    fmt.Println("Part 2:", total2)
}
