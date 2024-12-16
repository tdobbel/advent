use std::fs::File;
use std::io::{BufRead, BufReader};

fn check_vector(vec: &mut Vec<char>) -> i32 {
    if vec.iter().collect::<String>() == "XMAS" {
        return 1;
    }
    vec.reverse();
    if vec.iter().collect::<String>() == "XMAS" {
        return 1;
    }
    return 0;
}

fn count_xmas(vec: &Vec<char>, j: usize) -> i32 {
    let mut count = 0;
    let nx = vec.len() / 4;
    let mut vertical = Vec::<char>::new();
    for k in 0..4 {
        vertical.push(vec[k*nx+j]);
    }
    count += check_vector(&mut vertical);
    if j > 2 {
        let mut diagleft = Vec::<char>::new();
        for k in 0..4 {
            diagleft.push(vec[k*nx+j-k]);
        }
        count += check_vector(&mut diagleft);
    }
    if j < nx - 3 {
        let mut diagright = Vec::<char>::new();
        for k in 0..4 {
            diagright.push(vec[k*nx+j+k]);
        }
        count += check_vector(&mut diagright);
    }
    count
}

fn part1<R>(reader: BufReader<R>) -> i32 where R: std::io::Read{
    let mut subarray = Vec::<char>::new();
    let mut total = 0;
    for (index, line) in reader.lines().enumerate() {
        let line = line.unwrap();
        let n = line.len();
        for (j,c) in line.chars().enumerate() {
            subarray.push(c);
            if j > 2 {
                let end = subarray.len();
                let mut horizontal = subarray[end-4..end].to_vec().clone();
                total += check_vector(&mut horizontal);
            }
        }
        if index < 3 {
            continue;
        }
        for col in 0..n {
            total += count_xmas(&subarray, col);
        }
        subarray = subarray[n..].to_vec();
    }
    total
}

fn ismas(vec: &mut Vec<char>) -> bool {
    if vec.iter().collect::<String>() == "MAS" {
        return true;
    }
    vec.reverse();
    if vec.iter().collect::<String>() == "MAS" {
        return true;
    }
    return false;
}

fn part2<R>(reader: BufReader<R>) -> i32 where R: std::io::Read{
    let mut subarray = Vec::<char>::new();
    let mut total = 0;
    for (index, line) in reader.lines().enumerate() {
        let line = line.unwrap();
        let n = line.len();
        for c in line.chars() {
            subarray.push(c);
        }
        if index < 2 {
            continue;
        }
        for col in 0..n-2 {
            let mut diag1 = vec![subarray[col], subarray[n+col+1], subarray[2*n+col+2]];
            let mut diag2 = vec![subarray[2*n+col], subarray[n+col+1], subarray[col+2]];
            if ismas(&mut diag1) && ismas(&mut diag2) {
                total += 1;
            }
        }
        subarray = subarray[n..].to_vec();
    }
    total
}

fn main() {
    let file = File::open("input").unwrap();
    let reader = BufReader::new(file);
    //let total = part1(reader);
    let total = part2(reader);
    println!("Total: {}", total);
}
