package main

import (
	"bufio"
	"fmt"
	"log"
	"os"
	"slices"
	"sort"
	"strconv"
	"strings"
)

func checkOrdering(nums []int, ordering map[int][]int) bool {
	for i := range len(nums) - 1 {
		lowerNums := ordering[nums[i]]
		for j := i + 1; j < len(nums); j++ {
			if slices.Contains(lowerNums, nums[j]) {
				return false
			}
		}
	}
	return true
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

	readingOrder := true
	numLowerThan := make(map[int][]int)

	part1 := 0
	part2 := 0

	for fileScanner.Scan() {
		line := fileScanner.Text()
		if len(line) == 0 {
			readingOrder = false
			continue
		}
		if readingOrder {
			nums := strings.Split(line, "|")
			lower, err := strconv.Atoi(nums[0])
			if err != nil {
				log.Fatal(err)
				return
			}
			greater, err := strconv.Atoi(nums[1])
			if err != nil {
				log.Fatal(err)
				return
			}
			numLowerThan[greater] = append(numLowerThan[greater], lower)
		} else {
			var nums []int
			for _, numStr := range strings.Split(line, ",") {
				num, err := strconv.Atoi(strings.TrimSpace(numStr))
				if err != nil {
					log.Fatal(err)
					return
				}
				nums = append(nums, num)
			}
			n := len(nums)
			if checkOrdering(nums, numLowerThan) {
				part1 += nums[n/2]
			} else {
				sort.SliceStable(nums, func(i, j int) bool {
					return slices.Contains(numLowerThan[nums[j]], nums[i])
				})
				part2 += nums[n/2]
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
