use anyhow::Result;
use regex::Regex;
use std::io::{BufRead, BufReader};
use std::{env, fs::File};

#[derive(Debug, Copy, Clone)]
struct Point {
    x: i64,
    y: i64,
}

struct Sensor {
    position: Point,
    radius: u64,
}

fn main() -> Result<()> {
    let filename = env::args().nth(1).expect("Missing input file");
    let file = File::open(filename)?;
    let reader = BufReader::new(file);
    let pattern =
        Regex::new(r"Sensor at x=(-?\d+), y=(-?\d+): closest beacon is at x=(-?\d+), y=(-?\d+)")?;
    let mut sensors: Vec<Sensor> = Vec::new();
    let mut beacons: Vec<Point> = Vec::new();
    let mut xmin: i64 = i64::MAX;
    let mut xmax: i64 = i64::MIN;
    for line in reader.lines() {
        let line = line?;
        let caps = pattern.captures(&line).unwrap();
        let sensor = Point {
            x: caps.get(1).unwrap().as_str().parse()?,
            y: caps.get(2).unwrap().as_str().parse()?,
        };
        let beacon = Point {
            x: caps.get(3).unwrap().as_str().parse()?,
            y: caps.get(4).unwrap().as_str().parse()?,
        };
        let radius = sensor.distance(beacon);
        xmin = xmin.min(sensor.x - (radius as i64));
        xmax = xmax.max(sensor.x + (radius as i64));
        sensors.push(Sensor {
            position: sensor,
            radius,
        });
        if !beacons.contains(&beacon) {
            beacons.push(beacon);
        }
    }

    let part1 = solve_part1(&sensors, &beacons, 2000000, xmin, xmax);
    println!("Part 1: {}", part1);

    let search_size = 4000000;

    let mut part2 = 0;
    for y in 0..=search_size {
        let mut xmin = 0;
        let mut xmax = search_size;
        let excluded = find_possible_positions(&sensors, y);
        for &(start, stop) in excluded.iter() {
            if xmin >= start && xmin <= stop {
                xmin = xmin.max(stop);
            }
            if xmax >= start && xmax <= stop {
                xmax = xmax.min(start);
            }
        }
        if xmax < xmin + 2 {
            continue;
        }
        part2 = (xmin + 1) * 4000000 + y;
        break;
    }
    println!("Part 2: {}", part2);

    Ok(())
}

impl PartialEq for Point {
    fn eq(&self, other: &Self) -> bool {
        self.x == other.x && self.y == other.y
    }
}

impl Point {
    fn distance(&self, other: Point) -> u64 {
        self.x.abs_diff(other.x) + self.y.abs_diff(other.y)
    }
}

fn solve_part1(sensors: &Vec<Sensor>, beacons: &Vec<Point>, y: i64, xmin: i64, xmax: i64) -> u64 {
    let mut ret = 0;
    for x in xmin..=xmax {
        let point = Point { x, y };
        let mut within_radius: bool = false;
        for s in sensors.iter() {
            if s.position.distance(point) <= s.radius {
                within_radius = true;
                break;
            }
        }
        if within_radius {
            ret += 1;
        }
    }

    for b in beacons.iter() {
        if b.y == y {
            ret -= 1;
        }
    }
    ret
}

fn find_possible_positions(sensors: &Vec<Sensor>, y: i64) -> Vec<(i64, i64)> {
    let mut nok: Vec<(i64, i64)> = Vec::new();
    for s in sensors.iter() {
        let dy = s.position.y.abs_diff(y);
        if dy > s.radius {
            continue;
        }
        let dx = (s.radius - dy) as i64;
        let mut start = s.position.x - dx;
        let mut stop = s.position.x + dx;
        let mut added: bool = false;
        let mut next = Vec::new();
        for i in nok.iter() {
            if i.1 < start {
                // we are
                next.push(*i);
            } else if i.0 > stop {
                if !added {
                    next.push((start, stop));
                    added = true;
                }
                next.push(*i);
            } else {
                start = start.min(i.0);
                stop = stop.max(i.1);
            }
        }
        if !added {
            next.push((start, stop));
        }
        nok = next;
    }
    nok
}
