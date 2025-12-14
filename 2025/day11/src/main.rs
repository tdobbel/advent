use anyhow::Result;
use std::collections::HashMap;
use std::env;
use std::fs::File;
use std::io::{BufRead, BufReader};

pub fn count_path(
    adj: &HashMap<u32, Vec<u32>>,
    cache: &mut HashMap<u32, u64>,
    src: u32,
    dst: u32,
) -> u64 {
    if let Some(p) = cache.get(&src) {
        return *p;
    }
    if src == dst {
        return 1;
    }
    let count = match adj.get(&src) {
        Some(nodes) => nodes
            .iter()
            .map(|&cur| count_path(adj, cache, cur, dst))
            .sum(),
        None => 0,
    };
    cache.insert(src, count);

    count
}

pub fn get_path(adj: &HashMap<u32, Vec<u32>>, nodes: &[u32]) -> u64 {
    let mut res = 1;
    for i in 0..nodes.len() - 1 {
        res *= count_path(adj, &mut HashMap::new(), nodes[i], nodes[i + 1]);
        if res == 0 {
            return 0;
        }
    }
    res
}

fn main() -> Result<()> {
    let args = env::args().nth(1).expect("Please provide an input file");
    let file = File::open(args)?;
    let reader = BufReader::new(file);

    let mut n_node: u32 = 0;
    let mut nmap: HashMap<String, u32> = HashMap::new();
    let mut adj: HashMap<u32, Vec<u32>> = HashMap::new();

    for line in reader.lines() {
        let line = line?;
        let (name, rhs) = line.split_once(": ").unwrap();
        let key = name.to_string();
        if !nmap.contains_key(&key) {
            nmap.insert(key, n_node);
            n_node += 1;
        }
        let i = nmap[&name.to_string()];
        for node in rhs.split_whitespace() {
            let key = node.to_string();
            if !nmap.contains_key(&key) {
                nmap.insert(key, n_node);
                n_node += 1;
            }
            let j = nmap.get(&node.to_string()).unwrap();
            let list = adj.entry(i).or_default();
            list.push(*j);
        }
    }

    let you = nmap[&"you".to_string()];
    let out = nmap[&"out".to_string()];
    let svr = nmap[&"svr".to_string()];
    let dac = nmap[&"dac".to_string()];
    let fft = nmap[&"fft".to_string()];

    let part1 = count_path(&adj, &mut HashMap::new(), you, out);
    println!("Part 1: {}", part1);

    let path1 = get_path(&adj, &[svr, dac, fft, out]);
    let path2 = get_path(&adj, &[svr, fft, dac, out]);
    println!("Part 2: {}", path1 + path2);

    Ok(())
}
