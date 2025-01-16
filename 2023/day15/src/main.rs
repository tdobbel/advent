use std::env;
use std::fs;

fn hash(input: &str) -> u32 {
    let mut hash = 0;
    for c in input.chars() {
        hash += c as u32;
        hash *= 17;
        hash %= 256;
    }
    hash
}

fn part2(operations: &[&str], boxes: &mut [Vec<(String, u32)>]) -> u32 {
    for op in operations {
        if op.contains('=') {
            let label = op.split('=').next().unwrap();
            let focal_length: u32 = op.split('=').last().unwrap().parse().unwrap();
            let i = hash(label) as usize;
            let mut found = false;
            for (l, f) in boxes[i].iter_mut() {
                if l == label {
                    found = true;
                    *f = focal_length;
                    break;
                }
            }
            if !found {
                boxes[i].push((label.to_string(), focal_length));
            }
        } else {
            let label = &op[..op.len() - 1];
            let i = hash(label) as usize;
            boxes[i] = boxes[i]
                .iter()
                .map(|(l, f)| (l.clone(), *f))
                .filter(|(l, _)| l != label)
                .collect();
        }
    }
    let mut total = 0;
    for (i, box_) in boxes.iter().enumerate() {
        for (j, (_, f)) in box_.iter().enumerate() {
            total += (i as u32 + 1) * (j as u32 + 1) * f;
        }
    }
    total
}

fn main() {
    let args = env::args().nth(1).expect("Please provide an input file");
    let file = fs::read_to_string(args).expect("Failed to read the file");
    let line = file.lines().next().unwrap();
    let init_seq: Vec<&str> = line.split(',').collect();
    let total1: u32 = init_seq.iter().map(|x| hash(x)).sum();
    println!("Part 1: {}", total1);
    let mut boxes: Vec<Vec<(String, u32)>> = vec![vec![]; 256];
    println!("Part 2: {}", part2(&init_seq, &mut boxes));
}
