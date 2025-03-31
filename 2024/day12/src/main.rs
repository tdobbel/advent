use anyhow::Result;
use std::env::args;
use std::fs::File;
use std::io::{BufRead, BufReader};

enum Direction {
    North,
    East,
    South,
    West,
}

struct Fence {
    a: (usize, usize),
    b: (usize, usize),
}

impl Fence {
    pub fn as_vector(&self) -> (isize, isize) {
        let (x1, y1) = self.a;
        let (x2, y2) = self.b;
        (x2 as isize - x1 as isize, y2 as isize - y1 as isize)
    }
}

fn is_corner(f0: &Fence, f1: &Fence) -> bool {
    let (u0, v0) = f0.as_vector();
    let (u1, v1) = f1.as_vector();
    u0 != u1 || v0 != v1
}

fn get_neighbour(
    plots: &[Vec<char>],
    x: usize,
    y: usize,
    side: &Direction,
) -> Option<(usize, usize)> {
    let (x_, y_) = match side {
        Direction::North => (x, y.checked_sub(1)?),
        Direction::East => (x + 1, y),
        Direction::South => (x, y + 1),
        Direction::West => (x.checked_sub(1)?, y),
    };
    if x_ >= plots[0].len() || y_ >= plots.len() {
        None
    } else if plots[y_][x_] != plots[y][x] {
        None
    } else {
        Some((x_, y_))
    }
}

fn fence_from_side(x: usize, y: usize, side: &Direction) -> Fence {
    match side {
        Direction::North => Fence {
            a: (x + 1, y),
            b: (x, y),
        },
        Direction::West => Fence {
            a: (x, y),
            b: (x, y + 1),
        },
        Direction::South => Fence {
            a: (x, y + 1),
            b: (x + 1, y + 1),
        },
        Direction::East => Fence {
            a: (x + 1, y + 1),
            b: (x + 1, y),
        },
    }
}

fn compute_price(
    plots: &[Vec<char>],
    visited: &mut [Vec<bool>],
    x: usize,
    y: usize,
    area: &mut usize,
    perimeter: &mut usize,
    fences: &mut Vec<Fence>,
) {
    visited[y][x] = true;
    *area += 1;
    let sides = [
        Direction::North,
        Direction::East,
        Direction::South,
        Direction::West,
    ];
    for side in sides.iter() {
        let (x_, y_) = match get_neighbour(plots, x, y, side) {
            Some((x_, y_)) => (x_, y_),
            None => {
                *perimeter += 1;
                fences.push(fence_from_side(x, y, side));
                continue;
            }
        };
        if !visited[y_][x_] {
            compute_price(plots, visited, x_, y_, area, perimeter, fences);
        }
    }
}

fn count_corners(fences: &mut [Fence]) -> usize {
    let mut corners = 0;
    let mut icurr = 0;
    let mut first = 0;
    while icurr < fences.len() {
        for i in icurr + 1..fences.len() {
            if fences[icurr].b == fences[i].a {
                if is_corner(&fences[icurr], &fences[i]) {
                    corners += 1;
                }
                fences.swap(icurr + 1, i);
                icurr += 1;
                break;
            }
        }
        if fences[first].a == fences[icurr].b {
            if is_corner(&fences[first], &fences[icurr]) {
                corners += 1;
            }
            icurr += 1;
            first = icurr;
        }
    }
    corners
}

fn main() -> Result<()> {
    let filename = args().nth(1).expect("Missing input file");
    let file = File::open(filename)?;
    let reader = BufReader::new(file);
    let mut plots: Vec<Vec<char>> = Vec::new();
    for line in reader.lines() {
        let line = line?;
        plots.push(line.chars().collect());
    }

    let nx = plots[0].len();
    let ny = plots.len();
    // println!("nx: {}, ny: {}", nx, ny);
    let mut visited = vec![vec![false; nx]; ny];
    let mut part1 = 0;
    let mut part2 = 0;
    for y in 0..ny {
        for x in 0..nx {
            if visited[y][x] {
                continue;
            }
            let mut area = 0;
            let mut perimeter = 0;
            let mut fences = Vec::new();
            compute_price(
                &plots,
                &mut visited,
                x,
                y,
                &mut area,
                &mut perimeter,
                &mut fences,
            );
            part1 += area * perimeter;
            part2 += area * count_corners(&mut fences);
        }
    }

    println!("Part 1: {}", part1);
    println!("Part 2: {}", part2);

    Ok(())
}
