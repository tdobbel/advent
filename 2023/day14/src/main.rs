use std::env;
use std::fs::File;
use std::io::{BufRead, BufReader};

#[allow(dead_code)]
fn plot_grid(grid: &[Vec<char>]) {
    for row in grid {
        for c in row {
            print!("{}", c);
        }
        println!();
    }
}

fn flip(grid: &mut [Vec<char>]) {
    let ny = grid.len();
    let nx = grid[0].len();
    for j in 0..nx {
        let mut floor = 0;
        while grid[floor][j] == '#' {
            floor += 1;
        }
        for i in 0..ny {
            match grid[i][j] {
                'O' => {
                    let c = grid[floor][j];
                    grid[floor][j] = 'O';
                    grid[i][j] = c;
                    floor += 1;
                }
                '.' => continue,
                '#' => floor = i + 1,
                _ => panic!("Invalid character"),
            }
        }
    }
}

fn compute_load(grid: &[Vec<char>]) -> usize {
    let ny = grid.len();
    let mut total = 0;
    for (i, row) in grid.iter().enumerate() {
        for c in row {
            if *c == 'O' {
                total += ny - i;
            }
        }
    }
    total
}

fn rotate_clockwise(grid: &[Vec<char>]) -> Vec<Vec<char>> {
    let mut rotated: Vec<Vec<char>> = vec![];
    let ny = grid.len();
    for j in 0..grid[0].len() {
        let row: Vec<char> = (0..ny).map(|i| grid[ny - i - 1][j]).collect();
        rotated.push(row);
    }
    rotated
}

fn run_cycle(grid: &mut Vec<Vec<char>>) {
    for _ in 0..4 {
        flip(grid);
        *grid = rotate_clockwise(grid);
    }
}

fn main() {
    let args = env::args().nth(1).expect("Please provide an input file");
    let file = File::open(args).unwrap();
    let reader = BufReader::new(file);
    let mut grid: Vec<Vec<char>> = Vec::new();
    for line in reader.lines() {
        let line = line.unwrap();
        let row: Vec<char> = line.chars().collect();
        grid.push(row);
    }
    let grid_ = grid.clone();
    flip(&mut grid);
    println!("Part 1: {}", compute_load(&grid));
    grid = grid_;
    let nx = grid[0].len();
    let ny = grid.len();
    let mut record: Vec<Vec<usize>> = vec![];
    let istart;
    let mut cntr = 0;
    loop {
        run_cycle(&mut grid);
        let entry: Vec<usize> = (0..nx * ny)
            .filter(|x| grid[x / nx][x % nx] == 'O')
            .collect();
        if record.contains(&entry) {
            istart = record.iter().position(|e| *e == entry);
            break;
        } else {
            record.push(entry)
        }
        cntr += 1;
    }
    let istart = istart.unwrap();
    let period = cntr - istart;
    let n_cycle_tot = 1000000000;
    let n = (n_cycle_tot - istart) % period;
    for _ in 0..n - 1 {
        run_cycle(&mut grid);
    }
    println!("Part 2: {}", compute_load(&grid));
}
