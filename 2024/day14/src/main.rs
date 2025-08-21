use anyhow::Result;
use std::env;
use std::fs::File;
use std::io::{BufRead, BufReader};

const NX: i32 = 101;
const NY: i32 = 103;

#[derive(Clone)]
struct Robot {
    x: i32,
    y: i32,
    u: i32,
    v: i32,
}

fn build_grid(robots: &[Robot]) -> Vec<Vec<usize>> {
    let mut grid = vec![vec![0; NX as usize]; NY as usize];
    for robot in robots.iter() {
        let x = robot.x as usize;
        let y = robot.y as usize;
        grid[y][x] += 1;
    }
    grid
}

fn iterate(grid: &mut [Vec<usize>], robots: &mut [Robot]) {
    for robot in robots.iter_mut() {
        grid[robot.y as usize][robot.x as usize] -= 1;
        robot.x = (robot.x + robot.u) % NX;
        robot.y = (robot.y + robot.v) % NY;
        if robot.x < 0 {
            robot.x += NX;
        }
        if robot.y < 0 {
            robot.y += NY;
        }
        grid[robot.y as usize][robot.x as usize] += 1;
    }
}

fn get_score(robots: &[Robot]) -> usize {
    let x_mid = NX / 2;
    let y_mid = NY / 2;
    let mut top_left = 0;
    let mut top_right = 0;
    let mut bottom_left = 0;
    let mut bottom_right = 0;
    for robot in robots.iter() {
        if robot.x == x_mid || robot.y == y_mid {
            continue;
        }
        if robot.x < x_mid {
            if robot.y < y_mid {
                top_left += 1;
            } else {
                bottom_left += 1;
            }
        } else {
            if robot.y < y_mid {
                top_right += 1;
            } else {
                bottom_right += 1;
            }
        }
    }
    return top_left * top_right * bottom_left * bottom_right;
}

fn heighest_tree(grid: &[Vec<usize>], robots: &[Robot]) -> usize {
    let mut max_height = 0;
    let ny = NY as usize;
    for robot in robots.iter() {
        let mut height = 1;
        let mut x_start = robot.x - 1;
        let mut x_end = robot.x + 1;
        let mut y = robot.y as usize;
        while y < ny && x_start >= 0 && x_end < NX {
            let col_start = x_start as usize;
            let col_end = x_end as usize;
            let valid = (col_start..col_end + 1).all(|x| grid[y][x] > 0);
            if !valid {
                break;
            }
            height += 1;
            x_start -= 1;
            x_end += 1;
            y += 1;
        }
        if height > max_height {
            max_height = height;
        }
    }
    max_height
}

pub fn plot_grid(grid: &[Vec<usize>]) {
    for row in grid.iter() {
        for &cntr in row.iter() {
            if cntr == 0 {
                print!(".");
            } else {
                print!("{}", cntr);
            }
        }
        println!();
    }
}

fn main() -> Result<()> {
    let filename = env::args().nth(1).expect("Please provide a filename");
    let file = File::open(filename).unwrap();
    let reader = BufReader::new(file);
    let mut robots: Vec<Robot> = Vec::<Robot>::new();
    for line in reader.lines() {
        let line = line?;
        let elem: Vec<&str> = line.split_whitespace().collect();
        let pos: Vec<i32> = elem[0][2..]
            .split(',')
            .map(|x| x.parse::<i32>().unwrap())
            .collect();
        let vel: Vec<i32> = elem[1][2..]
            .split(',')
            .map(|x| x.parse::<i32>().unwrap())
            .collect();
        let robot = Robot {
            x: pos[0],
            y: pos[1],
            u: vel[0],
            v: vel[1],
        };
        robots.push(robot);
    }
    // Part 1
    let mut robots_copy = robots.clone();
    let mut grid = build_grid(&robots);
    for _ in 0..100 {
        iterate(&mut grid, &mut robots);
    }
    let part1 = get_score(&robots);
    println!("Part 1: {}", part1);
    grid = build_grid(&robots_copy);
    let mut tree_height = 0;
    let mut part2 = 0;
    while tree_height < 8 {
        iterate(&mut grid, &mut robots_copy);
        tree_height = heighest_tree(&grid, &robots);
        part2 += 1;
    }
    println!("Part2: {}", part2);
    plot_grid(&grid);
    Ok(())
}
