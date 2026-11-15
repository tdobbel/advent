use anyhow::{Result, anyhow};
use std::{collections::HashSet, env, fs};

struct Point {
    x: usize,
    y: usize,
}

fn main() -> Result<()> {
    let filename = env::args().nth(1).expect("Missing input file\n");
    let content = fs::read_to_string(filename)?;
    let lines: Vec<&str> = content.split("\n").collect();

    let mut walls: Vec<Vec<Point>> = Vec::new();

    let mut xmin = usize::MAX;
    let mut xmax = 0;
    let mut ymax = 0;
    for line in lines.iter() {
        if line.is_empty() {
            continue;
        }
        let parts: Vec<&str> = line.split(" -> ").collect();
        let mut wall: Vec<Point> = Vec::new();
        for point_str in parts.iter() {
            let (left, right) = point_str.split_once(",").unwrap();
            let x = left.parse()?;
            let y = right.parse()?;
            xmin = xmin.min(x);
            xmax = xmax.max(x);
            ymax = ymax.max(y);
            wall.push(Point { x, y });
        }
        for i in 1..wall.len() {
            let ok = wall[i].x == wall[i - 1].x || wall[i].y == wall[i - 1].y;
            if !ok {
                return Err(anyhow!("Unexpected wall"));
            }
        }
        walls.push(wall);
    }

    let nx = xmax - xmin + 1;
    let ny = ymax + 1;
    let mut grid: Vec<Vec<char>> = vec![vec!['.'; nx]; ny];
    for wall in walls.iter() {
        add_wall(&mut grid, wall, xmin);
    }
    let mut used: HashSet<(i32, i32)> = HashSet::new();
    for (y, row) in grid.iter().enumerate() {
        for (x, c) in row.iter().enumerate() {
            if c == &'#' {
                used.insert((x as i32, y as i32));
            }
        }
    }

    let mut part1 = 0;
    while drop_sand(&mut grid, 500 - xmin, 0) {
        part1 += 1;
    }
    println!("Part 1: {}", part1);

    let ground_level = (ymax + 2) as i32;
    let mut part2 = 0;
    while drop_sand_v2(&mut used, 500 - xmin as i32, 0, ground_level) {
        part2 += 1;
    }
    println!("Part 2: {}", part2 + 1);

    Ok(())
}

fn add_wall(grid: &mut [Vec<char>], wall: &[Point], xmin: usize) {
    let mut p0 = &wall[0];
    for i in 1..wall.len() {
        let p1 = &wall[i];
        if p0.x == p1.x {
            let y_start = p0.y.min(p1.y);
            let y_stop = p0.y.max(p1.y);
            for y in y_start..=y_stop {
                grid[y][p0.x - xmin] = '#';
            }
        } else {
            let x_start = p0.x.min(p1.x) - xmin;
            let x_stop = p0.x.max(p1.x) - xmin;
            for x in x_start..=x_stop {
                grid[p0.y][x] = '#';
            }
        }
        p0 = p1;
    }
}

fn drop_sand(grid: &mut [Vec<char>], xo: usize, yo: usize) -> bool {
    let mut x: usize = xo;
    let mut y: usize = yo;
    loop {
        if y + 1 == grid.len() {
            return false;
        }
        if grid[y + 1][x] == '.' {
            y += 1;
            continue;
        }
        // Here, we must go left
        if x == 0 {
            return false;
        }
        if grid[y + 1][x - 1] == '.' {
            x -= 1;
            y += 1;
            continue;
        }
        // Here, we must go right
        if x + 1 == grid[0].len() {
            return false;
        }
        if grid[y + 1][x + 1] == '.' {
            x += 1;
            y += 1;
            continue;
        }
        // We are blocked;
        grid[y][x] = 'o';
        return true;
    }
}

fn drop_sand_v2(used: &mut HashSet<(i32, i32)>, xo: i32, yo: i32, ground_level: i32) -> bool {
    let mut x = xo;
    let mut y = yo;
    loop {
        // Cannot go lower than that
        if y + 1 == ground_level {
            break;
        }
        if !used.contains(&(x, y + 1)) {
            y += 1;
            continue;
        }
        // Check if available left
        if !used.contains(&(x - 1, y + 1)) {
            x -= 1;
            y += 1;
            continue;
        }
        // Check if available right
        if !used.contains(&(x + 1, y + 1)) {
            x += 1;
            y += 1;
            continue;
        }
        // We are blocked
        break;
    }
    used.insert((x, y));
    !(x == xo && y == yo)
}
