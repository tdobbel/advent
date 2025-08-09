package main

import (
	"bufio"
	"fmt"
	"log"
	"os"
)

func isXmas(word []rune) bool {
	wordStr := string(word)
	return wordStr == "XMAS" || wordStr == "SAMX"
}

func isMas(word []rune) bool {
	wordStr := string(word)
	return wordStr == "MAS" || wordStr == "SAM"
}

func countXmas(grid []string) int {
	nx := len(grid[0])
	ny := len(grid)
	word := [4]rune{}
	count := 0
	for i := range ny {
		for j := range nx {
			if j < nx-3 {
				for k := range 4 {
					word[k] = rune(grid[i][j+k])
				}
				if isXmas(word[:]) {
					count++
				}
			}
			if i < ny-3 {
				for k := range 4 {
					word[k] = rune(grid[i+k][j])
				}
				if isXmas(word[:]) {
					count++
				}
			}
			if i < ny-3 && j < nx-3 {
				for k := range 4 {
					word[k] = rune(grid[i+k][j+k])
				}
				if isXmas(word[:]) {
					count++
				}
			}
			if i < ny-3 && j >= 3 {
				for k := range 4 {
					word[k] = rune(grid[i+k][j-k])
				}
				if isXmas(word[:]) {
					count++
				}
			}
		}
	}
	return count
}

func countCrossedMas(grid []string) int {
	nx := len(grid[0])
	ny := len(grid)
	diag1 := [3]rune{}
	diag2 := [3]rune{}
	count := 0
	for i := range ny - 2 {
		for x := range nx - 2 {
			for k := range 3 {
				diag1[k] = rune(grid[i+k][x+k])
				diag2[k] = rune(grid[i+k][x+2-k])
			}
			if isMas(diag1[:]) && isMas(diag2[:]) {
				count++
			}
		}
	}
	return count
}

func main() {

	if len(os.Args) != 2 {
		fmt.Println("Please provide a filename")
		os.Exit(1)
	}

	filePath := os.Args[1]
	readFile, err := os.Open(filePath)

	if err != nil {
		fmt.Println(err)
		return
	}
	defer readFile.Close()
	fileScanner := bufio.NewScanner(readFile)
	fileScanner.Split(bufio.ScanLines)

	var grid []string

	for fileScanner.Scan() {
		grid = append(grid, fileScanner.Text())
	}

	if err := fileScanner.Err(); err != nil {
		log.Fatal(err)
		return
	}

	part1 := countXmas(grid)
	part2 := countCrossedMas(grid)

	fmt.Printf("Part 1: %d\n", part1)
	fmt.Printf("Part 2: %d\n", part2)

}
