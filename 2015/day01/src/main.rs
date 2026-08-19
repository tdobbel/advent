use std::env;
use std::fs;

fn main() {
    let filename = env::args().nth(1).expect("Missing input file");
    let file = fs::read_to_string(filename).unwrap();
    let mut floor: i32 = 0;
    let mut part2 = None;
    for (i, c) in file.chars().enumerate() {
        match c {
            '(' => floor += 1,
            ')' => {
                if floor == 0 && part2.is_none() {
                    part2 = Some(i + 1);
                }
                floor -= 1;
            }
            _ => break,
        }
    }
    println!("Part 1: {}", floor);
    println!("Part 2: {}", part2.unwrap());
}
