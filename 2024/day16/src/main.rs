use std::env;
use std::fs::File;
use std::io::{BufRead, BufReader};
use anyhow::Result;

#[derive(Debug, Clone)]
enum Direction {
    Left,
    Right,
    Up,
    Down,
}

#[derive(Debug, Clone)]
struct MazeCell {
    x: i32,
    y: i32,
    direction: Direction,
    score: i32,
    visited: Vec<(i32, i32)>,
}

fn distance(a: &(i32, i32), b: &(i32, i32)) -> i32 {
    (a.0 - b.0).abs() + (a.1 - b.1).abs()
}

fn displacement(direction: &Direction) -> (i32, i32) {
    match direction {
        Direction::Left => (-1, 0),
        Direction::Right => (1, 0),
        Direction::Up => (0, -1),
        Direction::Down => (0, 1),
    }
}

fn plot_path(path: &[(i32, i32)], walls: &[(i32, i32)], nx: &i32, ny: &i32) {
    for y in 0..*ny {
        for x in 0..*nx {
            if walls.contains(&(x, y)) {
                print!("#");
            } else if path.contains(&(x, y)) {
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
        Direction::Down => Direction::Right,
    }
}

fn turn_right(direction: &Direction) -> Direction {
    match direction {
        Direction::Left => Direction::Up,
        Direction::Right => Direction::Down,
        Direction::Up => Direction::Right,
        Direction::Down => Direction::Left,
    }
}

fn get_next_cells(
    mc: &MazeCell,
    walls: &[(i32, i32)],
    closed: &mut Vec<(i32, i32)>,
) -> Vec<MazeCell> {
    let left_dir = turn_left(&mc.direction);
    let right_dir = turn_right(&mc.direction);
    let mut cells = Vec::<MazeCell>::new();
    for (i, d) in [mc.direction.clone(), left_dir, right_dir]
        .iter()
        .enumerate()
    {
        let (shiftx, shifty) = displacement(d);
        let (x, y) = (mc.x + shiftx, mc.y + shifty);
        if walls.contains(&(x, y)) || mc.visited.contains(&(x, y)) || closed.contains(&(x, y)) {
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
        closed.push((x, y));
        cells.push(MazeCell {
            x,
            y,
            direction: d.clone(),
            score,
            visited,
        });
    }
    cells
}

fn solve_maze(
    candidates: &mut Vec<MazeCell>,
    closed: &mut Vec<(i32, i32)>,
    end: &(i32, i32),
    walls: &[(i32, i32)],
    path: &mut Vec<(i32, i32)>,
    target_score: Option<i32>,
) -> i32 {
    let mut score = -1;
    while score < 0 {
        let mut ibest: usize = 0;
        let mut best_score = i32::MAX;
        for (i, mc) in candidates.iter().enumerate() {
            let cell_score = mc.score + distance(&(mc.x, mc.y), end);
            if cell_score < best_score {
                best_score = cell_score;
                ibest = i;
            }
        }
        if let Some(target_score) = target_score {
            if best_score > target_score {
                return -1;
            }
        }
        let best = &candidates[ibest];
        if best.x == end.0 && best.y == end.1 {
            for (x, y) in best.visited.iter() {
                path.push((*x, *y));
            }
            path.push((best.x, best.y));
            score = best.score;
            break;
        }
        let new_cells = get_next_cells(best, walls, closed);
        closed.push((best.x, best.y));
        candidates.remove(ibest);
        for cell in new_cells {
            candidates.push(cell);
        }
        if candidates.is_empty() {
            return -1;
        }
    }
    score
}

fn get_sub_path(
    ref_path: &[(i32, i32)],
    candidates: &mut Vec<MazeCell>,
    closed: &mut Vec<(i32, i32)>,
    walls: &[(i32, i32)],
    n: usize,
) {
    let start = ref_path[0];
    let mut tile = MazeCell {
        x: start.0,
        y: start.1,
        direction: Direction::Right,
        score: 0,
        visited: Vec::<(i32, i32)>::new(),
    };
    for rpath in ref_path.iter().take(n).skip(1) {
        closed.push((tile.x, tile.y));
        let next_tiles = get_next_cells(&tile, walls, closed);
        for next_tile in next_tiles {
            if next_tile.x == rpath.0 && next_tile.y == rpath.1 {
                tile = next_tile;
                break;
            }
        }
    }
    closed.push(ref_path[n]);
    candidates.push(tile);
}

fn get_best_paths(
    ref_path: &[(i32, i32)],
    target: i32,
    end: &(i32, i32),
    walls: &[(i32, i32)],
) -> usize {
    let mut all_tiles = Vec::<(i32, i32)>::new();
    for (x, y) in ref_path.iter() {
        all_tiles.push((*x, *y));
    }
    for n in 1..ref_path.len() - 2 {
        print!("Trying with n = {}/{} ", n, ref_path.len() - 2);
        let mut candidates = Vec::<MazeCell>::new();
        let mut closed = Vec::<(i32, i32)>::new();
        get_sub_path(ref_path, &mut candidates, &mut closed, walls, n);
        let mut new_path = Vec::<(i32, i32)>::new();
        let new_score = solve_maze(
            &mut candidates,
            &mut closed,
            end,
            walls,
            &mut new_path,
            Some(target),
        );
        if new_score < 0 {
            println!("-> No solution found");
            continue;
        }
        println!("-> Found new path with score {}", new_score);
        for (x, y) in new_path.iter() {
            if !all_tiles.contains(&(*x, *y)) {
                all_tiles.push((*x, *y));
            }
        }
    }
    all_tiles.len()
}

fn main() -> Result<()> {
    let args = env::args().nth(1).expect("No input file provided");
    let file = File::open(args)?;
    let reader = BufReader::new(file);
    let mut walls = Vec::<(i32, i32)>::new();
    let mut start = (-1, -1);
    let mut end = (-1, -1);
    let mut nx: i32 = 0;
    let mut ny: i32 = 0;
    for (iy, line) in reader.lines().enumerate() {
        let line = line.unwrap();
        ny += 1;
        nx = line.len() as i32;
        for (ix, c) in line.chars().enumerate() {
            match c {
                '#' => walls.push((ix as i32, iy as i32)),
                '.' => continue,
                'S' => start = (ix as i32, iy as i32),
                'E' => end = (ix as i32, iy as i32),
                _ => panic!("Invalid character"),
            }
        }
    }
    let mut candidates;
    let mut closed;
    // Find score of best path
    candidates = Vec::<MazeCell>::new();
    candidates.push(MazeCell {
        x: start.0,
        y: start.1,
        direction: Direction::Right,
        score: 0,
        visited: Vec::<(i32, i32)>::new(),
    });
    closed = Vec::<(i32, i32)>::new();
    let mut ref_path = Vec::<(i32, i32)>::new();
    let score = solve_maze(
        &mut candidates,
        &mut closed,
        &end,
        &walls,
        &mut ref_path,
        None,
    );
    plot_path(&ref_path, &walls, &nx, &ny);
    println!("Best score: {}", score);
    // Find all best paths
    let n_tiles = get_best_paths(&ref_path, score, &end, &walls);
    println!(
        "{} tiles are part of at least one of the best paths",
        n_tiles
    );
    Ok(())
}
