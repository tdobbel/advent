use std::fs::File;
use std::io::{BufRead, BufReader};
use std::env;

#[derive(Debug)]
struct Register {
    a: u64,
    b: u64,
    c: u64,
}

fn get_combo(literal: u64, register: &Register) -> u64 {
    if literal < 4 {
        return literal;
    }
    else if literal == 4 {
        return register.a;
    }
    else if literal == 5 {
        return register.b;
    }
    else if literal == 6 {
        return register.c;
    }
    else {
        panic!("Not supposed to be here");
    }
}

fn execute_program(program: &Vec<char>, register: &mut Register, op_pointer: &mut usize, output: &mut Vec<char>) -> bool{
    let opcode = program[*op_pointer];
    let literal = program[*op_pointer + 1].to_digit(10).unwrap() as u64;
    let combo = get_combo(literal, register);
    let mut ok = true;
    match opcode {
        '0' => {
            let div = register.a / u64::pow(2, combo as u32);
            register.a = div;
            *op_pointer += 2;
        },
        '1' => {
            let result = register.b ^ literal;
            register.b = result;
            *op_pointer += 2
        }
        '2' => {
            register.b = combo % 8;
            *op_pointer += 2;
        }
        '3' => {
            if register.a == 0 {
                *op_pointer += 2;
            }
            else {
                *op_pointer = literal as usize;
            }
        },
        '4' => {
            let result = register.b ^ register.c;
            register.b = result;
            *op_pointer += 2;
        },
        '5' => {
            let chr = char::from_digit((combo as u32) % 8, 10).unwrap();
            output.push(chr);
            let ip = output.len();
            if ip > program.len() {
                ok = false;
            }
            else if ip > 0 && program[ip-1] != output[ip-1] {
                ok = false;
            }
            *op_pointer += 2;
        },
        '6' => {
            let div = register.a / u64::pow(2, combo as u32);
            register.b = div;
            *op_pointer += 2;
        },
        '7' => {
            let div = register.a / u64::pow(2, combo as u32);
            register.c = div;
            *op_pointer += 2;
        },
        _ => panic!("Invalid opcode"),
    };
    ok
}

#[allow(dead_code)]
fn part2(program: &Vec<char>) -> u64 {
    let start = u64::pow(8,program.len() as u32-1);
    let mut step = 10;
    let stop = start*8;
    let mut solution = 0;
    for a0 in start..stop {
        let progress = (a0-start)*100/(stop-start);
        if progress >= step {
            println!("Progress: {} %", progress);
            step += 10;
        }
        let mut register = Register{a:a0, b:0, c:0};
        let mut output = Vec::<char>::new();
        let mut op_pointer = 0;
        while op_pointer < program.len() {
            let isok = execute_program(&program, &mut register, &mut op_pointer, &mut output);
            if !isok {
                break;
            }
        }
        if output.len() == program.len() {
            solution = a0;
            break;
        }
    }
    solution
}

#[allow(dead_code)]
fn test(program: &Vec<char>) {
    let a0 = u64::pow(8,program.len() as u32-1);
    for i in 0..10000 {
        let mut register = Register{a:a0+i, b:0, c:0};
        let mut output = Vec::<char>::new();
        let mut op_pointer = 0;
        while op_pointer < program.len() {
            execute_program(&program, &mut register, &mut op_pointer, &mut output);
        }
        let diff_size = output.len() as i32 - program.len() as i32;
        if output[0] == program[0] {
            println!("a0: {}; a0 % 8: {}, diff size: {}", a0+i, (a0+i)%8, diff_size);
        }
    }
}

fn main() {
    let args: Vec<String> = env::args().collect();
    assert_eq!(args.len(), 2);
    let file = File::open(&args[1]).unwrap();
    let reader = BufReader::new(file);
    let mut register = Register{a:0, b:0, c:0};
    let mut program: Vec<char> = Vec::new();
    let mut n_line = 0;
    for (i,line) in reader.lines().enumerate() {
        n_line += 1;
        let line = line.unwrap();
        if i < 3 {
            let value = line.split_whitespace().last().unwrap().parse::<u64>().unwrap();
            match i {
                0 => register.a = value,
                1 => register.b = value,
                2 => register.c = value,
                _ => panic!("Invalid register"),
            }
            continue;
        }
        else if i == 4{
            let pslice = line.split(": ").last().unwrap();
            program = pslice.split(",").map(|x| x.chars().next().unwrap()).collect();
        }
    }
    if n_line != 5 {
        panic!("Invalid input file");
    }
    let mut op_pointer: usize = 0;
    let mut output = Vec::<char>::new();
    while op_pointer < program.len() {
        execute_program(&program, &mut register, &mut op_pointer, &mut output);
    }
    for (i,val) in output.iter().enumerate() {
        print!("{}", val);
        if i < output.len()-1 {
            print!(",");
        }
    }
    println!();
    let a_start = part2(&program);
    println!("Register A must be set to {}", a_start);
    //test(&program);
}
