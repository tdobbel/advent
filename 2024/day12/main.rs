use std::fs::File;
use std::io::{BufRead, BufReader};

enum Direction {
    Up,
    Down,
    Left,
    Right
}

impl Direction {
    const VALUES: [Direction; 4] = [Direction::Up, Direction::Left, Direction::Down, Direction::Right];
}

#[derive(Debug)]
struct Edge {
    a: (i32,i32),
    b: (i32,i32),
}

fn get_neighbour(pos: &(i32, i32), direction: &Direction) -> (i32,i32) {
    return match direction {
        Direction::Up => (pos.0-1, pos.1),
        Direction::Left => (pos.0, pos.1-1),
        Direction::Down => (pos.0+1, pos.1),
        Direction::Right => (pos.0, pos.1+1),
    }
}

fn get_edge(pos: &(i32, i32), direction: &Direction) -> Edge {
    return match direction {
        Direction::Up => Edge{ a:(pos.0, pos.1+1), b:(pos.0, pos.1) },
        Direction::Left => Edge{ a:(pos.0, pos.1), b:(pos.0+1, pos.1) },
        Direction::Down => Edge{ a:(pos.0+1, pos.1), b:(pos.0+1, pos.1+1) },
        Direction::Right => Edge { a:(pos.0+1, pos.1+1), b:(pos.0, pos.1+1) },
    }
}

fn get_fencing(
    pos: (i32,i32), nrows: i32, ncols: i32, fields: &Vec<char>, visited: &mut Vec<bool>,
    area: &mut i32, perimeter: &mut i32, edges: &mut Vec<Edge>
) {
    let current = (pos.0*ncols+pos.1) as usize;
    visited[current] = true;
    *area += 1;
    let crop = fields[current];
    for d in Direction::VALUES.iter() {
        let (i,j) = get_neighbour(&pos, d);
        let edge = get_edge(&pos, d);
        if i < 0 || i >= nrows || j < 0 || j >= ncols {
            *perimeter += 1;
            edges.push(edge);
            continue
        }

        let index = (i*ncols+j) as usize;
        let other_crop = fields[index];
        if other_crop != crop {
            *perimeter += 1;
            edges.push(edge);
        }
        else if !visited[index] {
            get_fencing((i,j), nrows, ncols, fields, visited, area, perimeter, edges);
        }
    }
}

fn sort_edges(edges: &mut Vec<Edge>) -> Vec<Vec<Edge>>  {
    let mut contours = Vec::<Vec<Edge>>::new();
    let mut sorted = Vec::<Edge>::new();
    sorted.push(Edge{ a: edges[0].a, b: edges[0].b });
    edges.remove(0);
    let mut found: bool;
    while edges.len() > 0 {
        found = false;
        let tail = &sorted[sorted.len()-1];
        for (i,edge) in edges.iter().enumerate() {
            if edge.a.0 == tail.b.0 && edge.a.1 == tail.b.1 {
                let newedge = Edge{ a: edge.a, b: edge.b };
                sorted.push(newedge);
                edges.remove(i);
                found = true;
                break
            }
        }
        if !found {
           contours.push(sorted);
           sorted = Vec::<Edge>::new();
           sorted.push(Edge{ a: edges[0].a, b: edges[0].b });
           edges.remove(0);
        }
    }
    contours.push(sorted);
    contours
}

fn main() {
    let file = File::open("input").unwrap();
    let reader = BufReader::new(file);
    let mut nrows: i32 = 0;
    let mut ncols: i32 = 0;
    let mut fields = Vec::<char>::new();
    let mut visited = Vec::<bool>::new();
    for line in reader.lines() {
        let line = line.unwrap();
        ncols = line.len() as i32;
        nrows += 1;
        for v in line.chars() {
            fields.push(v);
            visited.push(false);
        }
    }
    let mut price1 = 0;
    let mut price2 = 0;
    for i in 0..nrows {
        for j in 0..ncols {
            let index = (i*ncols+j) as usize;
            if visited[index] {
                continue
            }
            let mut area = 0;
            let mut perimeter = 0;
            let mut edges = Vec::<Edge>::new();
            get_fencing((i,j), nrows, ncols, &fields, &mut visited, &mut area, &mut perimeter, &mut edges);
            let contours = sort_edges(&mut edges);
            let mut corners = 0;
            for sortededges in contours.iter() {
                for i in 0..sortededges.len() {
                    let e0 = sortededges.get(i).unwrap();
                    let e1 = sortededges.get((i+1)%sortededges.len()).unwrap();
                    let u0 = e0.b.0 - e0.a.0;
                    let v0 = e0.b.1 - e0.a.1;
                    let u1 = e1.b.0 -e1.a.0;
                    let v1 = e1.b.1 - e1.a.1;
                    if u0 != u1 || v0 != v1 {
                        corners += 1;
                    }
                }
            }
            price1 += area*perimeter;
            price2 += area*corners;
        }
    }
    println!("Total fencing part 1: {}", price1);
    println!("Total fencing part 2: {}", price2);
}
