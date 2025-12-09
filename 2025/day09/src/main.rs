use anyhow::Result;
use std::env;
use std::fs::File;
use std::io::{BufRead, BufReader};

pub fn rectangle_area(a: (u64, u64), b: (u64, u64)) -> u64 {
    (a.0.abs_diff(b.0) + 1) * (a.1.abs_diff(b.1) + 1)
}

fn main() -> Result<()> {
    let args = env::args().nth(1).expect("Please provide an input file");
    let file = File::open(args)?;
    let reader = BufReader::new(file);
    let mut points: Vec<(u64, u64)> = Vec::new();

    let mut xmin = u64::MAX;
    let mut xmax = u64::MIN;
    let mut ymin = u64::MAX;
    let mut ymax = u64::MIN;

    for line in reader.lines() {
        let line = line?;
        let (x_, y_) = line.split_once(",").unwrap();
        let x = x_.parse()?;
        let y = y_.parse()?;
        xmin = xmin.min(x);
        ymin = ymin.min(y);
        xmax = xmax.max(x);
        ymax = ymax.max(y);
        points.push((x, y));
    }
    println!("{} {} {} {}", xmin, xmax, ymin, ymax);
    let mut max_area = 0;
    for i in 0..points.len() {
        for j in i + 1..points.len() {
            max_area = max_area.max(rectangle_area(points[i], points[j]));
        }
    }

    println!("Part 1: {}", max_area);

    // for pnts in [&mut ul, &mut ur, &mut ll, &mut lr] {
    //     pnts.sort_by(|a, b| {
    //         let dista = yo.abs_diff(a.1) + xo.abs_diff(a.0);
    //         let distb = yo.abs_diff(b.1) + xo.abs_diff(b.0);
    //         distb.cmp(&dista)
    //     });
    // }

    Ok(())
}
