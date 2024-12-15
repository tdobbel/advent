use std::fs::File;
use std::io::{BufRead, BufReader};
use std::collections::HashMap;
use std::cmp::max;
use std::env;

enum Quadrant {
    TopLeft,
    TopRight,
    BottomLeft,
    BottomRight
}

impl Quadrant {
    const VALUES: [Quadrant; 4] = [Quadrant::TopLeft, Quadrant::TopRight, Quadrant::BottomLeft, Quadrant::BottomRight];
}

#[derive(Clone)]
struct Robot {
    x: i32,
    y: i32,
    v: (i32, i32)
}

#[allow(dead_code)]
fn show_robots(robots: &Vec<Robot>, nx: i32, ny: i32) {
    let mut cntr = HashMap::<(i32, i32), i32>::new();
    for robot in robots.iter() {
        let pos = (robot.x, robot.y);
        *cntr.entry(pos).or_insert(0) += 1;
    }
    for y in 0..ny {
        for x in 0..nx {
            let pos = (x, y);
            let c = cntr.get(&pos).unwrap_or(&0);
            if *c == 0 {
                print!(".");
            } else {
                print!("{}", *c);
            }
        }
        println!();
    }
}

fn iterate(robots: &mut Vec<Robot>, nx: i32, ny: i32) {
    for i in 0..robots.len() {
        let robot = robots.get_mut(i).unwrap();
        let mut x_new = (robot.x + robot.v.0) % nx;
        let mut y_new = (robot.y + robot.v.1) % ny;
        if x_new < 0 {
            x_new += nx;
        }
        if y_new < 0 {
            y_new += ny;
        }
        robot.x = x_new;
        robot.y = y_new;
    }
}

fn count_in_quadrants(robots: &Vec<Robot>, nx: i32, ny: i32) -> Vec<i32> {
    let xmid = nx / 2;
    let ymid = ny / 2;
    let mut counter = vec![0; 4];
    for robot in robots.iter() {
        for (i,quad) in Quadrant::VALUES.iter().enumerate() {
            let inside = match quad {
                Quadrant::TopLeft => robot.x < xmid && robot.y < ymid,
                Quadrant::TopRight => robot.x > xmid && robot.y < ymid,
                Quadrant::BottomLeft => robot.x < xmid && robot.y > ymid,
                Quadrant::BottomRight => robot.x > xmid && robot.y > ymid
            };
            if inside {
                counter[i] += 1;
                break
            }
        }
    }
    counter
}

fn largest_tree_size(robots: &Vec<Robot>, nx: i32, ny: i32) -> i32 {
    let mut cntr = HashMap::<(i32, i32), i32>::new();
    for robot in robots.iter() {
        let pos = (robot.x, robot.y);
        *cntr.entry(pos).or_insert(0) += 1;
    }
    let mut largest = 0;
    for ((x,y),_) in cntr.iter() {
        if *x < 1 || *x >= nx-1 {
            continue
        }
        let mut height = 0;
        let imax = ny-x;
        for i in 0..imax {
            let mut add = true;
            for shift in [-1, 0, 1].iter() {
                let value = cntr.get(&(x+shift, y+i)).unwrap_or(&0);
                if *value == 0 {
                    add = false;
                    break
                }
            }
            if !add {
                break
            }
            else {
                height += 1;
            }
        }
        largest = max(largest, height);
    }
    return largest
}

fn main() {
    let args: Vec<String> = env::args().collect();
    assert_eq!(args.len(), 2);
    let filename = &args[1];
    let nx: i32;
    let ny: i32;
    if filename == "example" {
        nx = 11;
        ny = 7;
    } else {
        nx = 101;
        ny = 103;
    }
    let file = File::open(&args[1]).unwrap();
    let reader = BufReader::new(file);
    let mut robots = Vec::<Robot>::new();
    for line in reader.lines() {
        let line = line.unwrap();
        let isep = line.find(" ").unwrap();
        let pos = line[2..isep].split(",")
            .map(|x| x.parse::<i32>().unwrap())
            .collect::<Vec<i32>>();
        let vel = line[isep+3..line.len()].split(",")
            .map(|x| x.parse::<i32>().unwrap())
            .collect::<Vec<i32>>();
        robots.push(Robot { x: pos[0], y: pos[1], v: (vel[0], vel[1]) });
    }
    // Part 1
    let mut robots1 = robots.clone();
    for _ in 0..100 {
        iterate(&mut robots1, nx, ny);
    }
    let counter = count_in_quadrants(&robots1, nx, ny);
    let mut safety_score = 1;
    for c in counter.iter() {
        safety_score *= c;
    }
    println!("Safety score: {}", safety_score);
    // Part 2
    for i in 0..10000 {
        iterate(&mut robots, nx, ny);
        let tree_size = largest_tree_size(&robots, nx, ny);
        if tree_size > 8 {
            println!("Time to wait: {}", i+1);
            show_robots(&robots, nx, ny);
            break
        }
    }
}
