use std::env;
use std::fs::File;
use std::io::{BufRead, BufReader};

enum Direction {
    Up,
    Down,
    Left,
    Right,
}

struct VerticalWall {
    x: i32,
    y0: i32,
    y1: i32,
    left: bool,
}

struct Bounds {
    minx: i32,
    maxx: i32,
    miny: i32,
    maxy: i32,
}

#[allow(dead_code)]
fn plot_grid(grid: &[Vec<char>]) {
    for row in grid.iter() {
        for c in row.iter() {
            print!("{}", c);
        }
        println!();
    }
}

fn intersect(wall: &VerticalWall, y: i32) -> bool {
    y >= wall.y0 && y <= wall.y1
}

fn compute_volume(walls: &[VerticalWall], miny: i32, maxy: i32) -> i32 {
    let mut volume = 0;
    for y in miny..=maxy {
        let walls_: Vec<&VerticalWall> = walls.iter().filter(|w| intersect(w, y)).collect();
        let mut ignore = vec![false; walls_.len()];
        for i in 1..walls_.len()-1 {
            if walls_[i].left && walls_[i-1].left {
                ignore[i] = true;
            }
            if !walls_[i].left && !walls_[i+1].left {
                ignore[i] = true;
            }
        }
        let mut prev = None;
        for (i,w) in walls_.iter().enumerate() {
            if ignore[i] {
                continue;
            }
            if w.left {
                prev = Some(w.x);
            } else {
                volume += w.x - prev.unwrap() + 1;
                prev = None;
            }
        }
    }
    volume
}

fn get_walls(rules: &[(Direction, usize)]) -> (Vec<VerticalWall>, Bounds) {
    let mut pos: (i32, i32) = (0, 0);
    let mut walls: Vec<VerticalWall> = Vec::new();
    let mut miny = i32::MAX;
    let mut maxy = i32::MIN;
    let mut minx = i32::MAX;
    let mut maxx = i32::MIN;
    for (dir, n) in rules.iter() {
        let (dy, dx) = match dir {
            Direction::Up => (-1, 0),
            Direction::Down => (1, 0),
            Direction::Left => (0, -1),
            Direction::Right => (0, 1),
        };
        let next_pos = (pos.0 + (*n as i32)*dy, pos.1 + (*n as i32)*dx);
        if dx == 0 {
            let (y0, y1) = if dy > 0 { (pos.0, next_pos.0) } else { (next_pos.0, pos.0) };
            walls.push(VerticalWall {
                x: pos.1,
                y0,
                y1,
                left: dy < 0
            });
        }
        pos = next_pos;
        miny = i32::min(miny, pos.0);
        maxy = i32::max(maxy, pos.0);
        minx = i32::min(minx, pos.1);
        maxx = i32::max(maxx, pos.1);
    }
    walls.sort_by_key(|w| w.x);
    (walls, Bounds { minx, maxx, miny, maxy })
}

fn main() {
    let args = env::args().nth(1).expect("Please provide an input file");
    let file = File::open(args).unwrap();
    let reader = BufReader::new(file);
    let mut rules1: Vec<(Direction, usize)> = Vec::new();
    for line in reader.lines() {
        let line = line.unwrap();
        let entries: Vec<&str> = line.split_whitespace().collect();
        let n = entries[1].parse::<usize>().unwrap();
        let dir = match entries[0] {
            "U" => Direction::Up,
            "D" => Direction::Down,
            "L" => Direction::Left,
            "R" => Direction::Right,
            _ => panic!("Invalid direction"),
        };
        rules1.push((dir, n));
    }
    let (walls, bounds) = get_walls(&rules1);
    // let nx = (bounds.maxx - bounds.minx + 1) as usize;
    // let ny = (bounds.maxy - bounds.miny + 1) as usize;
    println!("Part 1: {}", compute_volume(&walls, bounds.miny, bounds.maxy));
}

