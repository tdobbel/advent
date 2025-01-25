use std::collections::HashSet;
use std::env;
use std::fs::File;
use std::io::{BufRead, BufReader};

#[derive(Debug,Eq,PartialEq,Clone,Hash)]
enum Direction {
    Up,
    Down,
    Left,
    Right,
}

impl Direction {
    const VALUES: [Direction; 4] = [Direction::Up, Direction::Down, Direction::Left, Direction::Right];
}

trait Status {
    fn get_status(&self) -> ((usize, usize), Direction, usize);
}

#[derive(Debug)]
struct State {
    tile: (usize, usize),
    direction: Direction,
    counter: usize,
    total: usize,
}

impl Status for State {
    fn get_status(&self) -> ((usize, usize), Direction, usize) {
        (self.tile, self.direction.clone(), self.counter)
    }
}

fn distance(a: &(usize, usize), b: &(usize, usize)) -> usize {
    let dx =  if a.0 > b.0 {a.0 - b.0} else {b.0 - a.0};
    let dy = if a.1 > b.1 {a.1 - b.1} else {b.1 - a.1};
    dx + dy
}

#[allow(dead_code)]
fn plot_path(grid: &[Vec<usize>], path: &[(usize,usize)]) {
    for (i,row) in grid.iter().enumerate() {
        for (j,value) in row.iter().enumerate() {
            if path.contains(&(i ,j)) {
                print!("·");
            } else {
                print!("{}", value);
            }
        }
        println!();
    }
}

fn opposite_direction(dir_from: &Direction) -> Direction {
    match dir_from {
        Direction::Up => Direction::Down,
        Direction::Down => Direction::Up,
        Direction::Left => Direction::Right,
        Direction::Right => Direction::Left,
    }
}

fn next_pos(state: &State, grid: &[Vec<usize>]) -> Option<(usize,usize)> {
    let (y, x) = state.tile;
    let (y_, x_) = match state.direction {
        Direction::Up => (usize::checked_sub(y, 1)?, x),
        Direction::Down => (y+1, x),
        Direction::Left => (y, usize::checked_sub(x, 1)?),
        Direction::Right => (y, x+1)
    };
    if y_ >= grid.len() || x_ >= grid[0].len() {
        return None
    }
    Some((y_, x_))
}

fn find_shortest_path(
    grid: &[Vec<usize>],
    min_blocks: usize,
    max_blocks: usize,
) -> Option<usize> {
    let mut queue: Vec<State> = vec![
        State {tile: (0, 0), direction: Direction::Right, counter: 1, total: 0},
        State {tile: (0, 0), direction: Direction::Down, counter: 1, total: 0},
    ];
    let mut closed: HashSet<((usize,usize), Direction, usize)> = HashSet::new();
    let end = (grid.len() - 1, grid[0].len() - 1);
    while !queue.is_empty() {
        let state = queue.remove(0);
        closed.insert(state.get_status());
        let (y, x) =  match next_pos(&state, grid) {
            Some(pos) => pos,
            None => continue,
        };
        let total_ = state.total + grid[y][x];
        if (y, x) == end && state.counter >= min_blocks {
            return Some(total_);
        }
        for direction in Direction::VALUES.iter() {
            if direction == &opposite_direction(&state.direction) {
                continue;
            }
            let counter_ = if direction == &state.direction { state.counter + 1 } else { 1 };
            if counter_ > max_blocks {
                continue
            }
            if state.counter < min_blocks && direction != &state.direction {
                continue
            }
            if !closed.contains(&((y,x), direction.to_owned(), counter_)) {
                queue.push(State {
                    tile: (y, x),
                    direction: direction.to_owned(),
                    counter: counter_,
                    total: total_,
                });
            }
        }
        queue.retain(|state| !closed.contains(&state.get_status()));
        queue.sort_by_key(|state| state.total + distance(&state.tile, &end));
    }
    None
}

fn main() {
    let args = env::args().nth(1).expect("Please provide an input file");
    let file = File::open(args).unwrap();
    let reader = BufReader::new(file);
    let mut grid: Vec<Vec<usize>> = Vec::new();
    for line in reader.lines() {
        let line = line.unwrap();
        let row = line.chars().map(|c| c.to_digit(10).unwrap() as usize).collect();
        grid.push(row);
    }
    let part1 = find_shortest_path(&grid, 1, 3).unwrap();
    println!("Part 1: {}", part1);
    let part2 = find_shortest_path(&grid, 4, 10).unwrap();
    println!("Part 2: {}", part2);
}
