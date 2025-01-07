use std::env;
use std::fs::File;
use std::io::{BufRead, BufReader};
use std::collections::HashMap;

use anyhow::Result;

#[derive(PartialEq, Clone, Debug)]
enum Orientation {
    Up,
    Down,
    Left,
    Right
}

struct Guard {
    x: i32,
    y: i32,
    orientation: Orientation
}

fn move_guard(
    x0: i32, y0: i32, o0: &Orientation, xmax: i32, ymax: i32, obstacles: &HashMap<(usize,usize), bool>
) -> (HashMap<(usize,usize), Vec<Orientation>>, bool) {
    let mut guard = Guard { x: x0, y: y0, orientation: o0.clone() };
    let mut visited = HashMap::<(usize, usize), Vec<Orientation>>::new();
    let mut looping = false;
    while guard.x < xmax && guard.x >= 0 && guard.y < ymax && guard.y >= 0 {
        visited.entry((guard.y as usize, guard.x as usize))
            .and_modify( |values| {
                if !values.contains(&guard.orientation) {
                    values.push(guard.orientation.clone());
                }
            })
            .or_insert(Vec::<Orientation>::new());
        let (xnew, ynew) = match guard.orientation {
            Orientation::Up => (guard.x, guard.y - 1),
            Orientation::Down => (guard.x, guard.y + 1),
            Orientation::Left => (guard.x - 1, guard.y),
            Orientation::Right => (guard.x + 1, guard.y)
        };
        let blocked = match obstacles.get(&(ynew as usize, xnew as usize)) {
            Some(b) => *b,
            None => false
        };
        if !blocked {
            guard.x = xnew;
            guard.y = ynew;
        }
        else {
            guard.orientation = match guard.orientation {
                Orientation::Up => Orientation::Right,
                Orientation::Down => Orientation::Left,
                Orientation::Left => Orientation::Up,
                Orientation::Right => Orientation::Down
            };
        }
        looping = match visited.get(&(ynew as usize, xnew as usize)) {
            Some(v) => v.contains(&guard.orientation),
            None => false
        };
        if looping {
            break
        }
    }
    (visited, looping)
}

fn main() -> Result<()> {
    let args = env::args().nth(1).expect("Please provide an input file");
    let file = File::open(args)?;
    let reader = BufReader::new(file);
    let mut obstacles = HashMap::<(usize, usize), bool>::new();
    let mut xmax: i32 = 0;
    let mut ymax: i32 = 0;
    let mut x0: i32 = 0;
    let mut y0: i32 = 0;
    let mut o0: Orientation = Orientation::Up;
    for (i,line) in reader.lines().enumerate() {
        let line = line.unwrap();
        xmax = line.chars().count() as i32;
        ymax += 1;
        for (j,case) in line.chars().enumerate() {
            if case == '#' {
                obstacles.insert((i,j), true);
            }
            else if case == '.'{
                continue
            }
            else {
                x0 = j as i32;
                y0 = i as i32;
                o0 = match case {
                    '^' => Orientation::Up,
                    'v' => Orientation::Down,
                    '<' => Orientation::Left,
                    '>' => Orientation::Right,
                    _ => panic!("Invalid character")
                };
            }
        }
    }
    let route = move_guard(x0, y0, &o0, xmax, ymax, &obstacles).0;
    println!("Guard visited {} distinct positions", route.len());
    let mut total = 0;
    for (indx, (pos,_)) in route.iter().enumerate() {
        if indx == 0 {
            continue
        }
        obstacles.insert(*pos, true);
        let looping =  move_guard(x0, y0, &o0, xmax, ymax, &obstacles).1;
        if looping {
            total += 1;
        }
        obstacles.remove(&pos);
    }
    println!("{} possible positions for obstacle", total);
    Ok(())
}
