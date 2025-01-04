use std::env;
use std::cmp::max;
use std::collections::HashMap;
use std::fs::File;
use std::io::{BufRead, BufReader};

fn part1(start: &String, instructions: &Vec<char>, nodes: &HashMap<String,(String,String)>, indx0: usize) -> (String, usize) {
    let mut cntr = indx0;
    let mut node = start.to_string();
    while !node.ends_with("Z") || cntr == indx0 {
        let (left, right) = nodes.get(&node).unwrap();
        let instruction = instructions[cntr % instructions.len()];
        node = if instruction == 'R' { right.to_string() } else { left.to_string() };
        cntr += 1;
    }
    (node, cntr)
}

fn factorization(mut n: usize) -> Vec<usize> {
    let mut factors = Vec::new();
    let mut d = 2;
    while d * d <= n {
        if n % d == 0 {
            factors.push(d);
            n /= d;
        } else {
            d += 1;
        }
    }
    if n > 1 {
        factors.push(n);
    }
    factors
}

fn main() {
    let args: Vec<String> = env::args().collect();
    let file = File::open(&args[1]).unwrap();
    let reader = BufReader::new(file);
    let mut lines = reader.lines();
    let line = lines.next().unwrap().unwrap();
    let instructions: Vec<char> = line.chars().collect();
    let mut nodes = HashMap::<String,(String,String)>::new(); 
    let mut pos: Vec<String> = Vec::new();
    while let Some(line) = lines.next() {
        let line = line.unwrap();
        if line.len() == 0 {
            continue;
        }
        let mut splitted = line.split("=");
        let node = splitted.next().unwrap().trim();
        if node.ends_with("A") {
            pos.push(node.to_string());
        }
        let mut content = splitted.next().unwrap().trim();
        content = &content[1..content.len()-1];
        let lr = content.split(", ").collect::<Vec<&str>>();
        nodes.insert(node.to_string(), (lr[0].to_string(), lr[1].to_string()));
    }
    let (_, sol1) = part1(&"AAA".to_string(), &instructions, &nodes, 0);
    println!("Part1: {}", sol1);
    let mut factors = HashMap::<usize,usize>::new();
    for node in pos.iter() {
        let (_, steps) = part1(node, &instructions, &nodes, 0);
        let fs = factorization(steps);
        let mut cntr = HashMap::<usize,usize>::new();
        for f in fs.iter() {
            *cntr.entry(*f).or_insert(0) += 1;
        }
        for (k,v) in cntr.iter() {
            let value = factors.entry(*k).or_insert(0);
            *value = max(*value, *v);
        }
    }
    let sol2 = factors.iter().fold(1, |acc, (k,v)| acc * k.pow(*v as u32));
    println!("Part2: {}", sol2);
}
