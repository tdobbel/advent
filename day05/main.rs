use std::fs::File;
use std::io::{BufRead, BufReader};
use std::collections::HashMap;
use std::cmp::Ordering;

fn main() {
    let file = File::open("input").unwrap();
    let reader = BufReader::new(file);
    let mut rules = HashMap::<i32, (Vec<i32>, Vec<i32>)>::new();
    let mut total = 0;
    for line in reader.lines() {
        let line = line.unwrap();
        if line.len() == 0 {
            continue;
        }
        if line.contains("|"){
            let pair = line.split("|").map(|x| x.parse::<i32>().unwrap()).collect::<Vec<i32>>();
            let lhs = pair[0];
            let rhs = pair[1];
            rules.entry(lhs)
                .or_insert((Vec::<i32>::new(), Vec::<i32>::new()))
                .1.push(rhs);
            rules.entry(rhs)
                .or_insert((Vec::<i32>::new(), Vec::<i32>::new()))
                .0.push(lhs);
        }
        else {
            let mut correct = true;
            let mut pages = line.split(",").map(|x| x.parse::<i32>().unwrap()).collect::<Vec<i32>>();
            for i in 0..pages.len()-1 {
                let lhs = pages[i];
                for j in i+1..pages.len() {
                    let rhs = pages[j];
                    if !rules.get(&lhs).unwrap().1.contains(&rhs) {
                        correct = false;
                        break;
                    }
                }
                if !correct {
                    break;
                }
            }
            if correct {
                continue
            }
            pages.sort_by(|a, b| {
                let lg = rules.get(a).unwrap();
                if lg.0.contains(b) {
                    return Ordering::Less;
                }
                return Ordering::Greater;
            });
            total += pages[pages.len()/2];
        }
    }
    println!("{}", total);
}
