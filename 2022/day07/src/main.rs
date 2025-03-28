use anyhow::Result;
use std::env;
use std::fs::File;
use std::io::{BufRead, BufReader};
use std::{cell::RefCell, rc::Rc};

struct Directory {
    name: String,
    fsize: usize,
    parent: Option<DirectoryRef>,
    first_child: Option<DirectoryRef>,
    next_sibling: Option<DirectoryRef>,
}

impl Directory {
    pub fn new(name: String) -> Self {
        Self {
            name,
            fsize: 0,
            parent: None,
            first_child: None,
            next_sibling: None,
        }
    }
}

fn add_child(parent: DirectoryRef, name: String) {
    let child = Rc::new(RefCell::new(Directory::new(name)));
    child.borrow_mut().parent = Some(Rc::clone(&parent));
    let mut parent_ = parent.borrow_mut();
    if parent_.first_child.is_none() {
        parent_.first_child = Some(child);
        return;
    }
    let mut sibling = parent_.first_child.clone();
    while let Some(dir) = sibling {
        let mut dir = dir.borrow_mut();
        match dir.next_sibling.clone() {
            None => {
                dir.next_sibling = Some(child);
                break;
            }
            Some(s) => {
                sibling = Some(s);
            }
        }
    }
}

fn get_directory(parent: DirectoryRef, name: String) -> Option<DirectoryRef> {
    if name == ".." {
        return parent.borrow_mut().parent.clone();
    }
    let mut sibling = parent.borrow_mut().first_child.clone();
    while let Some(s) = sibling {
        if s.borrow_mut().name == name {
            return Some(s);
        }
        sibling = s.borrow_mut().next_sibling.clone();
    }
    None
}

fn add_file(dir: DirectoryRef, size: usize) {
    dir.borrow_mut().fsize += size;
}

fn get_total_size(dir: DirectoryRef) -> usize {
    let mut total_size = dir.borrow().fsize;
    let mut sibling = dir.borrow_mut().first_child.clone();
    while let Some(s) = sibling {
        total_size += get_total_size(s.clone());
        sibling = s.borrow_mut().next_sibling.clone();
    }
    total_size
}

fn get_total_size_below(root: DirectoryRef, max_size: usize) -> usize {
    let mut total_size = 0;
    let size = get_total_size(root.clone());
    if size <= max_size {
        total_size += size;
    }
    let mut sibling = root.borrow_mut().first_child.clone();
    while let Some(s) = sibling {
        total_size += get_total_size_below(s.clone(), max_size);
        sibling = s.borrow_mut().next_sibling.clone();
    }
    total_size
}

fn get_minimum_free(root: DirectoryRef, target_size: usize, min_size: &mut usize) {
    let size = get_total_size(root.clone());
    if size < target_size {
        return;
    }
    if size < *min_size {
        *min_size = size;
    }
    let mut sibling = root.borrow_mut().first_child.clone();
    while let Some(s) = sibling {
        get_minimum_free(s.clone(), target_size, min_size);
        sibling = s.borrow_mut().next_sibling.clone();
    }
}

type DirectoryRef = Rc<RefCell<Directory>>;

fn main() -> Result<()> {
    let filename = env::args().nth(1).expect("Missing filename");
    let file = File::open(filename)?;
    let reader = BufReader::new(file);
    let root = Rc::new(RefCell::new(Directory::new("/".to_string())));
    let mut current = Rc::clone(&root);
    let mut skip = true;
    for line in reader.lines() {
        if skip {
            skip = false;
            continue;
        }
        let line = line?;
        if line.starts_with("$") {
            let command = line.split_whitespace().nth(1).expect("Missing command");
            if command == "ls" {
                continue;
            }
            let directory = line.split_whitespace().nth(2).expect("Missing directory");
            current =
                get_directory(current.clone(), directory.to_string()).expect("Directory not found");
        } else if line.starts_with("dir") {
            let dir_name = line
                .split_whitespace()
                .nth(1)
                .expect("Missing directory name");
            add_child(current.clone(), dir_name.to_string());
        } else {
            let file_size = line
                .split_whitespace()
                .nth(0)
                .expect("Missing file size")
                .parse::<usize>()?;
            add_file(current.clone(), file_size);
        }
    }

    println!("Part 1: {}", get_total_size_below(root.clone(), 100000));
    let total_disk_space = 70000000;
    let required_space = 30000000;
    let free_space = total_disk_space - get_total_size(root.clone());
    let target_size = required_space - free_space;
    let mut min_size = std::usize::MAX;
    get_minimum_free(root.clone(), target_size, &mut min_size);
    println!("Part 2: {}", min_size);

    Ok(())
}
