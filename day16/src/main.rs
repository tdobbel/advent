use std::fs::File;
use std::io::{BufRead, BufReader};
use std::env;


#[derive(Debug,Clone)]
enum Direction {
    Left,
    Right,
    Up,
    Down
}

fn displacement(direction: &Direction) -> (i32, i32) {
    match direction {
        Direction::Left => (-1, 0),
        Direction::Right => (1, 0),
        Direction::Up => (0, -1),
        Direction::Down => (0, 1)
    }
}

fn turn_left(direction: &Direction) -> Direction {
    match direction {
        Direction::Left => Direction::Down,
        Direction::Right => Direction::Up,
        Direction::Up => Direction::Left,
        Direction::Down => Direction::Right
    }
}

fn turn_right(direction: &Direction) -> Direction {
    match direction {
        Direction::Left => Direction::Up,
        Direction::Right => Direction::Down,
        Direction::Up => Direction::Right,
        Direction::Down => Direction::Left
    }
}

fn solve_maze_dumb(
    x: i32, y: i32, direction: Direction, score: i32, end: &(i32,i32),
    walls: &Vec<(i32,i32)>, visited: &mut Vec<(i32,i32)>, found: &mut Vec<i32>
) -> bool {
    if x == end.0 && y == end.1 {
        found.push(score);
        return true;
    }
    if visited.contains(&(x,y)) || walls.contains(&(x,y)){
        return false;
    }
    visited.push((x,y));
    let (shiftx, shifty) = displacement(&direction);
    let mut result = false;
    result |= solve_maze_dumb(x+shiftx, y+shifty, direction.clone(), score+1, end, walls, visited, found);
    if result {
        return true;
    }
    let left_dir = turn_left(&direction);
    let (shiftx, shifty) = displacement(&left_dir);
    result |= solve_maze_dumb(x+shiftx, y+shifty, left_dir, score+1001, end, walls, &mut visited.clone(), found);
    let right_dir = turn_right(&direction);
    let (shiftx, shifty) = displacement(&right_dir);
    result |= solve_maze_dumb(x+shiftx, y+shifty, right_dir, score+1001, end, walls, &mut visited.clone(), found);
    result
}

fn main() {
    let args: Vec<String> = env::args().collect();
    assert_eq!(args.len(), 2);
    let file = File::open(&args[1]).unwrap();
    let reader = BufReader::new(file);
    let mut walls = Vec::<(i32, i32)>::new();
    let mut visited = Vec::<(i32, i32)>::new();
    let mut start = (-1, -1);
    let mut end = (-1, -1);
    for (iy,line) in reader.lines().enumerate() {
        let line = line.unwrap();
        for (ix,c) in line.chars().enumerate() {
            match c {
                '#' => walls.push((ix as i32, iy as i32)),
                '.' => continue,
                'S' => start = (ix as i32, iy as i32),
                'E' => end = (ix as i32, iy as i32),
                _ => panic!("Invalid character")
            }
        }
    }
    let mut scores = Vec::<i32>::new();
    solve_maze_dumb(start.0, start.1, Direction::Right, 0, &end, &walls, &mut visited, &mut scores);
    println!("Minimum score: {}", scores.iter().min().unwrap());
}
