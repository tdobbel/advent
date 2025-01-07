use std::cmp::{max, min};
use std::env;
use std::fs::File;
use std::io::{BufRead, BufReader};

fn sum_distances(
    galaxies: &[(usize, usize)],
    grows: &[bool],
    gcols: &[bool],
    expansion: u64,
) -> u64 {
    let mut total: u64 = 0;
    for i in 0..galaxies.len() - 1 {
        for j in i + 1..galaxies.len() {
            let g1 = galaxies[i];
            let g2 = galaxies[j];
            let dx = (g2.0 as isize - g1.0 as isize).abs();
            let dy = (g2.1 as isize - g1.1 as isize).abs();
            let mut dist = dx as u64 + dy as u64;
            let y_from = min(g1.1, g2.1);
            let y_to = max(g1.1, g2.1);
            for has_galaxies in grows[y_from..y_to].iter() {
                if !has_galaxies {
                    dist += expansion - 1;
                }
            }
            let x_from = min(g1.0, g2.0);
            let x_to = max(g1.0, g2.0);
            for has_galaxies in gcols[x_from..x_to].iter() {
                if !has_galaxies {
                    dist += expansion - 1;
                }
            }
            total += dist;
        }
    }
    total
}

fn main() {
    let args = env::args().nth(1).expect("Please provide an input file");
    let file = File::open(args).unwrap();
    let reader = BufReader::new(file);
    let mut galaxies = Vec::<(usize, usize)>::new();
    let mut nx = 0;
    let mut ny = 0;
    for (y, line) in reader.lines().enumerate() {
        ny += 1;
        let line = line.unwrap();
        nx = line.len();
        for (x, c) in line.chars().enumerate() {
            if c == '#' {
                galaxies.push((x, y))
            }
        }
    }
    let mut gcols = vec![false; nx];
    let mut grows = vec![false; ny];
    for (x, y) in galaxies.iter() {
        gcols[*x] = true;
        grows[*y] = true;
    }
    let total1 = sum_distances(&galaxies, &grows, &gcols, 2);
    let total2 = sum_distances(&galaxies, &grows, &gcols, 1000000);
    println!("{}", total1);
    println!("{}", total2);
}
