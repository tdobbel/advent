use anyhow::Result;
use std::cell::RefCell;
use std::collections::HashMap;
use std::env;
use std::fs::File;
use std::io::{BufRead, BufReader};

pub struct Splitter {
    counter: u64,
    n_children: usize,
    children: [(usize, usize); 2],
}

impl Splitter {
    pub fn new() -> RefCell<Self> {
        RefCell::new(Self {
            counter: 0,
            n_children: 0,
            children: Default::default(),
        })
    }

    pub fn add_child(&mut self, pos: (usize, usize)) {
        self.children[self.n_children] = pos;
        self.n_children += 1;
    }
}

pub fn find_splits(
    source: (usize, usize),
    manifold: &[Vec<char>],
    splitters: &mut HashMap<(usize, usize), RefCell<Splitter>>,
    left: bool,
) {
    let (mut x, mut y) = source;
    if left {
        if x == 0 {
            return;
        }
        x -= 1;
    } else if x >= manifold[0].len() - 1 {
        return;
    } else {
        x += 1;
    }
    while y < manifold.len() && manifold[y][x] != '^' {
        y += 1;
    }
    if y == manifold.len() {
        return;
    }
    let pos = (x, y);
    let parent = splitters.get_mut(&source).unwrap();
    parent.borrow_mut().add_child(pos);
    if splitters.contains_key(&pos) {
        return;
    }
    splitters.insert(pos, Splitter::new());
    find_splits(pos, manifold, splitters, true);
    find_splits(pos, manifold, splitters, false);
}

fn main() -> Result<()> {
    let args = env::args().nth(1).expect("Please provide an input file");
    let file = File::open(args)?;
    let reader = BufReader::new(file);
    let mut manifold: Vec<Vec<char>> = Vec::new();

    let mut ny = 0;
    let mut src_x = 0;
    let mut src_y = 0;
    for line in reader.lines() {
        let line = line?;
        let mut row = Vec::new();
        for (x, c) in line.chars().enumerate() {
            if c == 'S' {
                src_x = x;
                src_y = ny;
            }
            row.push(c);
        }
        ny += 1;
        manifold.push(row);
    }
    let mut splitters = HashMap::new();
    while manifold[src_y][src_x] != '^' {
        src_y += 1;
    }
    splitters.insert((src_x, src_y), Splitter::new());
    find_splits((src_x, src_y), &mut manifold, &mut splitters, true);
    find_splits((src_x, src_y), &mut manifold, &mut splitters, false);
    println!("Part 1: {}", splitters.len());

    for (y, line) in manifold.iter().enumerate().rev() {
        for (x, c) in line.iter().enumerate() {
            if *c != '^' {
                continue;
            }
            let mut splitter = match splitters.get(&(x, y)) {
                None => continue,
                Some(split) => split.borrow_mut(),
            };
            let children = splitter.children;
            for (i, pos) in children.iter().enumerate() {
                if i < splitter.n_children {
                    splitter.counter += splitters[pos].borrow().counter;
                } else {
                    splitter.counter += 1;
                }
            }
        }
    }
    let part2 = splitters[&(src_x, src_y)].borrow().counter;
    println!("Part 2: {}", part2);

    Ok(())
}
