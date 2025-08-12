use anyhow::Result;
use std::env::args;
use std::fs;

struct Block {
    capacity: usize,
    size: usize,
    values: Vec<usize>,
}

fn parse_blocks(disk_map: &[usize]) -> Vec<Block> {
    let mut blocks: Vec<Block> = Vec::new();
    for (i, &capacity) in disk_map.iter().enumerate() {
        let (values, size) = if i % 2 == 0 {
            (vec![i / 2; capacity], capacity)
        } else {
            (vec![Default::default(); capacity], 0)
        };
        blocks.push(Block {
            capacity,
            size,
            values,
        });
    }
    blocks
}

fn check_sum(disk: &[Block]) -> u64 {
    let mut result = 0;
    let mut i = 0;
    for block in disk.iter() {
        for (j, &value) in block.values.iter().enumerate() {
            if j < block.size {
                result += i * value as u64;
            }
            i += 1;
        }
    }
    result
}

fn compact_method1(disk: &mut [Block]) -> u64 {
    let mut left = 1;
    let mut right = disk.len() - 1;
    if right % 2 == 1 {
        right -= 1;
    }
    while left < right {
        while disk[left].size < disk[left].capacity && disk[right].size > 0 {
            disk[left].values[disk[left].size] = disk[right].values[disk[right].size - 1];
            disk[left].size += 1;
            disk[right].size -= 1;
        }
        if disk[left].size == disk[left].capacity {
            left += 2;
        }
        if disk[right].size == 0 {
            right -= 2;
        }
    }
    check_sum(disk)
}

fn compact_method2(disk: &mut [Block]) -> u64 {
    let mut right = disk.len() - 1;
    if right % 2 == 1 {
        right -= 1;
    }
    while right > 0 {
        let mut left = 1;
        while left < right {
            let free_space = disk[left].capacity - disk[left].size;
            if free_space >= disk[right].size {
                for i in 0..disk[right].size {
                    let il = disk[left].size + i;
                    disk[left].values[il] = disk[right].values[i];
                }
                disk[left].size += disk[right].size;
                disk[right].size = 0;
                break;
            }
            left += 2;
        }
        right -= 2;
    }
    check_sum(disk)
}

fn main() -> Result<()> {
    let filename = args().nth(1).expect("Missing input file");
    let disk_map: Vec<usize> = fs::read_to_string(filename)?
        .trim()
        .chars()
        .map(|c| c.to_digit(10).unwrap() as usize)
        .collect();
    let part1 = compact_method1(&mut parse_blocks(&disk_map));
    println!("Part 1: {}", part1);
    let part2 = compact_method2(&mut parse_blocks(&disk_map));
    println!("Part 2: {}", part2);

    Ok(())
}
