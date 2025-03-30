use anyhow::Result;
use std::collections::HashMap;
use std::env;
use std::fs::File;
use std::io::{BufRead, BufReader};
use std::{
    cell::RefCell,
    rc::{Rc, Weak},
};

struct Directory {
    fsize: usize,
    parent: Option<Weak<RefCell<Directory>>>,
    children: HashMap<String, DirectoryRef>,
}

type DirectoryRef = Rc<RefCell<Directory>>;

impl Directory {
    pub fn new() -> Self {
        Self {
            fsize: 0,
            parent: None,
            children: HashMap::new(),
        }
    }
}

fn add_child(parent: &DirectoryRef, name: String) {
    let child = Rc::new(RefCell::new(Directory::new()));
    child.borrow_mut().parent = Some(Rc::downgrade(parent));
    let mut parent_ = parent.borrow_mut();
    parent_.children.insert(name, child);
}

fn get_directory(current_dir: &DirectoryRef, name: String) -> Option<DirectoryRef> {
    if name == ".." {
        match &current_dir.borrow().parent {
            Some(parent) => return Weak::upgrade(parent),
            None => return None,
        }
    }
    current_dir.borrow().children.get(&name).cloned()
}

fn add_file(dir: &DirectoryRef, size: usize) {
    dir.borrow_mut().fsize += size;
}

fn get_total_size(dir: &DirectoryRef) -> usize {
    let d = dir.borrow();
    let mut total_size = dir.borrow().fsize;
    for child in d.children.values() {
        total_size += get_total_size(child);
    }
    total_size
}

fn get_total_size_below(root: &DirectoryRef, max_size: usize) -> usize {
    let mut total_size = 0;
    let size = get_total_size(root);
    if size <= max_size {
        total_size += size;
    }
    for child in root.borrow().children.values() {
        total_size += get_total_size_below(child, max_size);
    }
    total_size
}

fn get_minimum_free(root: &DirectoryRef, target_size: usize, min_size: &mut usize) {
    let size = get_total_size(root);
    if size < target_size {
        return;
    }
    if size < *min_size {
        *min_size = size;
    }
    for child in root.borrow().children.values() {
        get_minimum_free(child, target_size, min_size);
    }
}

fn main() -> Result<()> {
    let filename = env::args().nth(1).expect("Missing filename");
    let file = File::open(filename)?;
    let reader = BufReader::new(file);
    let root = Rc::new(RefCell::new(Directory::new()));
    let mut current = Rc::clone(&root);
    let mut skip = true;
    for line in reader.lines() {
        if skip {
            skip = false;
            continue;
        }
        let line = line?;
        if line.starts_with('$') {
            let command = line.split_whitespace().nth(1).expect("Missing command");
            if command == "ls" {
                continue;
            }
            let directory = line.split_whitespace().nth(2).expect("Missing directory");
            current = get_directory(&current, directory.to_string()).expect("Directory not found");
        } else if line.starts_with("dir") {
            let dir_name = line
                .split_whitespace()
                .nth(1)
                .expect("Missing directory name");
            add_child(&current, dir_name.to_string());
        } else {
            let file_size = line
                .split_whitespace()
                .next()
                .ok_or_else(|| anyhow::anyhow!("Missing file size in line {}", line))?
                .parse::<usize>()
                .map_err(|e| anyhow::anyhow!("Failed to parse file size: {}", e))?;
            add_file(&current, file_size);
        }
    }

    println!("Part 1: {}", get_total_size_below(&root, 100000));
    let total_disk_space = 70000000;
    let required_space = 30000000;
    let free_space = total_disk_space - get_total_size(&root);
    let target_size = required_space - free_space;
    let mut min_size = usize::MAX;
    get_minimum_free(&root, target_size, &mut min_size);
    println!("Part 2: {}", min_size);

    Ok(())
}
