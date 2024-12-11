use std::fs::read_to_string;

fn exec_line(line: &str) -> i32 {
    if line.len() < 4 {
        return 0;
    }
    let mut total = 0;
    let mut mul_indx = Vec::<usize>::new();
    for i in 0..line.len()-3 {
        if &line[i..i+4] == "mul(" {
            mul_indx.push(i);
        }
    }
    if mul_indx.len() == 0 {
        return 0;
    }
    mul_indx.push(line.len());
    for islice in 0..mul_indx.len()-1 {
        let start = mul_indx[islice]+4;
        let stop = mul_indx[islice+1];
        let mut substring = &line[start..stop];
        let k = substring.chars().position(|x| x == ')');
        if k.is_none() {
            continue
        }
        substring = &substring[..k.unwrap()];
        let nums = substring.split(",").collect::<Vec<&str>>();
        if nums.len() != 2 {
            continue
        }
        let rhs = nums[1].parse::<i32>();
        let lhs = nums[0].parse::<i32>();
        if rhs.is_err() || lhs.is_err() {
            continue
        }
        total += rhs.unwrap() * lhs.unwrap();
    }
    total
}

fn main() {
    let mut total = 0;
    let mut enabled = true;
    for line in read_to_string("input").unwrap().lines() {
        let mut start = 0;
        for i in 7..line.len() {
            if enabled && &line[i-7..i] == "don't()" {
                total += exec_line(&line[start..i-7]);
                enabled = false;
            }
            if !enabled && &line[i-4..i] == "do()" {
                enabled = true;
                start = i;
            }
        }
        if enabled {
            total += exec_line(&line[start..]);
        }
    }
    println!("{}", total);
}
