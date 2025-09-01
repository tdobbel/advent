use anyhow::Result;
use std::env;
use std::fs::File;
use std::io::{BufRead, BufReader};
mod maze;

fn main() -> Result<()> {
    let filename = env::args().nth(1).expect("No input file provided");
    let file = File::open(filename)?;
    let mut ny = 0;
    let reader = BufReader::new(file);
    let mut start_pos = (usize::MAX, usize::MAX);
    let mut end_pos = (usize::MAX, usize::MAX);
    let mut is_wall: Vec<Vec<bool>> = Vec::new();
    for line in reader.lines() {
        let line = line?;
        let nx = line.len();
        let mut row = vec![false; nx];
        for (x, c) in line.chars().enumerate() {
            match c {
                'S' => start_pos = (x, ny),
                'E' => end_pos = (x, ny),
                '#' => row[x] = true,
                '.' => continue,
                _ => panic!("Invalid character"),
            }
        }
        ny += 1;
        is_wall.push(row);
    }

    let mut heading = maze::Direction::East;
    let shortest_path =
        maze::find_shortest_path(start_pos, end_pos, &heading, 0, &is_wall, None).unwrap();
    println!("Part 1: {}", shortest_path.get_score());

    let mut is_shortest: Vec<Vec<bool>> = vec![vec![false; is_wall[0].len()]; is_wall.len()];
    let positions = shortest_path.get_positions();
    for p in positions.iter() {
        is_shortest[p.1][p.0] = true;
    }
    let min_score = shortest_path.get_score();
    let mut base_score = 0;
    let n = positions.len();
    let mut start = start_pos;
    for i in 0..n - 2 {
        let (x, y) = positions[i + 1];
        is_wall[y][x] = true;
        let shortest = maze::find_shortest_path(
            start,
            end_pos,
            &heading,
            base_score,
            &is_wall,
            Some(min_score),
        );
        if let Some(path) = shortest {
            for p in path.get_positions().iter() {
                is_shortest[p.1][p.0] = true;
            }
        }
        is_wall[y][x] = false;
        let new_heading = maze::direction_from_positions(start, (x, y));
        base_score += 1;
        if new_heading != heading {
            base_score += 1000;
            heading = new_heading;
        }
        start = (x, y);
    }

    let mut part2 = 0;
    for row in is_shortest.iter() {
        for &cell in row.iter() {
            if cell {
                part2 += 1;
            }
        }
    }

    println!("Part 2: {}", part2);

    Ok(())
}
