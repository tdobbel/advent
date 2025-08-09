package main

import (
	"bufio"
	"fmt"
	"log"
	"os"
	"strconv"
	"strings"
)

func isSafe(nums []int) bool {
	ascending := nums[1] > nums[0]
	for i := 1; i < len(nums); i++ {
		diff := nums[i] - nums[i-1]
		if !ascending {
			diff *= -1
		}
		if diff < 1 || diff > 3 {
			return false
		}
	}
	return true
}

func isSafeDampened(nums []int) bool {
	for iskip := range nums {
		var dampened []int
		for i := range nums {
			if i == iskip {
				continue
			}
			dampened = append(dampened, nums[i])
		}
		if isSafe(dampened) {
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
		line := strings.Fields(fileScanner.Text())
		var nums []int
		for _, numStr := range line {
			num, err := strconv.Atoi(numStr)
			if err != nil {
				log.Fatal(err)
				return
			}
			nums = append(nums, num)
		}
		if isSafe(nums) {
			part1++
			part2++
		} else if isSafeDampened(nums) {
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
