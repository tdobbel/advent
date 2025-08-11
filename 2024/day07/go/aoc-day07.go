package main

import (
	"bufio"
	"fmt"
	"log"
	"os"
	"strconv"
	"strings"
)

const (
	Sum int = iota
	Product
	Concatenate
)

func isSolvable(target, result int, numbers, ops []int) bool {
	if len(numbers) == 0 {
		return result == target
	}
	for _, op := range ops {
		var num int
		switch op {
		case Sum:
			num = result + numbers[0]
		case Product:
			num = result * numbers[0]
		case Concatenate:
			num, _ = strconv.Atoi(fmt.Sprintf("%d%d", result, numbers[0]))
		}
		if num > target {
			continue
		}
		if isSolvable(target, num, numbers[1:], ops) {
			return true
		}
	}
	return false
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

	part1 := 0
	part2 := 0

	for fileScanner.Scan() {
		line := fileScanner.Text()
		lineSplit := strings.Split(line, ":")
		var numbers []int
		target, err := strconv.Atoi(lineSplit[0])
		if err != nil {
			log.Fatal(err)
			return
		}
		numberStrs := strings.Fields(lineSplit[1])
		for _, numStr := range numberStrs {
			num, err := strconv.Atoi(numStr)
			if err != nil {
				log.Fatal(err)
				return
			}
			numbers = append(numbers, num)
		}
		if isSolvable(target, numbers[0], numbers[1:], []int{Sum, Product}) {
			part1 += target
			part2 += target
		} else if isSolvable(target, numbers[0], numbers[1:], []int{Sum, Product, Concatenate}) {
			part2 += target
		}
	}

	if err := fileScanner.Err(); err != nil {
		log.Fatal(err)
		return
	}

	fmt.Println("Part 1:", part1)
	fmt.Println("Part 2:", part2)

}
