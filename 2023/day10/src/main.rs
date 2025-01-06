use std::env;
use std::collections::HashMap;
use std::fs::File;
use std::io::{BufRead, BufReader};

#[derive(PartialEq)]
enum Direction {
    North,
    East,
    South,
    West,
}

fn direction_from(dx: i32, dy: i32) -> Direction {
    match (dx,dy) {
        (-1,0) => Direction::East,
        (0,-1) => Direction::South,
        (1,0) => Direction::West,
        (0,1) => Direction::North,
        _ => panic!("Invalid direction"),
    }
}

fn run_loop(
    grid: &HashMap<(i32,i32),char>, pipes: &HashMap<char,(Direction,Direction)>, nx: i32, ny: i32, start: &(i32,i32)
) -> usize {
    let mut prev = (start.0, start.1);
    let mut pos = (0, 0);
    let mut n = 1;
    let mut ok = false;
    for (dx,dy) in [(-1,0), (0,-1), (1,0), (0,1)].iter() {
        let x = prev.0 + dx;
        let y = prev.1 + dy;
        if x < 0 || x >= nx || y < 0 || y >= ny {
            continue
        }
        let c = grid.get(&(x,y)).unwrap();
        if *c == '.' {
            continue
        }
        let dfrom = direction_from(*dx, *dy);
        let (d0,d1) = pipes.get(c).unwrap();
        if dfrom != *d0 && dfrom != *d1 {
            continue
        }
        pos = (x,y);
        ok = true;
        break
    }
    if !ok {
        panic!("Could no start loop :(");
    }
    while pos.0 != start.0 || pos.1 != start.1 {
        let dx = pos.0-prev.0;
        let dy = pos.1-prev.1;
        let dfrom = direction_from(dx, dy);
        let c = grid.get(&pos).unwrap();
        let (d0, d1) = pipes.get(c).unwrap();
        let dto: &Direction;
        if dfrom == *d0 {
            dto = d1;
        }
        else if dfrom == *d1 {
            dto = d0;
        }
        else {
            panic!("Somethin went wrong");
        }
        let (dx,dy) = match dto {
            Direction::North => (0,-1),
            Direction::South => (0,1),
            Direction::East => (1,0),
            Direction::West => (-1,0)
        };
        prev = pos;
        pos = (prev.0+dx, prev.1+dy);
        n += 1;
    } 
    n
}

fn main() {
    let args: Vec<String> = env::args().collect();
    let file = File::open(&args[1]).unwrap();
    let reader = BufReader::new(file);
    let mut grid = HashMap::<(i32,i32),char>::new();
    let mut start = (0,0);
    let mut pipes =  HashMap::<char,(Direction,Direction)>::new();
    pipes.insert('|',(Direction::North,Direction::South));
    pipes.insert('-',(Direction::East,Direction::West));
    pipes.insert('L',(Direction::North,Direction::East));
    pipes.insert('J',(Direction::North,Direction::West));
    pipes.insert('7',(Direction::South,Direction::West));
    pipes.insert('F',(Direction::South,Direction::East));
    let mut nx: i32 = 0;
    let mut ny: i32 = 0;
    for (y, line) in reader.lines().enumerate() {
        ny += 1;
        let line = line.unwrap();
        nx = line.len() as i32;
        for (x, c) in line.chars().enumerate() {
            let pos = (x as i32, y as i32);
            grid.insert(pos,c);
            if c == 'S' {
                start = pos; 
            }
        }
    }
    let loop_size = run_loop(&grid, &pipes, nx, ny, &start);
    println!("{}", loop_size / 2);
}

