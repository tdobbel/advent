use std::fs::File;
use std::io::{BufRead, BufReader};
use std::env;

#[derive(Debug)]
struct Number {
    value: i32,
    row: i32,
    col_start: i32,
    col_end: i32,
}


fn adjacent(num: &Number, i: i32, j: i32) -> bool {
    return num.row >= i-1 && num.row <= i+1 && j >= num.col_start-1 && j <= num.col_end+1
}

fn main() {
    let args: Vec<String> = env::args().collect();
    let file = File::open(&args[1]).unwrap();
    let reader = BufReader::new(file);
    let mut symbols = Vec::<(i32,i32)>::new();
    let mut numbers = Vec::<Number>::new();
    let mut stars = Vec::<(i32,i32)>::new();
    for (i,line) in reader.lines().enumerate() {
        let line = line.unwrap();
        let mut number = false;
        let mut value = 0;
        let mut start = 0;
        let mut end = 0;
        for (j,c) in line.chars().enumerate() {
            if c.is_numeric() {
                let v = c.to_digit(10).unwrap() as i32;
                if !number {
                    start = j;
                    end = j;
                    number = true;
                    value = v;
                }
                else {
                    end = j;
                   value = value*10 + v;
                }
                continue;
            }
            if number {
                number = false;
                numbers.push(Number { value, row:i as i32, col_start:start as i32, col_end:end as i32 });
            }
            if c == '.' {
                continue
            }
            symbols.push((i as i32,j as i32));
            if c == '*' {
                stars.push((i as i32,j as i32));
            }
        }
        if number {
            numbers.push(Number { value, row:i as i32, col_start:start as i32, col_end:end as i32 });
        }
    }
    let mut total = 0;
    let mut indx_part = Vec::<usize>::new();
    for (k,number) in numbers.iter().enumerate() {
        for (i,j) in symbols.iter() {
            if adjacent(number,*i,*j) {
                total += number.value;
                indx_part.push(k);
                break;
            }
        }
    }
    println!("Total: {}", total);
    let mut gear_sum = 0;
    for (i,j) in stars {
        let mut count = 0;
        let mut ratio = 1;
        for k in indx_part.iter() {
            let num = &numbers[*k];
            if adjacent(num, i, j) {
                ratio *= num.value;
                count += 1;
            }
        }
        if count == 2 {
            gear_sum += ratio;
        }
    }
    println!("Sum of gear ratios: {}", gear_sum);
}
