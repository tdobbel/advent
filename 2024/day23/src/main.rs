use std::fs::File;
use std::io::{BufRead, BufReader};
use std::collections::{HashMap, HashSet};
use std::env;

fn find_triangles(
    node: usize, adj: &HashMap<usize, Vec<usize>>, start: usize, depth: i32,
    triangle: Vec<usize>, loops: &mut HashSet<(usize,usize,usize)>
) {
    let neighbors = adj.get(&node).unwrap();
    let mut new_tri = triangle.clone();
    new_tri.push(node);
    if depth == 1 {
        if neighbors.contains(&start) {
            new_tri.sort();
            let tri = (new_tri[0], new_tri[1], new_tri[2]);
            loops.insert(tri);
        }
        return
    }
    for v in neighbors.iter() {
        find_triangles(*v, adj, start, depth-1, new_tri.clone(), loops);
    }
}

fn find_max_clique(adj: &HashMap<usize, Vec<usize>>) -> Vec<usize> {
    let n_node = adj.len();
    let mut touched = vec![false; n_node];
    let mut best = Vec::<usize>::new();
    touched[0] = true;
    let mut clique = vec![0];
    let mut n = n_node-1;
    while n > 0 {
        let mut added = 0;
        for node in 0..n_node {
            let mut add = true;
            if touched[node] {
                continue
            }
            let neighbors = adj.get(&node).unwrap();
            for v in clique.iter() {
                if !neighbors.contains(v) {
                    add = false;
                    break
                }
            }
            if add {
                clique.push(node);
                touched[node] = true;
                added += 1;
                n -= 1;
            }
        }
        if added == 0 {
            if clique.len() > best.len() {
                best = clique.clone();
            }
            if n > 0 {
                let i = touched.iter().position(|&x| x == false).unwrap();
                clique = vec![i];
                touched[i] = true;
                n -= 1;
            }
        }

    }
    best
}

fn main() {
    let args: Vec<String> = env::args().collect();
    let file = File::open(&args[1]).unwrap();
    let reader = BufReader::new(file);
    let mut nodes = HashSet::<String>::new();
    let mut edges = Vec::<(String, String)>::new();
    for line in reader.lines() {
        let line = line.unwrap();
        let splitted = line.split("-").map(|x| x.to_string()).collect::<Vec<String>>();
        let v0 = splitted[0].clone();
        let v1 = splitted[1].clone();
        nodes.insert(v0.clone());
        nodes.insert(v1.clone());
        edges.push((v0, v1));
    }
    let mut node_map = HashMap::<&str, usize>::new();
    let mut node_tags = Vec::<&str>::new();
    for (i,tag) in nodes.iter().enumerate() {
        node_map.insert(tag.as_str(), i);
        node_tags.push(tag.as_str());
    }
    let mut adj = HashMap::<usize,Vec<usize>>::new();
    for (v0,v1) in edges.iter() {
        let i = node_map.get(v0.as_str()).unwrap();
        let j = node_map.get(v1.as_str()).unwrap();
        adj.entry(*i).or_insert(Vec::<usize>::new()).push(*j);
        adj.entry(*j).or_insert(Vec::<usize>::new()).push(*i);
    }
    let mut triangles = HashSet::<(usize,usize,usize)>::new();
    for (name,id) in node_map.iter() {
        if !name.starts_with('t') {
            continue
        }
        find_triangles(*id, &adj, *id, 3, Vec::<usize>::new(), &mut triangles);
    }
    println!("Number of triangles: {}", triangles.len());
    let max_clique = find_max_clique(&adj);
    let mut tags = Vec::<&str>::new();
    for i in max_clique.iter() {
        tags.push(node_tags[*i]);
    }
    tags.sort();
    println!("Maximum community: {}", tags.join(","));
}
