use std::fs::File;
use std::io::{BufRead, BufReader};

struct Block {
    free_space: usize,
    values: Vec<i32>,
    sizes: Vec<usize>,
}

fn part1(values: &Vec<i32>) -> u64 {
    let mut ileft = 0;
    let mut result = Vec::<i32>::new();
    let mut fidleft = 0;
    let mut iright = values.len() - 1;
    if values.len() % 2 == 0 {
        iright -= 1;
    }
    let mut n_right = values[iright];
    let mut fidright = iright as i32 / 2;
    while iright > ileft {
        for _ in 0..values[ileft] {
            result.push(fidleft);
        }
        ileft += 1;
        for _ in 0..values[ileft] {
            result.push(fidright);
            n_right -= 1;
            if n_right == 0 {
                iright -= 2;
                fidright -= 1;
                n_right = values[iright];
            }
            if iright < ileft {
                break;
            }
        }
        ileft += 1;
        fidleft += 1;
    }
    if n_right > 0 {
        for _ in 0..n_right {
            result.push(fidright);
        }
    }
    let mut checksum: u64 = 0;
    for (i, v) in result.iter().enumerate() {
        checksum += (*v as u64) * (i as u64);
    }
    checksum
}

fn part2(values: &Vec<i32>) -> u64 {
    let mut blocks = Vec::<Block>::new();
    let mut free = false;
    for (i, v) in values.iter().enumerate() {
        if free {
            blocks.push(Block {
                free_space: *v as usize,
                values: Vec::<i32>::new(),
                sizes: Vec::<usize>::new(),
            });
        } else {
            blocks.push(Block {
                free_space: 0,
                values: vec![(i as i32) / 2],
                sizes: vec![*v as usize],
            });
        }
        free = !free;
    }
    let mut iright = blocks.len() - 1;
    while iright > 0 {
        let value = blocks[iright].values[0];
        let size = blocks[iright].sizes[0];
        let mut moved = false;
        for i in 0..iright {
            let block = &mut blocks[i];
            if block.free_space >= size {
                block.values.push(value);
                block.sizes.push(size);
                block.free_space -= size;
                moved = true;
                break;
            }
        }
        if moved {
            blocks[iright].values.pop();
            blocks[iright].sizes.pop();
            blocks[iright].free_space = size;
        }
        iright -= 2;
    }
    let mut i: u64 = 0;
    let mut checksum: u64 = 0;
    for block in blocks.iter() {
        for j in 0..block.values.len() {
            for _ in 0..block.sizes[j] {
                checksum += (block.values[j] as u64) * i;
                //print!("{}", block.values[j]);
                i += 1;
            }
        }
        for _ in 0..block.free_space {
            //print!(".");
            i += 1;
        }
    }
    //print!("\n");
    checksum
}

fn main() {
    let file = File::open("input").unwrap();
    let reader = BufReader::new(file);
    let line = reader.lines().next().unwrap().unwrap();
    let values = line
        .chars()
        .map(|c| c.to_string().parse::<i32>().unwrap())
        .collect::<Vec<i32>>();
    println!("checksum {}", part1(&values));
    println!("checksum {}", part2(&values));
}
