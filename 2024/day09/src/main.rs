use anyhow::Result;
use std::env::args;
use std::fs;

struct Block {
    size: usize,
    values: Vec<Option<usize>>,
    index: usize,
    n_values: usize,
}

fn parse_blocks(disk_map: &[u32]) -> Vec<Block> {
    let mut blocks = Vec::<Block>::new();
    for (i, &size) in disk_map.iter().enumerate() {
        let size = size as usize;
        let (n_values, value) = if i % 2 == 0 {
            (size, Some(i / 2))
        } else {
            (0, None)
        };
        let block = Block {
            size,
            values: vec![value; size],
            index: 0,
            n_values,
        };
        blocks.push(block);
    }
    blocks
}

fn next_left(disk: &[Block], ileft: usize) -> Option<usize> {
    let mut index = ileft;
    while index < disk.len() {
        let block = &disk[index];
        if block.index < block.size {
            return Some(index);
        }
        index += 2;
    }
    None
}

fn next_right(disk: &[Block], iright: usize) -> Option<usize> {
    let mut index = iright;
    while disk[index].n_values == 0 {
        index = index.checked_sub(2)?;
    }
    Some(index)
}

fn check_sum(disk: &[Block]) -> u64 {
    let mut result = 0;
    let mut i = 0;
    for block in disk.iter() {
        for value in block.values.iter() {
            if let Some(val) = value {
                let increment = val * i;
                result += increment as u64;
            }
            i += 1;
        }
    }
    result
}

fn part1(disk: &mut [Block]) -> u64 {
    let mut ileft = next_left(&disk, 1).unwrap();
    let mut iright = next_right(
        &disk,
        if disk.len() % 2 == 0 {
            disk.len() - 2
        } else {
            disk.len() - 1
        },
    )
    .unwrap();
    while ileft < iright {
        while disk[ileft].index < disk[ileft].size && disk[iright].n_values > 0 {
            let indx_left = disk[ileft].index;
            let indx_right = disk[iright].n_values - 1;
            disk[ileft].values[indx_left] = disk[iright].values[indx_right];
            disk[iright].values[indx_right] = None;
            disk[ileft].index += 1;
            disk[iright].n_values -= 1;
        }
        ileft = next_left(&disk, ileft).unwrap();
        iright = next_right(&disk, iright).unwrap();
    }
    check_sum(&disk)
}

fn part2(disk: &mut [Block]) -> u64 {
    let mut iright = if disk.len() % 2 == 0 {
        disk.len() - 2
    } else {
        disk.len() - 1
    };
    loop {
        let mut ileft = 1;
        while ileft < iright {
            if disk[ileft].size >= disk[iright].size {
                for i in 0..disk[iright].size {
                    disk[ileft].values[disk[ileft].index] = disk[iright].values[i];
                    disk[iright].values[i] = None;
                    disk[ileft].index += 1;
                    disk[ileft].size -= 1;
                }
                break;
            }
            ileft += 2;
        }
        if iright == 0 {
            break;
        }
        iright -= 2;
    }
    check_sum(disk)
}

fn main() -> Result<()> {
    let filename = args().nth(1).expect("Missing input file");
    let disk_map: Vec<u32> = fs::read_to_string(filename)?
        .trim()
        .chars()
        .map(|c| c.to_digit(10).unwrap())
        .collect();
    let sol1 = part1(&mut parse_blocks(&disk_map));
    println!("Part 1: {}", sol1);
    let sol2 = part2(&mut parse_blocks(&disk_map));
    println!("Part 2: {}", sol2);

    Ok(())
}
