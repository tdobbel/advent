use std::fs::File;
use std::io::{BufRead, BufReader};
use std::collections::HashMap;
use std::cmp::max;
use std::env;

#[derive(PartialEq,Eq,Hash)]
enum Color {
    RED,
    BLUE,
    GREEN,
}

fn parse_color_info(info: &str) -> Vec<(Color,usize)> {
    let mut res = Vec::<(Color,usize)>::new();
    for sub in info.split(',') {
        let color_num = sub.split_whitespace().collect::<Vec<&str>>();
        assert_eq!(color_num.len(), 2);
        let color = match color_num[1] {
            "red" => Color::RED,
            "blue" => Color::BLUE,
            "green" => Color::GREEN,
            _ => panic!("Invalid color"),
        };
        res.push((color, color_num[0].parse::<usize>().unwrap()));
    }
    res
}

fn main() {
    let args: Vec<String> = env::args().collect();
    let file = File::open(&args[1]).unwrap();
    let reader = BufReader::new(file);
    let mut capacity = HashMap::<Color,usize>::new();
    capacity.insert(Color::RED, 12);
    capacity.insert(Color::GREEN, 13);
    capacity.insert(Color::BLUE, 14);
    let mut total = 0;
    let mut power_sum = 0;
    for (i,line) in reader.lines().enumerate() {
        let line = line.unwrap();
        let indx = line.chars().position(|x| x == ':').unwrap();
        let line = &line[indx+1..];
        let mut ok = true;
        let mut req = HashMap::<Color,usize>::new();
        req.insert(Color::RED, 0);
        req.insert(Color::GREEN, 0);
        req.insert(Color::BLUE, 0);
        for sub in line.split(';') {
            let bags = parse_color_info(sub.trim());
            for (color, num) in bags {
                if num > capacity[&color] {
                    ok = false;
                }
                let r = req.get_mut(&color).unwrap();
                *r = max(*r, num);
            }
        }
        let power = req.values().product::<usize>();
        power_sum += power;
        if ok {
            total += i+1;
        }
    }
    println!("Sum of game IDs: {}", total);
    println!("Sum of power: {}", power_sum);
}
