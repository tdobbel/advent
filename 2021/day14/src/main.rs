use anyhow::Result;
use std::collections::HashMap;
use std::env;
use std::fs;

pub fn iterate(rules: &HashMap<&str, char>, template: String) -> String {
    let mut letters: Vec<char> = Vec::new();
    for (i, letter) in template[0..template.len() - 1].chars().enumerate() {
        let pair = &template[i..i + 2];
        letters.push(letter);
        if let Some(c) = rules.get(pair) {
            letters.push(*c);
        }
    }
    letters.push(template.chars().last().unwrap());
    String::from_iter(letters)
}

pub fn get_polymer_score(poly: &str) -> usize {
    let mut counter: HashMap<char, usize> = HashMap::new();
    for c in poly.chars() {
        let n = counter.entry(c).or_insert(0);
        *n += 1;
    }
    let mut counts: Vec<usize> = counter.values().copied().collect();
    counts.sort();
    counts.last().unwrap() - counts[0]
}

fn main() -> Result<()> {
    let filename = env::args().nth(1).expect("Missing input file");
    let file = fs::read_to_string(filename)?;
    let lines: Vec<&str> = file.split("\n").collect();

    let mut rules: HashMap<&str, char> = HashMap::new();
    for line in lines.iter().skip(2) {
        if line.is_empty() {
            continue;
        }
        let (pair, c) = line.split_once(" -> ").unwrap();
        rules.insert(pair, c.chars().next().unwrap());
    }

    let mut template = lines[0].to_string();
    for _ in 0..10 {
        template = iterate(&rules, template);
    }
    let part1 = get_polymer_score(&template);

    println!("Part 1: {}", part1);
    Ok(())
}
