use anyhow::{anyhow, Result};
use std::collections::HashSet;
use std::io::{BufRead, BufReader};
use std::{env, fs};

#[derive(Eq, PartialEq, Hash, Clone, Copy)]
enum Direction {
    Up,
    Right,
    Down,
    Left,
}

#[derive(PartialEq, Eq, Hash, Clone, Copy)]
struct BeamState {
    x: usize,
    y: usize,
    heading: Direction,
}

type Contraption = Vec<Vec<char>>;

fn main() -> Result<()> {
    let filename = env::args().nth(1).expect("Missing input file");
    let file = fs::File::open(filename)?;
    let reader = BufReader::new(file);

    let mut contraption: Contraption = Vec::new();

    for line in reader.lines() {
        let line = line?;
        let row: Vec<char> = line.chars().collect();
        contraption.push(row);
    }
    let ny = contraption.len();
    let nx = contraption[0].len();

    let mut visited = HashSet::new();
    let mut start = BeamState {
        x: 0,
        y: 0,
        heading: Direction::Right,
    };
    let part1 = count_energized(&contraption, start, &mut visited)?;
    println!("Part 1: {}", part1);

    let mut part2: usize = 0;
    for y in 0..ny {
        // left
        start = BeamState {
            x: 0,
            y,
            heading: Direction::Right,
        };
        part2 = part2.max(count_energized(&contraption, start, &mut visited)?);

        // right
        start = BeamState {
            x: nx - 1,
            y,
            heading: Direction::Left,
        };
        part2 = part2.max(count_energized(&contraption, start, &mut visited)?);
    }

    for x in 0..nx {
        // top
        start = BeamState {
            x,
            y: 0,
            heading: Direction::Down,
        };
        part2 = part2.max(count_energized(&contraption, start, &mut visited)?);

        // bottom
        start = BeamState {
            x,
            y: ny - 1,
            heading: Direction::Up,
        };
        part2 = part2.max(count_energized(&contraption, start, &mut visited)?);
    }

    println!("Part 2: {}", part2);

    Ok(())
}

fn next_state(contraption: &Contraption, bs: &mut BeamState) -> bool {
    let ny = contraption.len();
    let nx = contraption[0].len();
    match bs.heading {
        Direction::Up => {
            if bs.y == 0 {
                return false;
            }
            bs.y -= 1;
        }
        Direction::Right => {
            if bs.x == nx - 1 {
                return false;
            }
            bs.x += 1;
        }
        Direction::Down => {
            if bs.y == ny - 1 {
                return false;
            }
            bs.y += 1;
        }
        Direction::Left => {
            if bs.x == 0 {
                return false;
            }
            bs.x -= 1;
        }
    }
    true
}

fn propagate_beam(
    contraption: &Contraption,
    bs: BeamState,
    visited: &mut HashSet<BeamState>,
) -> Result<()> {
    visited.insert(bs);
    let mut next_dir: [Option<Direction>; 2] = [None, None];
    match contraption[bs.y][bs.x] {
        '.' => {
            next_dir[0] = Some(bs.heading);
        }
        '/' => {
            next_dir[0] = match bs.heading {
                Direction::Up => Some(Direction::Right),
                Direction::Right => Some(Direction::Up),
                Direction::Down => Some(Direction::Left),
                Direction::Left => Some(Direction::Down),
            };
        }
        '\\' => {
            next_dir[0] = match bs.heading {
                Direction::Up => Some(Direction::Left),
                Direction::Right => Some(Direction::Down),
                Direction::Down => Some(Direction::Right),
                Direction::Left => Some(Direction::Up),
            };
        }
        '-' => match bs.heading {
            Direction::Right => next_dir[0] = Some(Direction::Right),
            Direction::Left => next_dir[0] = Some(Direction::Left),
            Direction::Up | Direction::Down => {
                next_dir[0] = Some(Direction::Right);
                next_dir[1] = Some(Direction::Left);
            }
        },
        '|' => match bs.heading {
            Direction::Up => next_dir[0] = Some(Direction::Up),
            Direction::Down => next_dir[0] = Some(Direction::Down),
            Direction::Left | Direction::Right => {
                next_dir[0] = Some(Direction::Up);
                next_dir[1] = Some(Direction::Down);
            }
        },
        _ => return Err(anyhow!("Unexpected character {}", contraption[bs.y][bs.x])),
    }
    for dir in next_dir.iter() {
        if dir.is_none() {
            break;
        }
        let d = dir.unwrap();
        let mut new_state = BeamState {
            x: bs.x,
            y: bs.y,
            heading: d,
        };
        if !next_state(contraption, &mut new_state) {
            continue;
        }
        if !visited.contains(&new_state) {
            propagate_beam(contraption, new_state, visited)?;
        }
    }
    Ok(())
}

fn count_energized(
    contraption: &Contraption,
    bs: BeamState,
    visited: &mut HashSet<BeamState>,
) -> Result<usize> {
    visited.clear();
    propagate_beam(contraption, bs, visited)?;
    let total = visited
        .iter()
        .map(|bs| (bs.x, bs.y))
        .collect::<HashSet<(usize, usize)>>()
        .len();
    Ok(total)
}
