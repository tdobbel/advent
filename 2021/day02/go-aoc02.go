package main

import (
	"bufio"
	"fmt"
	"log"
	"os"
	"strconv"
	"strings"
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

	x := 0
	y := 0
	aim := 0


	for fileScanner.Scan() {
		line := fileScanner.Text()
		splitted := strings.Split(line, " ")
		n, err := strconv.Atoi(splitted[1])
		if err != nil {
			log.Fatal(err)
			return
		}
		if splitted[0] == "forward" {
			x += n
			y += aim * n
		} else if splitted[0] == "up" {
			aim -= n
		} else if splitted[0] == "down" {
			aim += n
		} else {
			log.Fatal("Unknown direction")
			return
		}
	}

	if err := fileScanner.Err(); err != nil {
		log.Fatal(err)
		return
	}

	fmt.Println("Part 1:", x * aim)
	fmt.Println("Part 2:", x * y)
}
