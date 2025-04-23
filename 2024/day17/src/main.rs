use std::env;
use std::fs::File;
use std::io::{BufRead, BufReader};

#[derive(Debug)]
struct Register {
    a: u64,
    b: u64,
    c: u64,
}

fn get_combo(literal: u64, register: &Register) -> u64 {
    if literal < 4 {
        literal
    } else if literal == 4 {
        register.a
    } else if literal == 5 {
        register.b
    } else if literal == 6 {
        register.c
    } else {
        panic!("Not supposed to be here");
    }
}

fn execute_program(
    program: &[char],
    register: &mut Register,
    op_pointer: &mut usize,
    output: &mut Vec<char>,
) {
    let opcode = program[*op_pointer];
    let literal = program[*op_pointer + 1].to_digit(10).unwrap() as u64;
    let combo = get_combo(literal, register);
    match opcode {
        '0' => {
            let div = register.a >> combo;
            register.a = div;
            *op_pointer += 2;
        }
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
            } else {
                *op_pointer = literal as usize;
            }
        }
        '4' => {
            let result = register.b ^ register.c;
            register.b = result;
            *op_pointer += 2;
        }
        '5' => {
            let chr = char::from_digit((combo as u32) % 8, 10).unwrap();
            output.push(chr);
            *op_pointer += 2;
        }
        '6' => {
            register.b = register.a >> combo;
            *op_pointer += 2;
        }
        '7' => {
            register.c = register.a >> combo;
            *op_pointer += 2;
        }
        _ => panic!("Invalid opcode"),
    };
}

// I definitely cheated for this one. Thank you to HyperNeutrino for his video
fn part2(program: &Vec<char>, prev: u64, n: usize) -> Option<u64> {
    if n == 0 {
        return Some(prev);
    }
    for i in 0..8 {
        let a = prev << 3 | i;
        let mut register = Register { a, b: 0, c: 0 };
        let mut op_pointer = 0;
        let mut output = Vec::<char>::new();
        while op_pointer < program.len() - 2 {
            execute_program(program, &mut register, &mut op_pointer, &mut output);
        }
        let value = output.pop().unwrap();
        if value != program[n - 1] {
            continue;
        } else if let Some(v) = part2(program, a, n - 1) {
            return Some(v);
        }
    }
    None
}

fn main() {
    let args: Vec<String> = env::args().collect();
    assert_eq!(args.len(), 2);
    let file = File::open(&args[1]).unwrap();
    let reader = BufReader::new(file);
    let mut register = Register { a: 0, b: 0, c: 0 };
    let mut program: Vec<char> = Vec::new();
    let mut n_line = 0;
    for (i, line) in reader.lines().enumerate() {
        n_line += 1;
        let line = line.unwrap();
        if i < 3 {
            let value = line
                .split_whitespace()
                .last()
                .unwrap()
                .parse::<u64>()
                .unwrap();
            match i {
                0 => register.a = value,
                1 => register.b = value,
                2 => register.c = value,
                _ => panic!("Invalid register"),
            }
            continue;
        } else if i == 4 {
            let pslice = line.split(": ").last().unwrap();
            program = pslice
                .split(',')
                .map(|x| x.chars().next().unwrap())
                .collect();
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
    for (i, val) in output.iter().enumerate() {
        print!("{}", val);
        if i < output.len() - 1 {
            print!(",");
        }
    }
    println!();
    match part2(&program, 0, program.len()) {
        Some(v) => println!("Register A must be set to {}", v),
        None => println!("No solution found"),
    }
    //test(&program);
}
