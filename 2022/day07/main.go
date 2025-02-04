package main

import (
    "bufio"
    "fmt"
    "log"
    "os"
    "strings"
    "strconv"
)

type File struct {
    size int
    name string
}

type Directory struct {
    name string
    files []File
    parent *Directory
    children []*Directory
}

func handleError(err error) {
    if err != nil {
        log.Fatalf("Error: %v", err)
    }
}

func createDirectory(name string, parent *Directory) *Directory {
    return &Directory{
        name: name,
        files: make([]File, 0),
        parent: parent,
        children: make([]*Directory, 0),
    }
}

func (d *Directory) addChild(input string) error {
    splitted := strings.Split(input, " ")
    if splitted[0] == "dir" {
        child := createDirectory(splitted[1], d)
        d.children = append(d.children, child)
    } else {
        size, err := strconv.Atoi(splitted[0])
        handleError(err)
        d.files = append(d.files, File{size: size, name: splitted[1]})
    }
    return nil
}

func (d *Directory) getDirectory(name string) *Directory {
    if name == ".." {
        return d.parent
    }
    for i,child := range(d.children) {
        if child.name == name {
            return d.children[i]
        }
    }
    return nil
}

func (d *Directory) getSize() int {
    total := 0
    for _, file := range(d.files) {
        total += file.size
    }
    for _, c := range(d.children) {
        total += c.getSize()
    }
    return total
}


func (d *Directory) getDirectoryWthSizeAtMost(maxSize int) int {
    total := 0
    dirSize := d.getSize()
    if dirSize <= maxSize {
        total += dirSize
    }
    for _, c := range(d.children) {
        total += c.getDirectoryWthSizeAtMost(maxSize)
    }
    return total
}

func minimalDirectorySize(d *Directory, target int, freed *int) {
    space := d.getSize()
    if space < target {
        return
    }
    *freed = min(*freed, space)
    for _,c  := range(d.children) {
        minimalDirectorySize(c, target, freed)
    }
}

func processFile(filename string) *Directory {
    file, err := os.Open(filename)
    handleError(err)
    defer file.Close()

    scanner := bufio.NewScanner(file)
    root := createDirectory("/", nil)
    current := root
    firstLine := true

    for scanner.Scan() {
        line := scanner.Text()
        if firstLine {
            firstLine = false
            continue
        }
        splitted := strings.Split(line, " ")
        if splitted[0] == "$" {
            if splitted[1] == "ls" {
                continue
            } else if splitted[1] == "cd" {
                current = current.getDirectory(splitted[2])
            } else {
                log.Panicf("Uknown command %s", line)
            }
        } else {
            current.addChild(line)
        }
    }

    handleError(scanner.Err())

    return root
}

func main() {
    if len(os.Args) != 2 {
        fmt.Println("Usage: provide a filename")
        os.Exit(1)
    }

    filename := os.Args[1]
    root := processFile(filename)

    part1 := root.getDirectoryWthSizeAtMost(100000)
    fmt.Println("Part 1", part1)

    totalDiskSpace := 70000000
    requiredSpace := 30000000
    usedSpace := root.getSize()
    unusedSpace := totalDiskSpace - usedSpace
    target := requiredSpace - unusedSpace
    part2 := 2*usedSpace
    minimalDirectorySize(root, target, &part2)
    fmt.Println("Part 2", part2)
}
