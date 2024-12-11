use std::fs::read_to_string;

fn one_less(v: &Vec<i32>, indx: usize) -> Vec<i32> {
    let mut res = Vec::new();
    for i in 0..v.len() {
        if i == indx {
            continue;
        }
        res.push(v[i]);
    }
    res
}

fn is_safe(v: &Vec<i32>) -> bool {
    let incr = v[1] > v[0];
    for i in 1..v.len() {
        let mut diff = v[i] - v[i-1];
        if !incr {
            diff *= -1;
        }
        if diff < 1 || diff > 3 {
            return false;

        }
    }
    true
}

fn is_safe_dampened(v: &Vec<i32>) -> bool {
    for i in 0..v.len() {
        let new = one_less(&v, i);
        if is_safe(&new) {
            return true;
        }
    }
    false
}

fn main() {
    let mut total = 0;
    for line in read_to_string("input").unwrap().lines() {
        let numbers: Vec<i32> = line.split_whitespace().map(|x| x.parse::<i32>().unwrap()).collect();
        if is_safe(&numbers) {
            total += 1;
            continue
        }
        if is_safe_dampened(&numbers){
            total += 1;
        }
    }
    println!("{}", total);
}
