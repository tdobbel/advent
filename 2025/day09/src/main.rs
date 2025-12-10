use anyhow::Result;
use std::env;
use std::fs::File;
use std::io::{BufRead, BufReader};

pub struct VerticalEdge {
    x: u64,
    y0: u64,
    y1: u64,
}

impl VerticalEdge {
    pub fn crossed(&self, a: (u64, u64), b: (u64, u64)) -> bool {
        let xmin = a.0.min(b.0);
        let xmax = a.0.max(b.0);
        let ymin = a.1.min(b.1);
        let ymax = a.1.max(b.1);
        let ytop = self.y0.min(self.y1);
        let ybot = self.y0.max(self.y1);
        if self.x <= xmin || self.x >= xmax {
            return false;
        }
        if ymin >= ybot || ymax <= ytop {
            return false;
        }
        true
    }
}

pub struct HorizontalEdge {
    y: u64,
    x0: u64,
    x1: u64,
}

impl HorizontalEdge {
    pub fn crossed(&self, a: (u64, u64), b: (u64, u64)) -> bool {
        let xmin = a.0.min(b.0);
        let xmax = a.0.max(b.0);
        let ymin = a.1.min(b.1);
        let ymax = a.1.max(b.1);
        let xleft = self.x0.min(self.x1);
        let xright = self.x0.max(self.x1);
        if self.y <= ymin || self.y >= ymax {
            return false;
        }
        if xmax <= xleft || xmin >= xright {
            return false;
        }
        true
    }

    pub fn inside(&self, dy: i32, clockwise: bool) -> bool {
        let dx = self.x1 as i32 - self.x0 as i32;
        dx * dy >= 0 && clockwise
    }
}

pub fn valid_rectangle(
    a: (u64, u64),
    b: (u64, u64),
    clockwise: bool,
    v_edges: &[VerticalEdge],
    h_edges: &[HorizontalEdge],
) -> bool {
    let dy = b.1 as i32 - a.1 as i32;
    for edge in h_edges.iter() {
        if edge.y == a.1 && !edge.inside(dy, clockwise) {
            return false;
        }
        if edge.y == b.1 && !edge.inside(-dy, clockwise) {
            return false;
        }
        if edge.crossed(a, b) {
            return false;
        }
    }
    for edge in v_edges.iter() {
        if edge.crossed(a, b) {
            return false;
        }
    }
    true
}

pub fn rectangle_area(a: (u64, u64), b: (u64, u64)) -> u64 {
    (a.0.abs_diff(b.0) + 1) * (a.1.abs_diff(b.1) + 1)
}

fn main() -> Result<()> {
    let args = env::args().nth(1).expect("Please provide an input file");
    let file = File::open(args)?;
    let reader = BufReader::new(file);
    let mut points: Vec<(u64, u64)> = Vec::new();

    let mut xmin = u64::MAX;
    for line in reader.lines() {
        let line = line?;
        let (x_, y_) = line.split_once(",").unwrap();
        let x = x_.parse()?;
        let y = y_.parse()?;
        xmin = xmin.min(x);
        points.push((x, y));
    }

    let mut v_edges: Vec<VerticalEdge> = Vec::new();
    let mut h_edges: Vec<HorizontalEdge> = Vec::new();
    for i in 0..points.len() {
        let a = points[i];
        let b = points[(i + 1) % points.len()];
        if a.0 == b.0 {
            v_edges.push(VerticalEdge {
                x: a.0,
                y0: a.1,
                y1: b.1,
            });
        } else {
            h_edges.push(HorizontalEdge {
                y: a.1,
                x0: a.0,
                x1: b.0,
            });
        }
    }
    let mut ileftmost = 0;
    for (i, edge) in v_edges.iter().enumerate() {
        if edge.x == xmin {
            ileftmost = i;
            break;
        }
    }
    let edge = &v_edges[ileftmost];
    let clockwise = edge.y0 > edge.y1;
    let mut part1 = 0;
    let mut part2 = 0;
    for i in 0..points.len() {
        for j in i + 1..points.len() {
            let area = rectangle_area(points[i], points[j]);
            part1 = part1.max(area);
            if valid_rectangle(points[i], points[j], clockwise, &v_edges, &h_edges) {
                part2 = part2.max(area);
            }
        }
    }

    println!("Part 1: {}", part1);
    println!("Part 2: {}", part2);

    Ok(())
}
