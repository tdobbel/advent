use std::fs::read_to_string;

fn main() {
    let mut total = 0;
    for line in read_to_string("input").unwrap().lines() {
        let numbers: Vec<i32> = line.split_whitespace().map(|x| x.parse::<i32>().unwrap()).collect();
        let mut safe = true;
        let incr = numbers[1] > numbers[0];
        for i in 1..numbers.len() {
            let mut diff = numbers[i] - numbers[i-1];
            if !incr {
                diff *= -1;
            }
            if diff < 1 || diff > 3 {
                safe = false;
                break;
            }
        }
        if safe {
            total += 1;
        }
    }
    println!("{}", total);
}
