use std::collections::HashSet;
use std::env;
use std::fs::File;
use std::io::{BufRead, BufReader};

#[derive(Eq, Hash, PartialEq)]
enum Direction {
    Up,
    Down,
    Left,
    Right,
}

fn update_pos(
    grid: &[Vec<char>],
    pos: (usize, usize),
    direction: &Direction,
) -> Option<(usize, usize)> {
    let new_pos = match direction {
        Direction::Up => (usize::checked_sub(pos.0, 1)?, pos.1),
        Direction::Down => (pos.0 + 1, pos.1),
        Direction::Left => (pos.0, usize::checked_sub(pos.1, 1)?),
        Direction::Right => (pos.0, pos.1 + 1),
    };
    if new_pos.0 >= grid.len() || new_pos.1 >= grid[0].len() {
        return None;
    }
    Some(new_pos)
}

fn propagate_beam<'a>(
    grid: &[Vec<char>],
    pos: (usize, usize),
    direction: &'a Direction,
    visited: &mut Vec<(usize, usize, &'a Direction)>,
) {
    visited.push((pos.0, pos.1, direction));
    let mut new_dir: Vec<&Direction> = vec![];
    match grid[pos.0][pos.1] {
        '|' => {
            match direction {
                Direction::Up => new_dir.push(&Direction::Up),
                Direction::Down => new_dir.push(&Direction::Down),
                Direction::Left => {
                    new_dir.push(&Direction::Up);
                    new_dir.push(&Direction::Down);
                }
                Direction::Right => {
                    new_dir.push(&Direction::Up);
                    new_dir.push(&Direction::Down);
                }
            };
        }
        '-' => match direction {
            Direction::Left => new_dir.push(&Direction::Left),
            Direction::Right => new_dir.push(&Direction::Right),
            Direction::Up => {
                new_dir.push(&Direction::Left);
                new_dir.push(&Direction::Right);
            }
            Direction::Down => {
                new_dir.push(&Direction::Left);
                new_dir.push(&Direction::Right);
            }
        },
        '/' => {
            new_dir.push(match direction {
                Direction::Up => &Direction::Right,
                Direction::Down => &Direction::Left,
                Direction::Left => &Direction::Down,
                Direction::Right => &Direction::Up,
            });
        }
        '\\' => {
            new_dir.push(match direction {
                Direction::Up => &Direction::Left,
                Direction::Down => &Direction::Right,
                Direction::Left => &Direction::Up,
                Direction::Right => &Direction::Down,
            });
        }
        _ => {
            new_dir.push(direction);
        }
    }
    for d in new_dir.iter() {
        let new_pos = update_pos(grid, pos, d);
        if let Some(p) = new_pos {
            if !visited.contains(&(p.0, p.1, d)) {
                propagate_beam(grid, p, d, visited);
            }
        }
    }
}

fn get_energized_tiles(grid: &[Vec<char>], pos: (usize, usize), direction: Direction) -> usize {
    let mut visited: Vec<(usize, usize, &Direction)> = Vec::new();
    propagate_beam(grid, pos, &direction, &mut visited);
    let cells: HashSet<(usize, usize)> = visited.iter().map(|x| (x.0, x.1)).collect();
    cells.len()
}

fn part2(grid: &[Vec<char>]) -> usize {
    let ny = grid.len();
    let nx = grid[0].len();
    let mut best = 0;
    for i in 0..ny {
        let vleft = get_energized_tiles(grid, (i, 0), Direction::Right);
        if vleft > best {
            best = vleft;
        }
        let vright = get_energized_tiles(grid, (i, nx - 1), Direction::Left);
        if vright > best {
            best = vright;
        }
    }
    for j in 0..nx {
        let vtop = get_energized_tiles(grid, (0, j), Direction::Down);
        if vtop > best {
            best = vtop;
        }
        let vbot = get_energized_tiles(grid, (ny - 1, j), Direction::Up);
        if vbot > best {
            best = vbot;
        }
    }
    best
}

fn main() {
    let args = env::args().nth(1).expect("Please provide an input file");
    let file = File::open(args).unwrap();
    let reader = BufReader::new(file);
    let mut grid: Vec<Vec<char>> = vec![];
    for line in reader.lines() {
        let line = line.unwrap();
        grid.push(line.chars().collect());
    }
    let part1 = get_energized_tiles(&grid, (0, 0), Direction::Right);
    println!("Part 1: {}", part1);
    println!("Part 2: {}", part2(&grid));
}
