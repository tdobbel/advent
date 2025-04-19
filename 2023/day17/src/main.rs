use anyhow::Result;
use std::cmp::Ordering;
use std::collections::{BinaryHeap, HashSet};
use std::env;
use std::fs::File;
use std::io::{BufRead, BufReader};

#[derive(Eq, PartialEq)]
enum Direction {
    Up,
    Right,
    Down,
    Left,
}

impl Direction {
    const VALUES: [Direction; 4] = [
        Direction::Up,
        Direction::Right,
        Direction::Down,
        Direction::Left,
    ];
}

#[derive(Eq, PartialEq)]
struct Crucible {
    block: (usize, usize),
    direction: usize,
    counter: usize,
    total: usize,
    distance: usize,
}

type State = ((usize, usize), usize, usize);

impl PartialOrd for Crucible {
    fn partial_cmp(&self, other: &Self) -> Option<Ordering> {
        Some(self.cmp(other))
    }
}

impl Ord for Crucible {
    fn cmp(&self, other: &Self) -> std::cmp::Ordering {
        let self_score = self.total + self.distance;
        let other_score = other.total + other.distance;
        other_score.cmp(&self_score)
    }
}

fn distance(a: &(usize, usize), b: &(usize, usize)) -> usize {
    a.0.abs_diff(b.0) + a.1.abs_diff(b.1)
}

fn next_pos(state: &Crucible, direction: &Direction) -> Option<(usize, usize)> {
    let (x, y) = state.block;
    let (x_new, y_new) = match *direction {
        Direction::Up => (x, y.checked_sub(1)?),
        Direction::Right => (x + 1, y),
        Direction::Down => (x, y + 1),
        Direction::Left => (x.checked_sub(1)?, y),
    };
    Some((x_new, y_new))
}

fn find_shortest_path(grid: &[Vec<usize>], min_blocks: usize, max_blocks: usize) -> Option<usize> {
    let nx = grid[0].len();
    let ny = grid.len();
    let end = (nx - 1, ny - 1);
    let mut queue = BinaryHeap::<Crucible>::new();
    let start = Crucible {
        block: (0, 0),
        direction: 1,
        counter: 0,
        total: 0,
        distance: distance(&(0, 0), &end),
    };
    queue.push(start);
    let mut visited = HashSet::<State>::new();
    while !queue.is_empty() {
        let current = queue.pop().unwrap();
        if current.block == end {
            return Some(current.total);
        }
        visited.insert((current.block, current.direction, current.counter));
        let direction_indices = [
            (current.direction + 3) % 4,
            current.direction,
            (current.direction + 1) % 4,
        ];
        for idir in direction_indices {
            if current.direction != idir && current.counter > 0 && current.counter < min_blocks {
                continue;
            }
            let counter = if idir == current.direction {
                current.counter + 1
            } else {
                1
            };
            if counter > max_blocks {
                continue;
            }
            let (x, y) = match next_pos(&current, &Direction::VALUES[idir]) {
                Some(pos) => pos,
                None => continue,
            };
            if x >= nx || y >= ny {
                continue;
            }
            if !visited.insert(((x, y), idir, counter)) {
                continue;
            }
            let next = Crucible {
                block: (x, y),
                direction: idir,
                counter,
                total: current.total + grid[y][x],
                distance: distance(&(x, y), &end),
            };
            queue.push(next);
        }
    }
    None
}

fn main() -> Result<()> {
    let args = env::args().nth(1).expect("Please provide an input file");
    let file = File::open(args)?;
    let reader = BufReader::new(file);
    let mut grid: Vec<Vec<usize>> = Vec::new();
    for line in reader.lines() {
        let line = line.unwrap();
        let row = line
            .chars()
            .map(|c| c.to_digit(10).unwrap() as usize)
            .collect();
        grid.push(row);
    }
    let part1 = find_shortest_path(&grid, 1, 3).unwrap();
    println!("Part 1: {}", part1);
    let part2 = find_shortest_path(&grid, 4, 10).unwrap();
    println!("Part 2: {}", part2);
    Ok(())
}
