use anyhow::Result;
use std::env;
use std::fs::File;
use std::io::{BufRead, BufReader};

struct Department {
    map: Vec<Vec<char>>,
    nx: usize,
    ny: usize,
    roll_positions: Vec<(usize, usize)>,
}

impl Department {
    pub fn new(map: Vec<Vec<char>>) -> Self {
        let ny = map.len();
        let nx = map[0].len();
        let mut pos: Vec<(usize, usize)> = Vec::new();
        for (y, row) in map.iter().enumerate() {
            for (x, c) in row.iter().enumerate() {
                if *c == '@' {
                    pos.push((x, y));
                }
            }
        }
        Department {
            map,
            nx,
            ny,
            roll_positions: pos,
        }
    }

    pub fn is_accessible(&self, x: usize, y: usize) -> bool {
        let xmin = x.saturating_sub(1);
        let ymin = y.saturating_sub(1);
        let xmax = (x + 1).min(self.nx - 1);
        let ymax = (y + 1).min(self.ny - 1);
        let mut n = 0;
        for iy in ymin..=ymax {
            for ix in xmin..=xmax {
                if ix == x && iy == y {
                    continue;
                }
                if self.map[iy][ix] == '@' {
                    n += 1
                }
                if n > 3 {
                    return false;
                }
            }
        }
        true
    }

    pub fn accessible_vec(&self) -> Vec<bool> {
        self.roll_positions
            .iter()
            .map(|(x, y)| self.is_accessible(*x, *y))
            .collect()
    }

    pub fn remove_all_accessible(&mut self) -> usize {
        let init_size = self.roll_positions.len();
        loop {
            let accessible = self.accessible_vec();
            let n = self.roll_positions.len();
            let mut new_pos: Vec<(usize, usize)> = Vec::new();
            for (i, (x, y)) in self.roll_positions.iter().enumerate() {
                if accessible[i] {
                    self.map[*y][*x] = '.';
                } else {
                    new_pos.push((*x, *y));
                }
            }
            let n_removed = n - new_pos.len();
            self.roll_positions = new_pos;
            if n_removed == 0 {
                break;
            }
        }
        init_size - self.roll_positions.len()
    }
}

fn main() -> Result<()> {
    let args = env::args().nth(1).expect("Please provide an input file");
    let file = File::open(args)?;
    let reader = BufReader::new(file);
    let mut map: Vec<Vec<char>> = Vec::new();
    for line in reader.lines() {
        let line = line?;
        map.push(line.chars().collect());
    }
    let mut dept = Department::new(map);
    let vec_access = dept.accessible_vec();
    let part1 = vec_access.iter().filter(|&x| *x).count();
    let part2 = dept.remove_all_accessible();
    println!("Part 1: {part1}");
    println!("Part 2: {part2}");
    Ok(())
}
