package main

import (
    "bufio"
    "fmt"
    "log"
    "os"
)

type Move struct {
    n, from, to int
}

func handleError(err error) {
    if err != nil {
        log.Fatalf("Error: %v", err)
    }
}

func moveFromStr(s string) Move {
    var n, from, to int
    fmt.Sscanf(s, "move %d from %d to %d", &n, &from, &to)
    return Move{n, from - 1, to - 1}
}

func reverse(stack []rune) {
    n := len(stack)
    for i := 0; i < n/2; i++ {
        stack[i], stack[n-i-1] = stack[n-i-1], stack[i]
    }
}

func processFile(filename string) ([][]rune, []Move) {
    file, err := os.Open(filename)
    handleError(err)
    defer file.Close()

    scanner := bufio.NewScanner(file)
    var stacks [][]rune
    var moves []Move
    is_stack := true

    for scanner.Scan() {
        line := scanner.Text()
        if len(line) == 0 {
            continue
        }
        if !is_stack {
            moves = append(moves, moveFromStr(line))
            continue
        }
        for i := 0; i < len(line); i += 4 {
            if i/4 >= len(stacks) {
                stacks = append(stacks, []rune{})
            }
            crate := line[i : i+3]
            if rune(crate[1]) == '1' {
                is_stack = false
                for _, stack := range stacks {
                    reverse(stack)
                }
                break
            }
            if rune(crate[0]) == ' ' {
                continue
            }
            stacks[i/4] = append(stacks[i/4], rune(crate[1]))
        }
    }

    handleError(scanner.Err())
    return stacks, moves
}

func pop(stack *[]rune) rune {
    n := len(*stack)
    if n == 0 {
        panic("pop on empty stack")
    }
    last := (*stack)[n-1]
    *stack = (*stack)[:n-1]
    return last
}

func clone_crates(stack [][]rune) [][]rune {
    res := make([][]rune, len(stack))
    for i, s := range stack {
        res[i] = make([]rune, len(s))
        copy(res[i], s)
    }
    return res
}

func move_crates(stack_from *[]rune, stack_to *[]rune, n int) {
    end := len(*stack_from)
    crates := (*stack_from)[end-n:]
    *stack_from = (*stack_from)[:end-n]
    *stack_to = append(*stack_to, crates...)
}

func print_stacks(stacks [][]rune) {
    for _, stack := range stacks {
        fmt.Print(string(stack[len(stack)-1]))
    }
    fmt.Println()
}

func main() {
    if len(os.Args) != 2 {
        fmt.Println("Usage: provide a filename")
        os.Exit(1)
    }

    filename := os.Args[1]
    stacks, moves := processFile(filename)
    stacks2 := clone_crates(stacks)

    for _, move := range moves {
        move_crates(&stacks2[move.from], &stacks2[move.to], move.n)
        for i := 0; i < move.n; i++ {
            crate := pop(&stacks[move.from])
            stacks[move.to] = append(stacks[move.to], crate)
        }
    }

    fmt.Print("Part 1: ")
    print_stacks(stacks)
    fmt.Print("Part 2: ")
    print_stacks(stacks2)
}
