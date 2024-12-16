use std::fs::File;
use std::io::{BufRead, BufReader};
use std::cmp::min;
use std::env;

const TOL_N: usize = 20;
const TOL_DIST: i32 = 40;

#[derive(Debug,Clone)]
enum Direction {
    Left,
    Right,
    Up,
    Down
}

#[derive(Debug,Clone)]
struct MazeCell {
    x: i32,
    y: i32,
    direction: Direction,
    score: i32,
    visited: Vec<(i32,i32)>,
}

fn distance(a: &(i32, i32), b: &(i32,i32)) -> i32 {
    (a.0-b.0).abs() + (a.1-b.1).abs()
}

fn displacement(direction: &Direction) -> (i32, i32) {
    match direction {
        Direction::Left => (-1, 0),
        Direction::Right => (1, 0),
        Direction::Up => (0, -1),
        Direction::Down => (0, 1)
    }
}

fn plot_path(path: &Vec<(i32,i32)>, walls: &Vec<(i32,i32)>, nx: &i32, ny: &i32) {
    for y in 0..*ny {
        for x in 0..*nx {
            if walls.contains(&(x,y)) {
                print!("#");
            } else if path.contains(&(x,y)) {
                print!("x");
            } else {
                print!(".");
            }
        }
        println!();
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

fn solve_maze(
    candidates: &mut Vec<MazeCell>, closed: &mut Vec<(i32,i32)>, end: &(i32,i32),
    walls: &Vec<(i32,i32)>, path: &mut Vec<(i32,i32)>
) -> i32 {
    let mut score = -1;
    while score < 0 {
        let mut ibest: usize = 0;
        let mut best_score = i32::MAX;
        for (i,mc) in candidates.iter().enumerate() {
            let cell_score = mc.score + distance(&(mc.x, mc.y), end);
            if cell_score < best_score {
                best_score = cell_score;
                ibest = i;
            }
        }
        let best = &candidates[ibest];
        if best.x == end.0 && best.y == end.1 {
            for (x,y) in best.visited.iter() {
                path.push((*x,*y));
            }
            path.push((best.x, best.y));
            score = best.score;
            break;
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
    }
    score
}

fn extra_score(tile: &MazeCell, end: &(i32,i32)) -> i32 {
    let mut score = 0;
    let (dx, dy) = (tile.x-end.0, tile.y-end.1);
    if dy > 0 {
        match tile.direction {
            Direction::Left => score += 1000,
            Direction::Right => score += 1000,
            Direction::Down => score += 2000,
            Direction::Up => score += 0,
        };
    }
    if dx > 0 {
        match tile.direction {
            Direction::Left => score += 2000,
            Direction::Right => score += 0,
            Direction::Down => score += 1000,
            Direction::Up => score += 1000,
        };
    }
    return score + dx + dy;
}

fn get_best_paths(
    candidates: &mut Vec<MazeCell>, closed: &mut Vec<(i32,i32)>, best_score: &i32,
    ref_path: &Vec<(i32,i32)>, end: &(i32,i32), walls: &Vec<(i32,i32)>
) -> Vec<(i32,Vec<(i32,i32)>)> {
    let mut paths = Vec::<(i32,Vec<(i32,i32)>)>::new();
    let mut n = candidates.len();
    while n > 0 {
        let mut ibest: usize = 0;
        let mut min_score = i32::MAX;
        for (i,mc) in candidates.iter().enumerate() {
            let cell_score = mc.score + distance(&(mc.x, mc.y), end);
            if cell_score < min_score {
                min_score = cell_score;
                ibest = i;
            }
        }
        let best = &candidates[ibest];
        if best.x == end.0 && best.y == end.1 {
            paths.push((best.score, best.visited.clone()));
        }
        let new_cells = get_next_cells(best, walls);
        candidates.remove(ibest);
        for cell in new_cells {
            let pos = (cell.x, cell.y);
            let mut ip = cell.visited.len();
            if ip > ref_path.len() + TOL_N  {
                closed.push(pos);
                continue
            }
            ip = min(ip, ref_path.len()-1);
            let dist = distance(&(cell.x, cell.y), &ref_path[ip]);
            if cell.score + extra_score(&cell, end) > *best_score || dist > TOL_DIST {
                closed.push(pos);
                continue
            }
            candidates.push(cell);
        }
        n = candidates.len();
    }
    paths
}

fn main() {
    let args: Vec<String> = env::args().collect();
    assert_eq!(args.len(), 2);
    let file = File::open(&args[1]).unwrap();
    let reader = BufReader::new(file);
    let mut walls = Vec::<(i32, i32)>::new();
    let mut start = (-1, -1);
    let mut end = (-1, -1);
    let mut nx: i32 = 0;
    let mut ny: i32 = 0;
    for (iy,line) in reader.lines().enumerate() {
        let line = line.unwrap();
        ny += 1;
        nx = line.len() as i32;
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
    let mut candidates;
    let mut closed;
    // Find score of best path
    candidates = Vec::<MazeCell>::new();
    candidates.push(MazeCell {
        x: start.0, y: start.1, direction: Direction::Right,
        score: 0, visited: Vec::<(i32,i32)>::new()
    });
    closed = Vec::<(i32,i32)>::new();
    let mut ref_path = Vec::<(i32,i32)>::new();
    let score = solve_maze(&mut candidates, &mut closed, &end, &walls, &mut ref_path);
    plot_path(&ref_path, &walls, &nx, &ny);
    println!("Best score: {}", score);
    // Find all best paths
    candidates = Vec::<MazeCell>::new();
    candidates.push(MazeCell {
        x: start.0, y: start.1, direction: Direction::Right,
        score: 0, visited: Vec::<(i32,i32)>::new()
    });
    closed = Vec::<(i32,i32)>::new();
    let paths = get_best_paths(&mut candidates, &mut closed, &score, &ref_path, &end, &walls);
    let mut best_cells = Vec::<(i32,i32)>::new();
    for (_,path) in paths.iter() {
        for (x,y) in path.iter() {
            if !best_cells.contains(&(*x,*y)) {
                best_cells.push((*x,*y));
            }
        }
    }
    println!("{} tiles are part of at least one of the best paths", best_cells.len()+1);
}
