use std::collections::HashMap;
use std::env;
use std::fs::File;
use std::io::{BufRead, BufReader};

fn distance(a: &(i32, i32), b: &(i32, i32)) -> i32 {
    (a.0 - b.0).abs() + (a.1 - b.1).abs()
}

#[allow(dead_code)]
fn plot_path(path: &[(i32, i32)], walls: &[(i32, i32)], nx: i32, ny: i32) {
    let mut symbols = HashMap::<(i32, i32), char>::new();
    let start = path[0];
    let end = path[path.len() - 1];
    let mut n_cheat = 0;
    for i in 1..path.len() - 1 {
        let a = path[i];
        let b = path[i + 1];
        let c: char = if a.0 == b.0 {
            if a.1 < b.1 {
                'v'
            } else {
                '^'
            }
        } else if a.0 < b.0 {
                '>'
        } else {
            '<'
        };
        symbols.insert(a, c);
    }
    for y in 0..ny {
        for x in 0..nx {
            if x == start.0 && y == start.1 {
                print!("S");
                continue;
            }
            if x == end.0 && y == end.1 {
                print!("E");
                continue;
            }
            match symbols.get(&(x, y)) {
                Some(c) => {
                    if walls.contains(&(x, y)) {
                        n_cheat += 1;
                        print!("{}", n_cheat);
                    } else {
                        print!("{}", c);
                    }
                }
                None => {
                    if walls.contains(&(x, y)) {
                        print!("#");
                    } else {
                        print!(".");
                    }
                }
            }
        }
        println!();
    }
}

fn get_next_moves(
    path: &[(i32, i32)],
    closed: &mut Vec<(i32, i32)>,
    walls: &[(i32, i32)],
    nx: i32,
    ny: i32,
) -> Vec<Vec<(i32, i32)>> {
    let mut new_paths = Vec::<Vec<(i32, i32)>>::new();
    let pos = path.last().unwrap();
    for (shiftx, shifty) in [(-1, 0), (0, -1), (1, 0), (0, 1)].iter() {
        let x = pos.0 + shiftx;
        let y = pos.1 + shifty;
        if x < 0 || x >= nx || y < 0 || y >= ny || closed.contains(&(x, y)) {
            continue;
        }
        if walls.contains(&(x, y)) {
            continue;
        }
        closed.push((x, y));
        let mut tiles = path.to_owned();
        tiles.push((x, y));
        new_paths.push(tiles);
    }
    new_paths
}

fn find_shortest_path(
    queue: &mut Vec<Vec<(i32, i32)>>,
    closed: &mut Vec<(i32, i32)>,
    walls: &[(i32, i32)],
    end: (i32, i32),
    nx: i32,
    ny: i32,
) -> Option<Vec<(i32, i32)>> {
    while !queue.is_empty() {
        let mut ibest: usize = 0;
        let mut best_score = i32::MAX;
        for (i, path) in queue.iter().enumerate() {
            let pos = path.last().unwrap();
            let score = (path.len() as i32) + distance(pos, &end);
            if score < best_score {
                best_score = score;
                ibest = i;
            }
        }
        let best = &queue[ibest];
        let tail = best.last().unwrap();
        if tail.0 == end.0 && tail.1 == end.1 {
            return Some(best.clone());
        }
        closed.push(*tail);
        let new_paths = get_next_moves(best, closed, walls, nx, ny);
        queue.remove(ibest);
        for path in new_paths {
            queue.push(path);
        }
    }
    None
}

fn count_shortcuts(path: &[(i32, i32)], min_save: usize, n_cheat: usize) -> HashMap<usize, i32> {
    let mut counter = HashMap::<usize, i32>::new();
    for i in 0..path.len() - min_save {
        for j in i + min_save + 1..path.len() {
            let distance = distance(&path[i], &path[j]) as usize;
            if distance > n_cheat {
                continue;
            }
            let save = j - i - distance;
            if save >= min_save {
                let cntr = counter.entry(save).or_insert(0);
                *cntr += 1;
            }
        }
    }
    counter
}

fn main() {
    let args: Vec<String> = env::args().collect();
    assert_eq!(args.len(), 2);
    let file = File::open(&args[1]).unwrap();
    let reader = BufReader::new(file);
    let mut walls = Vec::<(i32, i32)>::new();
    let mut start = (0, 0);
    let mut end = (0, 0);
    let mut nx = 0;
    let mut ny = 0;
    for (y, line) in reader.lines().enumerate() {
        let line = line.unwrap();
        nx = line.len() as i32;
        for (x, c) in line.chars().enumerate() {
            match c {
                '#' => walls.push((x as i32, y as i32)),
                'E' => end = (x as i32, y as i32),
                'S' => start = (x as i32, y as i32),
                _ => (),
            }
        }
        ny += 1;
    }
    let mut queue = vec![vec![start]];
    let mut closed = Vec::<(i32, i32)>::new();
    let path = find_shortest_path(&mut queue, &mut closed, &walls, end, nx, ny);
    let path = path.unwrap();
    plot_path(&path, &walls, nx, ny);
    println!(
        "The program can complete the course un {} picoseconds",
        path.len() - 1
    );
    let cheats: Vec<usize> = vec![2, 20];
    for n_cheat in cheats.iter() {
        println!("Total number of cheats: {}", *n_cheat);
        let shortcuts = count_shortcuts(&path, 100, *n_cheat);
        let mut total = 0;
        for (_, count) in shortcuts.iter() {
            //println!("There are {} cheats that save {} picoseconds", count, save);
            total += count;
        }
        println!(
            "There are {} cheats that save at least 100 picoseconds",
            total
        );
    }
}
