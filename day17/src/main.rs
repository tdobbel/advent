use std::fs::File;
use std::io::{BufRead, BufReader};
use std::env;

#[derive(Debug)]
struct Register {
    a: u32,
    b: u32,
    c: u32,
}


fn get_combo(literal: u32, register: &Register) -> u32 {
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

fn execute_program(program: &Vec<char>, register: &mut Register, op_pointer: &mut usize, output: &mut Vec<u32>) {
    let opcode = program[*op_pointer];
    let literal = program[*op_pointer + 1].to_digit(10).unwrap();
    let combo = get_combo(literal, register);
    match opcode {
        '0' => {
            let div = register.a / u32::pow(2, combo);
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
            output.push(combo % 8);
            *op_pointer += 2;
        },
        '6' => {
            let div = register.a / u32::pow(2, combo);
            register.b = div;
            *op_pointer += 2;
        },
        '7' => {
            let div = register.a / u32::pow(2, combo);
            register.c = div;
            *op_pointer += 2;
        },
        _ => panic!("Invalid opcode"),
    };
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
            let value = line.split_whitespace().last().unwrap().parse::<u32>().unwrap();
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
    let mut output = Vec::<u32>::new();
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
}
