package main

import (
	"bufio"
	"fmt"
	"log"
	"os"
	"strconv"
	// "strings"
)

type Block struct {
	values   []int
	capacity int
	size     int
}

func printBlocks(blocks []Block) {
	for _, block := range blocks {
		for i, value := range block.values {
			if i < block.size {
				fmt.Print(value)
			} else {
				fmt.Print(".")
			}
		}
	}
	fmt.Println()
}

func buildBlocks(line string) ([]Block, error) {
	var blocks []Block
	var value int
	for i, char := range line {
		capacity, err := strconv.Atoi(string(char))
		if err != nil {
			return nil, err
		}
		values := make([]int, capacity)
		size := 0
		if i%2 == 0 {
			value = i / 2
			for j := range capacity {
				values[j] = value
			}
			size = capacity
		}
		blocks = append(blocks, Block{values, capacity, size})
	}
	return blocks, nil
}

func checkSum(blocks []Block) int {
	sum := 0
	i := 0
	for _, block := range blocks {
		for j, value := range block.values {
			if j < block.size {
				sum += i * value
			}
			i++
		}
	}
	return sum
}

func compactMethod1(blocks []Block) {
	left := 1
	right := len(blocks) - 1
	if right%2 == 1 {
		right--
	}
	var il, ir int
	var blockL, blockR *Block
	for left < right {
		blockL = &blocks[left]
		blockR = &blocks[right]
		for blockL.size < blockL.capacity && blockR.size > 0 {
			il = blockL.size
			ir = blockR.size - 1
			blockL.values[il] = blockR.values[ir]
			blockL.size++
			blockR.size--
		}
		if blockL.size == blockL.capacity {
			left += 2
		}
		if blockR.size == 0 {
			right -= 2
		}
	}
}

func compactMethod2(blocks []Block) {
	right := len(blocks) - 1
	if right%2 == 1 {
		right--
	}
	var left, freeSpace int
	var blockL, blockR *Block
	for right > 0 {
		blockR = &blocks[right]
		left = 1
		for left < right {
			blockL = &blocks[left]
			freeSpace = blockL.capacity - blockL.size
			if freeSpace >= blockR.size {
				for i, value := range blockR.values {
					blockL.values[blockL.size+i] = value
				}
				blockL.size += blockR.size
				blockR.size = 0
				break
			}
			left += 2
		}
		right -= 2
	}
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

	var line string

	for fileScanner.Scan() {
		line = fileScanner.Text()
	}
	if err := fileScanner.Err(); err != nil {
		log.Fatal(err)
		return
	}

	blocks, err := buildBlocks(line)
	if err != nil {
		log.Fatal(err)
		return
	}
	compactMethod1(blocks)
	part1 := checkSum(blocks)

	blocks, err = buildBlocks(line)
	if err != nil {
		log.Fatal(err)
		return
	}
	compactMethod2(blocks)
	part2 := checkSum(blocks)

	fmt.Println("Part 1:", part1)
	fmt.Println("Part 2:", part2)

}
