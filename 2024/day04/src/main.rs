use std::env;
use std::fs::File;
use std::io::{BufRead, BufReader};

fn isword(array: &[char], word: &str) -> bool {
    for (i, c) in word.chars().enumerate() {
        if array[i] != c {
            return false;
        }
    }
    true
}

fn is_xmas(word: &[char]) -> bool {
    isword(word, "XMAS") || isword(word, "SAMX")
}

fn part1(grid: &[Vec<char>]) -> i32 {
    let mut count = 0;
    let nrows = grid.len();
    let ncols = grid[0].len();
    let mut word: Vec<char> = Vec::with_capacity(4);
    for i in 0..nrows {
        for j in 0..ncols {
            if i <= nrows - 4 {
                word = (0..4).map(|k| grid[i + k][j]).collect();
                if is_xmas(&word) {
                    count += 1;
                }
            }
            if j <= ncols - 4 {
                word[..4].copy_from_slice(&grid[i][j..j + 4]);
                if is_xmas(&word) {
                    count += 1;
                }
            }
            if i <= nrows - 4 && j <= ncols - 4 {
                word = (0..4).map(|k| grid[i + k][j + k]).collect();
                if is_xmas(&word) {
                    count += 1;
                }
            }
            if j >= 3 && i <= nrows - 4 {
                word = (0..4).map(|k| grid[i + k][j - k]).collect();
                if is_xmas(&word) {
                    count += 1;
                }
            }
        }
    }
    count
}

fn is_mas(word: &[char]) -> bool {
    isword(word, "MAS") || isword(word, "SAM")
}

fn part2(grid: &[Vec<char>]) -> i32 {
    let mut count = 0;
    let mut diag1 = vec!['.'; 3];
    let mut diag2 = vec!['.'; 3];
    let nrows = grid.len();
    let ncols = grid[0].len();
    for i in 0..nrows - 2 {
        for j in 0..ncols - 2 {
            for k in 0..3 {
                diag1[k] = grid[i + k][j + k];
                diag2[k] = grid[i + k][j + 2 - k];
            }
            if is_mas(&diag1) && is_mas(&diag2) {
                count += 1;
            }
        }
    }
    count
}

fn main() {
    let args = env::args().nth(1).expect("Please provide an input file");
    let file = File::open(args).unwrap();
    let reader = BufReader::new(file);
    let mut grid: Vec<Vec<char>> = Vec::new();
    for line in reader.lines() {
        let line = line.unwrap();
        let row = line.chars().collect::<Vec<char>>();
        grid.push(row);
    }
    println!("Part 1: {}", part1(&grid));
    println!("Part 2: {}", part2(&grid));
}
