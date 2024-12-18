use std::fs::File;
use std::io::{BufRead, BufReader};
use std::collections::HashMap;
use std::env;

fn distance(a: &(i32, i32), b: &(i32,i32)) -> i32 {
    (a.0-b.0).abs() + (a.1-b.1).abs()
}

fn plot_path(path: &Vec<(i32,i32)>, n: &i32, corrupted: &Vec<(i32,i32)>) {
    let mut symbols = HashMap::<(i32,i32),char>::new();
    for i in 0..path.len()-1 {
        let a = path[i];
        let b = path[i+1];
        let c: char;
        if a.0 == b.0 {
            c = if a.1 < b.1 { 'v' } else { '^' };
        } else {
            c = if a.0 < b.0 { '>' } else { '<' };
        }
        symbols.insert(a, c);
    }
    symbols.insert(path[path.len()-1], 'O');
    for y in 0..*n {
        for x in 0..*n {
            match symbols.get(&(x,y)) {
                Some(c) => print!("{}", c),
                None => {
                    if corrupted.contains(&(x,y)) {
                        print!("#");
                    } else {
                        print!(".");
                    }
                },
            }
        }
        println!();
    }
}

fn get_next_moves(path: &Vec<(i32,i32)>, n: &i32, corrupted: &Vec<(i32,i32)>, closed: &mut Vec<(i32,i32)>) -> Vec<Vec<(i32,i32)>> {
    let mut new_paths = Vec::<Vec<(i32,i32)>>::new();
    let pos = path.last().unwrap();
    for (shiftx, shifty) in [(-1,0), (0,-1), (1,0), (0,1)].iter() {
        let x = pos.0 + shiftx;
        let y = pos.1 + shifty;
        if x < 0 || x >= *n || y < 0 || y >= *n {
            continue
        }
        if corrupted.contains(&(x,y)) || path.contains(&(x,y)) || closed.contains(&(x,y)) {
            continue
        }
        closed.push((x,y));
        let mut new_path = path.clone();
        new_path.push((x,y));
        new_paths.push(new_path);
    }
    new_paths
}

fn find_shortest_path(
    queue: &mut Vec<Vec<(i32,i32)>>, n: &i32, corrupted: &Vec<(i32,i32)>, closed: &mut Vec<(i32,i32)>
) -> Option<Vec<(i32,i32)>> {
    while queue.len() > 0 {
        let mut ibest: usize = 0;
        let mut best_score = i32::MAX;
        for (i,path) in queue.iter().enumerate() {
            let last = path.last().unwrap();
            let score = 10*(path.len() as i32) + distance(last, &(n-1,n-1));
            if score < best_score {
                best_score = score;
                ibest = i;
            }
        }
        let best = &queue[ibest];
        let last = best.last().unwrap();
        if  last.0 == *n-1 && last.1 == *n-1 {
            return Some(best.clone())
        }
        closed.push(*last);
        let new_paths = get_next_moves(best, n, corrupted, closed);
        queue.remove(ibest);
        for path in new_paths {
            queue.push(path);
        }
    }
    None
}

fn main() {
    let args: Vec<String> = env::args().collect();
    assert_eq!(args.len(), 2);
    let filename = &args[1];
    let n;
    let n_sel;
    if filename == "example" {
        (n, n_sel) = (7, 12);
    }
    else if filename == "input" {
        (n, n_sel) = (71, 1024);
    }
    else {
        panic!("Unexpected file: {}", filename)
    }
    let file = File::open(filename).unwrap();
    let reader = BufReader::new(file);
    let mut bytes = Vec::<(i32, i32)>::new();
    let mut n_byte = 0;
    for line in reader.lines() {
        let line = line.unwrap();
        let xy = line.split(",").map(|x| x.parse::<i32>().unwrap()).collect::<Vec<i32>>();
        bytes.push((xy[0], xy[1]));
        n_byte += 1;
    }
    let corrupted = &bytes[..n_sel].to_vec().clone();
    let mut closed = Vec::<(i32,i32)>::new();
    let mut path = Vec::<(i32,i32)>::new();
    path.push((0,0));
    let mut queue = vec![path];
    let path = find_shortest_path(&mut queue, &n, &corrupted, &mut closed);
    let path = path.unwrap();
    plot_path(&path, &n, &corrupted);
    println!("Part 1: {} steps needed to get to the exit", path.len()-1);
    let mut start = n_sel;
    let mut stop = n_byte;
    while stop-start > 1 {
        let mid = (start+stop)/2;
        closed = closed[..0].to_vec();
        queue = vec![vec![(0,0)]];
        let corrupted = &bytes[..mid+1].to_vec().clone();
        match find_shortest_path(&mut queue, &n, &corrupted, &mut closed) {
            Some(_) => {
                start = mid+1;
            },
            None => {
                stop = mid;
            }
        };
    }
    println!("Part 2: Unable to reah exit with byte {:?}", bytes[stop]);
}
