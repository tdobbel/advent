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
) -> Vec<(i32,i32)> {
    let mut prev = (start.0, start.1);
    let mut pos = (0, 0);
    let mut path = vec![(start.0,start.1)];
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
        path.push((pos.0,pos.1));
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
    } 
    path
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
    let path = run_loop(&grid, &pipes, nx, ny, &start);
    let clockwise = (0..path.len()-1).map(|i| {
        let x = path[i];
        let y = path[i+1];
        return (y.0-x.0)*(y.1-x.1)
    }).sum::<i32>() > 0;
    let n = path.len();
    let mut n_enclosed = 0;
    for y in 0..ny {
        let mut enclosed = false;
        for x in 0..nx {
            let pos = (x,y);
            let index = path.iter().position(|&x| x.0 == pos.0 && x.1 == pos.1);
            match index {
                Some(i) => {
                    let prev = if i == 0 {n-1} else {i-1};
                    let next = (i+1)%n;
                    for (i_,j_) in [(prev,i), (i, next)].iter() {
                        let dy = path[*j_].1-path[*i_].1;
                        if dy == 0 {
                            continue
                        }
                        enclosed = if clockwise { dy < 0 } else { dy > 0 };
                        break
                    }
                },
                None => {
                    if enclosed {
                        n_enclosed += 1;
                    }
                }
            };
        }
    }
    println!("Part 1: {}", n / 2);
    println!("Part 2: {}", n_enclosed);
}

