package main

import (
	"bufio"
	"fmt"
	"log"
	"os"
	"regexp"
	"strconv"
	"strings"
)

func main() {

	if len(os.Args) != 2 {
		fmt.Println("Please provide a filename")
		os.Exit(1)
	}

	re := regexp.MustCompile(`mul\(([0-9]+),([0-9]+)\)`)

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

	activated := true

	for fileScanner.Scan() {
		parts := strings.Split(fileScanner.Text(), "()")
		for _, part := range parts {
			groups := re.FindAllStringSubmatch(part, -1)
			for _, group := range groups {
				x, err := strconv.Atoi(group[1])
				if err != nil {
					log.Fatal(err)
					return
				}
				y, err := strconv.Atoi(group[2])
				if err != nil {
					log.Fatal(err)
					return
				}
				product := x * y
				part1 += product
				if activated {
					part2 += product
				}
				
			}
			n := len(part)
			if n >= 2 && strings.Compare(part[n-2:], "do") == 0 {
				activated = true
			}
			if n >= 5 && strings.Compare(part[n-5:], "don't") == 0 {
				activated = false
			}
		}

	}

	if err := fileScanner.Err(); err != nil {
		log.Fatal(err)
		return
	}

	fmt.Println("Part 1:", part1)
	fmt.Println("Part 2:", part2)
}
