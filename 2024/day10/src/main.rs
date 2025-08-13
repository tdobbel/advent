use std::collections::HashMap;
use std::env;
use std::fs::File;
use std::io::{BufRead, BufReader};

type Counter = HashMap<(usize, usize), usize>;

enum Direction {
    Up,
    Down,
    Left,
    Right,
}

fn get_neighbor(pos: (usize, usize), dir: &Direction) -> Option<(usize, usize)> {
    let (x, y) = pos;
    let neighbor = match dir {
        Direction::Up => (x, y.checked_sub(1)?),
        Direction::Down => (x, y + 1),
        Direction::Left => (x.checked_sub(1)?, y),
        Direction::Right => (x + 1, y),
    };
    Some(neighbor)
}

fn count_trails(topo: &[Vec<u8>], pos: (usize, usize), counter: &mut Counter) {
    let value = topo[pos.1][pos.0];
    if value == 9 {
        *counter.entry(pos).or_insert(0) += 1;
        return;
    }
    let ny = topo.len();
    let nx = topo[0].len();
    let directions = [
        Direction::Up,
        Direction::Down,
        Direction::Left,
        Direction::Right,
    ];
    for dir in directions.iter() {
        let (x, y) = match get_neighbor(pos, dir) {
            Some(p) => {
                if p.0 >= nx || p.1 >= ny {
                    continue;
                } else {
                    p
                }
            }
            None => continue,
        };
        if topo[y][x] == value + 1 {
            count_trails(topo, (x, y), counter);
        }
    }
}

fn main() {
    let args = env::args().nth(1).expect("Please provide an input file");
    let file = File::open(args).unwrap();
    let reader = BufReader::new(file);
    let mut topo: Vec<Vec<u8>> = Vec::new();
    let mut seeds: Vec<(usize, usize)> = Vec::new();
    for (y, line) in reader.lines().enumerate() {
        let line = line.unwrap();
        let mut row: Vec<u8> = Vec::with_capacity(line.len());
        for (x, c) in line.chars().enumerate() {
            let value = c.to_digit(10).unwrap() as u8;
            row.push(value);
            if value == 0 {
                seeds.push((x, y));
            }
        }
        topo.push(row);
    }

    let mut part1 = 0;
    let mut part2 = 0;
    for seed in seeds.iter() {
        let mut counter = HashMap::new();
        count_trails(&topo, *seed, &mut counter);
        part1 += counter.len();
        part2 += counter.values().sum::<usize>()
    }
    println!("Part 1: {}", part1);
    println!("Part 2: {}", part2);
}
