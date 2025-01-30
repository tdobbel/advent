use std::env;
use std::fs::File;
use std::io::{BufRead, BufReader};

enum Direction {
    Up,
    Down,
    Left,
    Right,
}

impl Direction {
    const VALUES: [Direction; 4] = [
        Direction::Up,
        Direction::Down,
        Direction::Left,
        Direction::Right,
    ];
}

struct VerticalWall {
    id: usize,
    x: i64,
    y0: i64,
    y1: i64,
    left: bool,
    next: usize,
}

impl VerticalWall {
    fn get_top_bottom(&self) -> (i64, i64) {
        if self.y0 < self.y1 {
            (self.y0, self.y1)
        } else {
            (self.y1, self.y0)
        }
    }

    fn intersect(&self, y: i64) -> bool {
        let (ymin, ymax) = self.get_top_bottom();
        y >= ymin && y <= ymax
    }
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

fn compute_level(walls: &[VerticalWall], y: i64) -> i64 {
    let mut total = 0;
    let walls_: Vec<&VerticalWall> = walls.iter().filter(|w| w.intersect(y)).collect();
    for i in 0..walls_.len() - 1 {
        if walls_[i].left == walls_[i + 1].left {
            total += walls_[i + 1].x - walls_[i].x;
            continue;
        }
        if walls_[i].left && !walls_[i + 1].left {
            total += walls_[i + 1].x - walls_[i].x;
            continue;
        }
        if !walls_[i].left && walls_[i + 1].left {
            if walls_[i].next == walls_[i + 1].id && walls_[i].y1 == y {
                total += walls_[i + 1].x - walls_[i].x;
            } else if walls_[i + 1].next == walls_[i].id && walls_[i].y0 == y {
                total += walls_[i + 1].x - walls_[i].x;
            } else {
                total += 1;
            }
        }
    }
    total + 1
}

fn compute_volume(walls: &[VerticalWall], levels: &[i64]) -> i64 {
    let mut v = compute_level(walls, levels[0]);
    let mut row = levels[0];
    let mut volume = v;
    for i in 1..levels.len() {
        let h = levels[i] - row;
        volume += (h - 1) * v;
        volume += compute_level(walls, levels[i]);
        if i == levels.len() - 1 {
            break;
        }
        row = levels[i] + 1;
        v = compute_level(walls, row);
        volume += v;
    }
    volume
}

fn get_walls(rules: &[(Direction, usize)]) -> (Vec<VerticalWall>, Vec<i64>) {
    let mut levels: Vec<i64> = Vec::new();
    let mut pos: (i64, i64) = (0, 0);
    let mut walls: Vec<VerticalWall> = Vec::new();
    let mut iwall = 0;
    for (dir, n) in rules.iter() {
        let (dy, dx) = match dir {
            Direction::Up => (-1, 0),
            Direction::Down => (1, 0),
            Direction::Left => (0, -1),
            Direction::Right => (0, 1),
        };
        let next_pos = (pos.0 + (*n as i64) * dy, pos.1 + (*n as i64) * dx);
        if !levels.contains(&pos.0) {
            levels.push(pos.0);
        }
        if dx == 0 {
            walls.push(VerticalWall {
                id: iwall,
                x: pos.1,
                y0: pos.0,
                y1: next_pos.0,
                left: dy < 0,
                next: iwall + 1,
            });
            iwall += 1;
        }
        pos = next_pos;
    }
    let last = walls.get_mut(iwall - 1).unwrap();
    last.next = 0;
    walls.sort_by_key(|w| w.x);
    levels.sort();
    (walls, levels)
}

#[allow(dead_code)]
fn solve(grid: &mut [Vec<char>], pos: (usize, usize), cntr: &mut usize) {
    grid[pos.0][pos.1] = '#';
    *cntr += 1;
    for dir in Direction::VALUES.iter() {
        let pos_ = match dir {
            Direction::Up => (pos.0 - 1, pos.1),
            Direction::Down => (pos.0 + 1, pos.1),
            Direction::Left => (pos.0, pos.1 - 1),
            Direction::Right => (pos.0, pos.1 + 1),
        };
        if grid[pos_.0][pos_.1] == '#' {
            continue;
        }
        solve(grid, pos_, cntr);
    }
}

fn main() {
    let args = env::args().nth(1).expect("Please provide an input file");
    let file = File::open(args).unwrap();
    let reader = BufReader::new(file);
    let mut rules1: Vec<(Direction, usize)> = Vec::new();
    let mut rules2: Vec<(Direction, usize)> = Vec::new();
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
        let hex = &entries[2][2..entries[2].len() - 1];
        let n2 = usize::from_str_radix(&hex[..hex.len() - 1], 16).unwrap();
        let dir = match &hex[hex.len() - 1..] {
            "0" => Direction::Right,
            "1" => Direction::Down,
            "2" => Direction::Left,
            "3" => Direction::Up,
            _ => panic!("Invalid direction"),
        };
        rules2.push((dir, n2));
    }
    for (i, rules) in [rules1, rules2].iter().enumerate() {
        let (walls, levels) = get_walls(rules);
        let sol = compute_volume(&walls, &levels);
        println!("Part {}: {}", i + 1, sol);
    }
}
