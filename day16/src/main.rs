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

struct MazeCell {
    x: i32,
    y: i32,
    direction: Direction,
    score: i32,
    visited: Vec<(i32,i32)>,
}

fn distance_from_end(mc: &MazeCell, end: &(i32,i32)) -> i32 {
    (mc.x-end.0).abs() + (mc.y-end.1).abs()
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

fn get_next_cells(mc: &MazeCell, walls: &Vec<(i32,i32)>) -> Vec<MazeCell> {
    let left_dir = turn_left(&mc.direction);
    let right_dir = turn_right(&mc.direction);
    let mut cells = Vec::<MazeCell>::new();
    for (i,d) in [mc.direction.clone(), left_dir, right_dir].iter().enumerate() {
        let (shiftx, shifty) = displacement(d);
        let (x, y) = (mc.x+shiftx, mc.y+shifty);
        if walls.contains(&(x,y)) || mc.visited.contains(&(x,y)) {
            continue;
        }
        let mut visited = mc.visited.clone();
        visited.push((mc.x, mc.y));
        let mut score = mc.score;
        if i == 0 {
            score += 1;
        } else {
            score += 1001;
        }
        cells.push(MazeCell { x, y, direction: d.clone(), score, visited });
    }
    cells
}

fn solve_maze(candidates: &mut Vec<MazeCell>, closed: &mut Vec<(i32,i32)>, end: &(i32,i32), walls: &Vec<(i32,i32)>) -> i32 {
    let mut ibest: usize = 0;
    let mut best_score = i32::MAX;
    for (i,mc) in candidates.iter().enumerate() {
        let score = mc.score + distance_from_end(mc, end);
        if score < best_score {
            best_score = score;
            ibest = i;
        }
    }
    let best = &candidates[ibest];
    if best.x == end.0 && best.y == end.1 {
        return best.score;
    }
    let new_cells = get_next_cells(best, walls);
    closed.push((best.x, best.y));
    for cell in new_cells {
        candidates.push(cell);
    }
    let mut remove = Vec::<usize>::new();
    for (i,mc) in candidates.iter().enumerate() {
        if closed.contains(&(mc.x, mc.y)) {
            remove.push(i);
        }
    }
    for i in remove.iter().rev() {
        candidates.remove(*i);
    }
    return solve_maze(candidates, closed, end, walls)
}

fn main() {
    let args: Vec<String> = env::args().collect();
    assert_eq!(args.len(), 2);
    let file = File::open(&args[1]).unwrap();
    let reader = BufReader::new(file);
    let mut walls = Vec::<(i32, i32)>::new();
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
    let mut candidates = Vec::<MazeCell>::new();
    candidates.push(MazeCell { x: start.0, y: start.1, direction: Direction::Right, score: 0, visited: Vec::<(i32,i32)>::new() });
    let mut closed = Vec::<(i32,i32)>::new();
    let score = solve_maze(&mut candidates, &mut closed, &end, &walls);
    println!("Best score: {}", score);
}
