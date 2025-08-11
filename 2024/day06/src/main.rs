use anyhow::Result;
use std::env;
use std::fs::File;
use std::io::{BufRead, BufReader};

#[derive(Clone, Copy)]
enum Orientation {
    Up = 0,
    Right = 1,
    Down = 2,
    Left = 3,
}

fn next_pos(x: usize, y: usize, heading: &Orientation) -> Option<(usize, usize)> {
    let new_pos = match heading {
        Orientation::Up => (x, y.checked_sub(1)?),
        Orientation::Down => (x, y + 1),
        Orientation::Left => (x.checked_sub(1)?, y),
        Orientation::Right => (x + 1, y),
    };
    Some(new_pos)
}

fn move_guard(
    x0: usize,
    y0: usize,
    is_wall: &[Vec<bool>],
    visited: &mut [bool],
    history: &mut [Vec<bool>; 4],
) -> bool {
    let ny = is_wall.len();
    let nx = is_wall[0].len();
    let mut x = x0;
    let mut y = y0;
    let mut heading = Orientation::Up;
    loop {
        visited[y * nx + x] = true;
        let (x_next, y_next) = match next_pos(x, y, &heading) {
            Some(pos) => pos,
            None => return false,
        };
        if x_next >= nx || y_next >= ny {
            return false;
        }
        if is_wall[y_next][x_next] {
            heading = match heading {
                Orientation::Up => Orientation::Right,
                Orientation::Right => Orientation::Down,
                Orientation::Down => Orientation::Left,
                Orientation::Left => Orientation::Up,
            };
        } else {
            x = x_next;
            y = y_next;
        }
        let heading_index = heading as usize;
        if history[heading_index][y * nx + x] {
            return true; // Loop detected
        }
        history[heading_index][y * nx + x] = true;
    }
}

fn main() -> Result<()> {
    let args = env::args().nth(1).expect("Please provide an input file");
    let file = File::open(args)?;
    let reader = BufReader::new(file);
    let mut x_start: usize = 0;
    let mut y_start: usize = 0;
    let mut is_wall: Vec<Vec<bool>> = Vec::new();
    for (y, line) in reader.lines().enumerate() {
        let line = line.unwrap();
        let mut row: Vec<bool> = Vec::new();
        for symbol in line.chars() {
            row.push(symbol == '#');
            if symbol == '^' {
                x_start = row.len() - 1;
                y_start = y;
            }
        }
        is_wall.push(row);
    }
    let ny = is_wall.len();
    let nx = is_wall[0].len();
    let mut history: [Vec<bool>; 4] = Default::default();
    for h in history.iter_mut() {
        *h = vec![false; ny * nx];
    }
    let mut visited = vec![false; ny * nx];
    move_guard(x_start, y_start, &is_wall, &mut visited, &mut history);
    let part1 = visited.iter().filter(|&&v| v).count();
    println!("Part 1: {}", part1);

    let new_walls: Vec<usize> = visited
        .iter()
        .enumerate()
        .filter_map(|(i, &v)| {
            if v && i != y_start * nx + x_start {
                Some(i)
            } else {
                None
            }
        })
        .collect();
    let mut part2 = 0;
    for pixel in new_walls {
        for h in history.iter_mut() {
            h.fill_with(|| false);
        }
        let x = pixel % nx;
        let y = pixel / nx;
        is_wall[y][x] = true;
        if move_guard(x_start, y_start, &is_wall, &mut visited, &mut history) {
            part2 += 1;
        }
        is_wall[y][x] = false;
    }
    println!("Part 2: {}", part2);
    Ok(())
}
