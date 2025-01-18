use std::fs::File;
use std::io::{BufRead, BufReader};
use std::collections::{HashMap,HashSet};
use std::cmp::{min,max};
use std::env;
use rand::Rng;

#[derive(Debug,Clone)]
enum Operation {
    AND,
    OR,
    XOR,
}

#[allow(dead_code)]
fn get_kth_bit(n: u64, k: usize) -> u8 {
    ((n >> k) & 1) as u8
}

#[allow(dead_code)]
fn random_initialization(wire_values: &mut HashMap<String,Option<u8>>, n_input: usize) {
    for i in 0..n_input {
        let mut rng = rand::thread_rng();
        let xvar = format!("x{:02}", i);
        let yvar = format!("y{:02}", i);
        let xvalue = if rng.gen::<f64>() > 0.5 {1} else {0};
        let yvalue = if rng.gen::<f64>() > 0.5 {1} else {0};
        wire_values.get_mut(&xvar).map(|v| *v = Some(xvalue));
        wire_values.get_mut(&yvar).map(|v| *v = Some(yvalue));
    }
}

fn wires_to_number(wire_values: &HashMap<String,Option<u8>>, prefix: &str, n: usize) -> u64 {
    let mut result = 0;
    for i in 0..n {
        let wire = format!("{}{:02}",prefix, i);
        let value = match wire_values.get(&wire).unwrap() {
            Some(v) => *v,
            None => panic!("Wire {} not found", wire),
        };
        result |= (value as u64) << i;
    }
    result
}

fn get_upstream(wire: &str, gates: &[(String, String, Operation, String)]) -> Vec<usize> {
    assert!(wire.starts_with("z"));
    let mut result = Vec::<usize>::new();
    let mut outputs: Vec<&str> = Vec::new();
    for (i,gate) in gates.iter().enumerate() {
        if gate.3 == wire {
            result.push(i);
            outputs.push(&gate.0);
            outputs.push(&gate.1);
            break;
        }
    }
    while !outputs.is_empty(){
        let mut next = Vec::<&str>::new();
        for (i,gate) in gates.iter().enumerate() {
            if outputs.contains(&&gate.3[..]) {
                result.push(i);
                next.push(&gate.0);
                next.push(&gate.1);
            }
        }
        outputs = next;
    }
    result
}

fn solve_circuit(
    wire_values: &mut HashMap<String,Option<u8>>,
    gates: &[(String, String, Operation, String)],
    n_input: usize
) -> u64 {
    loop {
        let mut updated = false;
        for gate in gates {
            if let Some(_) = wire_values.get(&gate.3).unwrap() {
                continue;
            }
            if let Some(v1) = wire_values.get(&gate.0).unwrap() {
                if let Some(v2) = wire_values.get(&gate.1).unwrap() {
                    let value = match gate.2 {
                        Operation::AND => v1 & v2,
                        Operation::OR => v1 | v2,
                        Operation::XOR => v1 ^ v2,
                    };
                    wire_values.insert(gate.3.clone(), Some(value));
                    updated = true;
                }
            }
        }
        if !updated {
            break;
        }
    }
    wires_to_number(wire_values, "z", n_input+1)
}

fn main() {
    let args = env::args().nth(1).expect("Please provide an input file");
    let file = File::open(args).unwrap();
    let reader = BufReader::new(file);
    let mut wire_values = HashMap::<String,Option<u8>>::new();
    let mut n_input = 0;
    let mut gates: Vec<(String, String, Operation, String)> = Vec::new();
    for line in reader.lines() {
        let line = line.unwrap();
        if line.len() == 0 {
            continue;
        }
        if line.contains(':') {
            let parts: Vec<&str> = line.split(": ").collect();
            let name = parts[0];
            if name.starts_with("x") {
                n_input += 1;
            }
            let value = parts[1].parse::<u8>().unwrap();
            wire_values.insert(name.to_string(), Some(value));
        }
        else {
            let parts = line.split_whitespace().collect::<Vec<&str>>();
            for (i,p) in parts.iter().enumerate() {
                if i == 1 {
                    continue;
                }
                wire_values.entry(p.to_string()).or_insert(None);
            }
            let op = match parts[1] {
                "AND" => Operation::AND,
                "OR" => Operation::OR,
                "XOR" => Operation::XOR,
                _ => panic!("Unknown operation"),
            };
            let gate = (parts[0].to_string(), parts[2].to_string(), op, parts[4].to_string());
            gates.push(gate);
        }
    }
    let part1 = solve_circuit(&mut wire_values, &gates, n_input);
    println!("Part 1: {}", part1);
    for i in 0..n_input+1 {
        let wire = format!("z{:02}", i);
        let indx_gates = get_upstream(&wire, &gates);
        println!("## {}:", wire);
        for j in indx_gates {
            let op = match gates[j].2 {
                Operation::AND => "AND",
                Operation::OR => "OR",
                Operation::XOR => "XOR",
            };
            println!("{} {} {} -> {}", gates[j].0, op, gates[j].1, gates[j].3);
        }
        println!();
    }
}
