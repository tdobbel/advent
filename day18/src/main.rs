use std::fs::File;
use std::io::{BufRead, BufReader};
use std::env;

#[derive(Debug,Clone)]
struct Tile {
    pos: (i32, i32),
    visited: Vec<(i32,i32)>,
}

fn distance(a: &(i32, i32), b: &(i32,i32)) -> i32 {
    (a.0-b.0).abs() + (a.1-b.1).abs()
}

fn plot_path(path: &Vec<(i32,i32)>, n: &i32, corrupted: &Vec<(i32,i32)>) {
    for y in 0..*n {
        for x in 0..*n {
            if corrupted.contains(&(x,y)) {
                print!("#");
            } else if path.contains(&(x,y)) {
                print!("0");
            } else {
                print!(".");
            }
        }
        println!();
    }
}

fn get_next_moves(tile: &Tile, n: &i32, corrupted: &Vec<(i32,i32)>, closed: &Vec<(i32,i32)>) -> Vec<Tile> {
    let mut next_tiles = Vec::<Tile>::new();
    for (shiftx, shifty) in [(-1,0), (0,-1), (1,0), (0,1)].iter() {
        let x = tile.pos.0 + shiftx;
        let y = tile.pos.1 + shifty;
        if x < 0 || x >= *n || y < 0 || y >= *n {
            continue
        }
        if corrupted.contains(&(x,y)) || closed.contains(&(x,y)) || tile.visited.contains(&(x,y)) {
            continue
        }
        let mut visited = tile.visited.clone();
        visited.push(tile.pos);
        next_tiles.push(Tile {pos: (x,y), visited});
    }
    next_tiles
}

fn find_shortest_path(
    tiles: &mut Vec<Tile>, n: &i32, corrupted: &Vec<(i32,i32)>, closed: &mut Vec<(i32,i32)>, path: &mut Vec<(i32,i32)>
) {
    while tiles.len() > 0 {
        let mut ibest: usize = 0;
        let mut best_score = i32::MAX;
        for (i,tile) in tiles.iter().enumerate() {
            let score = tile.visited.len() as i32 + distance(&tile.pos, &(*n-1,*n-1));
            if score < best_score {
                best_score = score;
                ibest = i;
            }
        }
        let best = &tiles[ibest];
        if best.pos.0 == *n-1 && best.pos.1 == *n-1 {
            for pos in best.visited.iter() {
                path.push(*pos);
            }
            path.push(best.pos);
            break;
        }
        let new_tiles = get_next_moves(best, n, corrupted, closed);
        closed.push(best.pos);
        tiles.remove(ibest);
        for tile in new_tiles {
            tiles.push(tile);
        }
    }
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
        (n, n_sel) = (71, 1034);
    }
    else {
        panic!("Unexpected file: {}", filename)
    }
    let file = File::open(filename).unwrap();
    let reader = BufReader::new(file);
    let mut bytes = Vec::<(i32, i32)>::new();
    for line in reader.lines() {
        let line = line.unwrap();
        let xy = line.split(",").map(|x| x.parse::<i32>().unwrap()).collect::<Vec<i32>>();
        bytes.push((xy[0], xy[1]));
    }
    let corrupted = &bytes[..n_sel].to_vec().clone();
    let mut closed = Vec::<(i32,i32)>::new();
    let mut tiles = Vec::<Tile>::new();
    let mut path = Vec::<(i32,i32)>::new();
    tiles.push(Tile {pos: (0,0), visited: Vec::<(i32,i32)>::new()});
    find_shortest_path(&mut tiles, &n, &corrupted, &mut closed, &mut path);
    plot_path(&path, &n, &corrupted);
    println!("Part 1: {} steps needed to get to the exit", path.len());
}
