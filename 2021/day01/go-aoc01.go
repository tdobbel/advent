package main

import (
	"bufio"
	"fmt"
	"log"
	"os"
	"strconv"
)

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

	var part1 int = 0
	var depth, prev int
	var part2 int = 0
	var values = [3]int{0, 0, 0}
	var sum1, sum2 int
	iline := 0

	for fileScanner.Scan() {
		line := fileScanner.Text()
		depth, err = strconv.Atoi(line)
		if err != nil {
			log.Fatal(err)
			return
		}
		iline++
		if iline > 1 && depth > prev {
			part1++
		}
		prev = depth
		for i := range 3 {
			if i < iline {
				values[i] += depth
			}
		}
		if iline < 3 {
			continue
		}
		sum1 = sum2
		sum2 = values[0]
		for i := range 2 {
			values[i] = values[i+1]
		}
		values[2] = 0
		if iline > 4 && sum2 > sum1 {
			part2++
		}

	}

	if err := fileScanner.Err(); err != nil {
		log.Fatal(err)
		return
	}

	fmt.Println("Part 1:", part1)
	fmt.Println("Part 2:", part2)
}
