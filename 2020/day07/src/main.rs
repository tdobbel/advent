use anyhow::Result;
use std::collections::{HashMap, HashSet};
use std::env;
use std::fs::File;
use std::io::{BufRead, BufReader};

#[derive(Default)]
struct Bag {
    parents: Vec<(usize, usize)>,
    children: Vec<(usize, usize)>,
}

#[derive(Default)]
pub struct BagRegister {
    name_map: HashMap<String, usize>,
    bags: Vec<Bag>,
}

impl BagRegister {
    pub fn get_bag_index(&mut self, name: &str) -> usize {
        if let Some(&index) = self.name_map.get(name) {
            return index;
        }
        let new_index = self.name_map.len();
        self.name_map.insert(name.to_string(), new_index);
        self.bags.push(Bag::default());
        new_index
    }

    pub fn add_bags(&mut self, parent: &str, qty: usize, child: &str) {
        let ip = self.get_bag_index(parent);
        let ic = self.get_bag_index(child);
        self.bags[ic].parents.push((ip, qty));
        self.bags[ip].children.push((ic, qty));
    }

    pub fn get_children(&self, index: usize) -> &[(usize, usize)] {
        &self.bags[index].children
    }
}

pub fn solve_part1(reg: &BagRegister, index: usize, found: &mut HashSet<usize>) {
    for (parent_index, _qty) in reg.bags[index].parents.iter() {
        found.insert(*parent_index);
        solve_part1(reg, *parent_index, found);
    }
}

pub fn solve_part2(reg: &BagRegister, index: usize, cache: &mut HashMap<usize, usize>) -> usize {
    if let Some(&n_bags) = cache.get(&index) {
        return n_bags;
    }
    let children = reg.get_children(index);
    let mut n_bags = 0;
    for (child_index, qty) in children.iter() {
        n_bags += (*qty) * (1 + solve_part2(reg, *child_index, cache));
    }
    cache.insert(index, n_bags);
    n_bags
}

fn main() -> Result<()> {
    let args = env::args().nth(1).expect("Please provide an input file");
    let file = File::open(args)?;
    let reader = BufReader::new(file);
    let mut bags = BagRegister::default();
    for line in reader.lines() {
        let line = line?;
        let (parent, children) = &line[..line.len() - 1].split_once(" contain ").unwrap();
        let parent = &parent[..parent.len() - 1]; // remove final s
        for entry in children.split(", ") {
            if entry == "no other bags" {
                break;
            }
            let (qty_str, child) = entry.split_once(" ").unwrap();
            let qty: usize = qty_str.parse()?;
            if qty == 1 {
                bags.add_bags(parent, qty, child);
            } else {
                bags.add_bags(parent, qty, &child[..child.len() - 1]); // remove final s
            }
        }
    }
    let mut found = HashSet::new();
    let start_index = bags.get_bag_index("shiny gold bag");
    solve_part1(&bags, start_index, &mut found);

    let mut cache = HashMap::new();
    let part2 = solve_part2(&bags, start_index, &mut cache);

    println!("Part 1:  {}", found.len());
    println!("Part 2:  {}", part2);
    Ok(())
}
