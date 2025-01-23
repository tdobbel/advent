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

func findCommon(left, right string) rune {
    for _, l := range left {
        if strings.ContainsRune(right, l) {
            return l
        }
    }
    panic("No common character found")
}

func findCommon3(str1, str2, str3 string) rune {
    for _, c := range str1 {
        if strings.ContainsRune(str2, c) && strings.ContainsRune(str3, c) {
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
            total2 += priority[findCommon3(group[0], group[1], group[2])]
            group = []string{}
        }
    }

    if err := scanner.Err(); err != nil {
        log.Fatal(err)
    }

    fmt.Println("Part 1:", total1)
    fmt.Println("Part 2:", total2)

}
