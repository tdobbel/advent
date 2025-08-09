package main

import (
	"bufio"
	"fmt"
	"log"
	"os"
	"sort"
	"strconv"
	"strings"
)

func main() {

	filePath := os.Args[1]
	readFile, err := os.Open(filePath)

	if err != nil {
		fmt.Println(err)
		return
	}
	defer readFile.Close()
	fileScanner := bufio.NewScanner(readFile)
	fileScanner.Split(bufio.ScanLines)
	var lhs []int
	var rhs []int
	cntr := make(map[int]int)

	for fileScanner.Scan() {
		splitted := strings.Fields(fileScanner.Text())
		left, err := strconv.Atoi(splitted[0])
		if err != nil {
			log.Fatal(err)
			return
		}
		lhs = append(lhs, left)
		right, err := strconv.Atoi(splitted[1])
		if err != nil {
			log.Fatal(err)
			return
		}
		rhs = append(rhs, right)
		cntr[right] += 1
	}

	if err := fileScanner.Err(); err != nil {
		log.Fatal(err)
		return
	}

	sort.Ints(lhs)
	sort.Ints(rhs)

	part1 := 0
	part2 := 0

	for i, left := range lhs {
		right := rhs[i]
		if right > left {
			part1 += right - left
		} else {
			part1 += left - right
		}
		part2 += cntr[left] * left
	}

	fmt.Println("Part 1:", part1)
	fmt.Println("Part 2:", part2)
}
