use anyhow::Result;
use octree::Octree;
use std::cmp::Ordering;
use std::collections::BTreeSet;
use std::env;
use std::fs::File;
use std::io::{BufRead, BufReader};

struct Junction {
    a: usize,
    b: usize,
    length: f64,
}

impl Ord for Junction {
    fn cmp(&self, other: &Self) -> Ordering {
        self.length.total_cmp(&other.length)
    }
}

impl PartialOrd for Junction {
    fn partial_cmp(&self, other: &Self) -> Option<Ordering> {
        Some(self.cmp(other))
    }
}

impl PartialEq for Junction {
    fn eq(&self, other: &Self) -> bool {
        (self.a, self.b) == (other.a, other.b)
    }
}

impl Eq for Junction {}

pub fn add_junction(circuits: &mut Vec<Vec<usize>>, pair: (usize, usize)) {
    let mut ia = circuits.len();
    let mut ib = circuits.len();
    let (a, b) = pair;
    for i in 0..circuits.len() {
        if circuits[i].contains(&a) {
            ia = i;
        }
        if circuits[i].contains(&b) {
            ib = i;
        }
    }
    if ia == ib {
        return;
    }
    let circuit_right = circuits.remove(ia.max(ib));
    circuits[ia.min(ib)].extend(circuit_right);
}

fn main() -> Result<()> {
    let args = env::args().nth(1).expect("Please provide an input file");
    let file = File::open(args)?;
    let reader = BufReader::new(file);
    let mut points: Vec<[f64; 3]> = Vec::new();

    for line in reader.lines() {
        let line = line?;
        let mut point: [f64; 3] = Default::default();
        for (i, v) in line.split(",").enumerate() {
            point[i] = v.parse::<f64>().unwrap();
        }
        points.push(point);
    }
    let mut xmin = f64::MAX;
    let mut xmax = f64::MIN;
    let mut ymin = f64::MAX;
    let mut ymax = f64::MIN;
    let mut zmin = f64::MAX;
    let mut zmax = f64::MIN;

    for p in points.iter() {
        xmin = xmin.min(p[0]);
        xmax = xmax.max(p[0]);
        ymin = ymin.min(p[1]);
        ymax = ymax.max(p[1]);
        zmin = zmin.min(p[2]);
        zmax = zmax.max(p[2]);
    }
    let max_dist = (xmax - xmin).max(ymax - ymin).max(zmax - zmin);
    let tree = Octree::new(points);
    let mut connections: BTreeSet<Junction> = BTreeSet::new();
    for i in 0..tree.points.len() {
        let resp = tree.search(tree.points[i], 0.5 * max_dist);
        for (j, dist) in resp {
            if i == j {
                continue;
            }
            let jun = Junction {
                a: i.min(j),
                b: i.max(j),
                length: dist,
            };
            connections.insert(jun);
        }
    }
    let mut circuits: Vec<Vec<usize>> = (0..tree.points.len()).map(|i| vec![i]).collect();
    for _ in 0..1000 {
        let jun = connections.pop_first().unwrap();
        add_junction(&mut circuits, (jun.a, jun.b));
    }
    circuits.sort_by(|a, b| b.len().cmp(&a.len()));
    let part1 = circuits[0].len() * circuits[1].len() * circuits[2].len();
    println!("Part 1: {part1}");

    let mut part2 = 0;
    while circuits.len() > 1 {
        let jun = connections.pop_first().unwrap();
        add_junction(&mut circuits, (jun.a, jun.b));
        part2 = (tree.points[jun.a][0] * tree.points[jun.b][0]) as u64;
    }
    println!("Part 2: {part2}");

    Ok(())
}
