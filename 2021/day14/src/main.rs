use anyhow::Result;
use std::collections::HashMap;
use std::convert::TryFrom;
use std::env;
use std::error::Error;
use std::fmt;
use std::fs;

#[derive(Debug)]
struct BadSize;

impl fmt::Display for BadSize {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        write!(f, "Bad size")
    }
}

impl Error for BadSize {}

#[derive(PartialEq, Eq, Hash, Copy, Clone, Debug)]
struct Pair([char; 2]);

impl TryFrom<&str> for Pair {
    type Error = BadSize;

    fn try_from(value: &str) -> Result<Self, Self::Error> {
        let mut char_iter = value.chars();
        match (char_iter.next(), char_iter.next()) {
            (Some(a), Some(b)) => Ok(Pair([a, b])),
            _ => Err(BadSize),
        }
    }
}

type NextGeneration = HashMap<Pair, usize>;

fn generated_pairs(
    pair: &Pair,
    n: usize,
    rules: &HashMap<Pair, char>,
    cache: &mut HashMap<(Pair, usize), NextGeneration>,
) -> NextGeneration {
    let mut hm: HashMap<Pair, usize> = HashMap::new();
    if n == 0 {
        hm.insert(*pair, 1);
        return hm;
    }
    let key = (*pair, n);
    if cache.contains_key(&key) {
        return cache.get(&key).unwrap().clone();
    }
    let p: [char; 2] = pair.0;
    let mut children: Vec<Pair> = Vec::new();
    if let Some(&c) = rules.get(pair) {
        let pair1 = Pair([p[0], c]);
        children.push(pair1);
        let pair2 = Pair([c, p[1]]);
        children.push(pair2);
    } else {
        children.push(*pair);
    }
    for child in children.iter() {
        let next_gen = generated_pairs(child, n - 1, rules, cache);
        for (k, v) in next_gen.iter() {
            let cnt = hm.entry(*k).or_insert(0);
            *cnt += v;
        }
    }
    cache.insert(key, hm.clone());
    hm
}

fn main() -> Result<()> {
    let filename = env::args().nth(1).expect("Missing input file");
    let n_gen: usize = env::args()
        .nth(2)
        .expect("Missing number of generations")
        .parse()
        .unwrap();
    let file = fs::read_to_string(filename)?;
    let lines: Vec<&str> = file.split("\n").collect();

    let mut rules: HashMap<Pair, char> = HashMap::new();
    for line in lines.iter().skip(2) {
        if line.is_empty() {
            continue;
        }
        let (pair_, c) = line.split_once(" -> ").unwrap();
        let pair: Pair = pair_.try_into()?;
        rules.insert(pair, c.chars().next().unwrap());
    }

    let template = lines[0].to_string();
    let first_char = template.chars().next().unwrap();
    let last_char = template.chars().last().unwrap();
    let mut cache = HashMap::new();
    let mut total_pairs: NextGeneration = HashMap::new();
    for i in 0..template.len() - 1 {
        let temp = &template[i..i + 2];
        let p: Pair = temp.try_into()?;
        let hm = generated_pairs(&p, n_gen, &rules, &mut cache);
        for (&k, &v) in hm.iter() {
            let cnt = total_pairs.entry(k).or_insert(0);
            *cnt += v;
        }
    }
    // Count chars
    let mut char_cntr: HashMap<char, usize> = HashMap::new();
    for (&pair, &n) in total_pairs.iter() {
        for &c in pair.0.iter() {
            let cntr = char_cntr.entry(c).or_insert(0);
            *cntr += n;
        }
    }
    for (&c, n) in char_cntr.iter_mut() {
        *n >>= 1;
        if c == last_char || c == first_char {
            *n += 1;
        }
        println!("character {} appeared {} times", c, *n);
    }
    let mut counts: Vec<usize> = char_cntr.values().copied().collect();
    counts.sort();

    let score = counts.last().unwrap() - counts[0];
    println!("Puzzle result is: {}", score);

    Ok(())
}
