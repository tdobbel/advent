use anyhow::Result;
use std::collections::HashSet;
use std::env;
use std::fs::File;
use std::io::{BufRead, BufReader};

struct Rope {
    knots: Vec<(i32, i32)>,
    tail_positions: HashSet<(i32, i32)>,
}

impl Rope {
    pub fn new(n: usize) -> Self {
        let knots = vec![(0, 0); n];
        let mut tail_positions: HashSet<(i32, i32)> = HashSet::new();
        tail_positions.insert((0, 0));
        Rope {
            knots,
            tail_positions,
        }
    }

    pub fn move_one_step(&mut self, dx: i32, dy: i32) {
        self.knots[0].0 += dx;
        self.knots[0].1 += dy;
        for i in 1..self.knots.len() {
            let (px, py) = self.knots[i - 1];
            let mut dx_ = px - self.knots[i].0;
            let mut dy_ = py - self.knots[i].1;
            if dx_.abs() > 1 || dy_.abs() > 1 {
                dx_ = dx_.signum();
                dy_ = dy_.signum();
                self.knots[i].0 += dx_;
                self.knots[i].1 += dy_;
            }
        }
        self.tail_positions.insert(*self.knots.last().unwrap());
    }

    pub fn apply_motion(&mut self, direction: char, steps: usize) {
        let mut dx = 0;
        let mut dy = 0;
        match direction {
            'U' => dy = -1,
            'D' => dy = 1,
            'L' => dx = -1,
            'R' => dx = 1,
            _ => panic!("Invalid direction"),
        }
        for _ in 0..steps {
            self.move_one_step(dx, dy);
        }
    }
}

fn main() -> Result<()> {
    let filename = env::args().nth(1).expect("Missing input file");
    let file = File::open(filename)?;
    let reader = BufReader::new(file);
    let mut rope2 = Rope::new(2);
    let mut rope10 = Rope::new(10);
    for line in reader.lines() {
        let line = line?;
        let direction = line.chars().next().expect("Empty line");
        let steps = line[2..].parse::<usize>()?;
        rope2.apply_motion(direction, steps);
        rope10.apply_motion(direction, steps);
    }

    println!("Part 1 : {}", rope2.tail_positions.len());
    println!("Part 2 : {}", rope10.tail_positions.len());
    Ok(())
}
