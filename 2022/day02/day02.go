package main

import (
    "bufio"
    "fmt"
    "os"
    "log"
    "strings"
)

type Pair struct {
    x, y string
}

func check(e error) {
    if e != nil {
        panic(e)
    }
}

func getOucome(x, y int) int {
    if x == y { return 1 }
    if y == x%3+1 { return 2 }
    return 0
}

func getMoveY(x, expected int) int {
    if expected == 1 { return x }
    if expected == 2 { return x%3+1 }
    if x == 1 { return 3 }
    return x-1
}

func main() {

    player1 := map[string]int{
        "A": 1,
        "B": 2,
        "C": 3,
    }
    player2 := map[string]int{
        "X": 1,
        "Y": 2,
        "Z": 3,
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
    var pairs []Pair
    for scanner.Scan() {
        line := scanner.Text()
        moves := strings.Fields(line)
        if len(moves) != 2 {
            panic("Expected 2 moves")
        }
        pairs = append(pairs, Pair{moves[0], moves[1]})
    }

    if err := scanner.Err(); err != nil {
        log.Fatal(err)
    }

    score1 := 0
    for _, pair := range pairs {
        result := getOucome(player1[pair.x], player2[pair.y])
        score1 += player2[pair.y] + 3*result
    }
    fmt.Println("Part 1:", score1)

    expected := map[string]int{
        "X": 0,
        "Y": 1,
        "Z": 2,
    }
    score2 := 0
    for _, pair := range pairs {
        result := expected[pair.y]
        score2 += getMoveY(player1[pair.x], result) + 3*result
    }
    fmt.Println("Part 2:", score2)


}
