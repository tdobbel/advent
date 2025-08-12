use std::collections::{HashMap, HashSet};
use std::env;
use std::fs::File;
use std::io::{BufRead, BufReader};

fn add_antinode(antinodes: &mut HashSet<(i32, i32)>, x: i32, y: i32, nx: i32, ny: i32) -> bool {
    if x < 0 || x >= nx || y < 0 || y >= ny {
        return false;
    }
    antinodes.insert((x, y));
    true
}

fn count_antinodes(antennas: &HashMap<String, Vec<(i32, i32)>>, nx: i32, ny: i32) -> usize {
    let mut antinodes: HashSet<(i32, i32)> = HashSet::new();
    for positions in antennas.values() {
        for i in 0..positions.len() - 1 {
            let (xa, ya) = positions[i];
            for j in i + 1..positions.len() {
                let (xb, yb) = positions[j];
                let dx = xb - xa;
                let dy = yb - ya;
                add_antinode(&mut antinodes, xa - dx, ya - dy, nx, ny);
                add_antinode(&mut antinodes, xb + dx, yb + dy, nx, ny);
            }
        }
    }
    antinodes.len()
}

fn count_antinodes2(antennas: &HashMap<String, Vec<(i32, i32)>>, nx: i32, ny: i32) -> usize {
    let mut antinodes: HashSet<(i32, i32)> = HashSet::new();
    for positions in antennas.values() {
        for i in 0..positions.len() - 1 {
            let (xa, ya) = positions[i];
            for j in i + 1..positions.len() {
                let (xb, yb) = positions[j];
                let dx = xb - xa;
                let dy = yb - ya;
                let mut x = xa;
                let mut y = ya;
                while add_antinode(&mut antinodes, x, y, nx, ny) {
                    x -= dx;
                    y -= dy;
                }
                x = xb;
                y = yb;
                while add_antinode(&mut antinodes, x, y, nx, ny) {
                    x += dx;
                    y += dy;
                }
            }
        }
    }
    antinodes.len()
}

fn main() {
    let args = env::args().nth(1).expect("Please provide an input file");
    let file = File::open(args).unwrap();
    let reader = BufReader::new(file);
    let mut ny = 0;
    let mut nx = 0;
    let mut antennas: HashMap<String, Vec<(i32, i32)>> = HashMap::new();

    for line in reader.lines() {
        let line = line.unwrap();
        nx = line.len();
        for (x, c) in line.chars().enumerate() {
            if c == '.' {
                continue;
            }
            antennas
                .entry(c.to_string())
                .or_default()
                .push((x as i32, ny as i32));
        }
        ny += 1;
    }
    let part1 = count_antinodes(&antennas, nx as i32, ny as i32);
    println!("Part 1: {}", part1);
    let part2 = count_antinodes2(&antennas, nx as i32, ny as i32);
    println!("Part 2: {}", part2);
}
