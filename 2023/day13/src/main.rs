use std::cmp::min;
use std::collections::HashMap;
use std::env;
use std::fs::File;
use std::io::{BufRead, BufReader};

fn find_symmetry(grid: &[Vec<char>], target_err: usize) -> Option<usize> {
    let mut xs: Vec<usize> = (1..grid[0].len()).collect();
    let mut nwrong: HashMap<usize, usize> = HashMap::new();
    for row in grid.iter() {
        let mut x_next = vec![];
        for x in xs.iter() {
            let n = min(*x, row.len() - *x);
            let nerr = nwrong.entry(*x).or_insert(0);
            for i in 0..n {
                if row[x - i - 1] != row[x + i] {
                    *nerr += 1;
                }
                if *nerr > target_err {
                    break;
                }
            }
            if *nerr <= target_err {
                x_next.push(*x);
            }
        }
        xs = x_next;
        if xs.is_empty() {
            return None;
        }
    }
    for (k, v) in nwrong.iter() {
        if *v == target_err {
            return Some(*k);
        }
    }
    None
}

fn transpose(grid: &[Vec<char>]) -> Vec<Vec<char>> {
    let mut new_grid = vec![];
    for i in 0..grid[0].len() {
        let mut row = vec![];
        for j in 0..grid.len() {
            row.push(grid[j][i]);
        }
        new_grid.push(row);
    }
    new_grid
}

fn get_symmetry_value(grid: &[Vec<char>], target_err: usize) -> usize {
    match find_symmetry(grid, target_err) {
        Some(i) => i,
        None => {
            let transposed = transpose(grid);
            match find_symmetry(&transposed, target_err) {
                Some(i) => i * 100,
                None => panic!("No symmetry found"),
            }
        }
    }
}

fn main() {
    let args = env::args().nth(1).expect("Please provide an input file");
    let file = File::open(args).unwrap();
    let reader = BufReader::new(file);
    let mut grid: Vec<Vec<char>> = vec![];
    let mut total1 = 0;
    let mut total2 = 0;
    for line in reader.lines() {
        let line = line.unwrap();
        if line.is_empty() {
            total1 += get_symmetry_value(&grid, 0);
            total2 += get_symmetry_value(&grid, 1);
            grid = vec![];
            continue;
        }
        grid.push(line.chars().collect());
    }
    if !grid.is_empty() {
        total1 += get_symmetry_value(&grid, 0);
        total2 += get_symmetry_value(&grid, 1);
    }
    println!("Part 1: {}", total1);
    println!("Part 2: {}", total2);
}
