package main

import (
    "bufio"
    "fmt"
    "log"
    "os"
    "strings"
    "unicode"
)

func check(e error) {
    if e != nil {
        panic(e)
    }
}


func findCommon(args ...string) rune {
    var first = args[0]
    for _, c := range(first) {
        var common = true
        for _, s := range args[1:] {
            if !strings.ContainsRune(s, c) {
                common = false
                break
            }
        }
        if common {
            return c
        }
    }
    panic("No common character found")
}

func main() {
    priority := make(map[rune]int)
    i := 1
    for c := 'a'; c <= 'z'; c++ {
        priority[c] = i
        priority[unicode.ToUpper(c)] = 26 + i
        i++
    }

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
    var group []string
    for scanner.Scan() {
        line := scanner.Text()
        mid := len(line) / 2
        left := line[:mid]
        right := line[mid:]
        total1 += priority[findCommon(left, right)]
        group = append(group, line)
        if len(group) == 3 {
            total2 += priority[findCommon(group[0], group[1], group[2])]
            group = []string{}
        }
    }

    if err := scanner.Err(); err != nil {
        log.Fatal(err)
    }

    fmt.Println("Part 1:", total1)
    fmt.Println("Part 2:", total2)

}
