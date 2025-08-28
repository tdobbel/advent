package main

import (
	"bufio"
	"fmt"
	"log"
	"os"
	"strconv"
	"strings"
)

func mix(x, y uint64) uint64 {
	return x ^ y
}

func prune(x uint64) uint64 {
	return x & ((1 << 24) - 1)
}

func getNextNumber(x uint64) uint64 {
	res := prune(mix(x<<6, x))
	res = prune(mix(res>>5, res))
	res = prune(mix(res<<11, res))
	return res
}

func decodeKey(key uint32) [4]int {
	res := [4]int{0, 0, 0, 0}
	for i := range 4 {
		res[3-i] = (int(key) & 0x1F) - 9
		key = key >> 5
	}
	return res
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

	var part1 uint64 = 0
	var price, prev int
	var key uint32
	var gains = make(map[uint32]int)

	for fileScanner.Scan() {
		line := fileScanner.Text()
		secret, err := strconv.ParseUint(strings.TrimSpace(line), 10, 64)
		if err != nil {
			log.Fatal(err)
			return
		}
		var buyer = make(map[uint32]int)
		key = 0
		prev = int(secret % 10)
		for i := range 2000 {
			secret = getNextNumber(secret)
			price = int(secret % 10)
			key = ((key << 5) | uint32(price-prev+9)) & 0xFFFFF
			prev = price
			if i >= 3 && buyer[key] == 0 {
				buyer[key] = price
			}

		}
		part1 += secret
		for key, value := range buyer {
			gains[key] += value
		}
	}

	if err := fileScanner.Err(); err != nil {
		log.Fatal(err)
		return
	}

	part2 := 0
	var bestKey uint32 = 0
	for key, value := range gains {
		if value > part2 {
			part2 = value
			bestKey = key
		}
	}

	fmt.Println("Part 1:", part1)
	fmt.Printf("Part 2: %d with %v\n", part2, decodeKey(bestKey))
}
