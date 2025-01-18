use std::fs::File;
use std::io::{BufRead, BufReader};
use std::collections::HashMap;
use std::env;

#[derive(Debug,Clone)]
enum Operation {
    And,
    Or,
    Xor,
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
    assert!(wire.starts_with('z'));
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
            if wire_values.get(&gate.3).unwrap().is_some() {
                continue;
            }
            if let Some(v1) = wire_values.get(&gate.0).unwrap() {
                if let Some(v2) = wire_values.get(&gate.1).unwrap() {
                    let value = match gate.2 {
                        Operation::And => v1 & v2,
                        Operation::Or => v1 | v2,
                        Operation::Xor => v1 ^ v2,
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
        if line.is_empty() {
            continue;
        }
        if line.contains(':') {
            let parts: Vec<&str> = line.split(": ").collect();
            let name = parts[0];
            if name.starts_with('x') {
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
                "AND" => Operation::And,
                "OR" => Operation::Or,
                "XOR" => Operation::Xor,
                _ => panic!("Unknown operation"),
            };
            let gate = (parts[0].to_string(), parts[2].to_string(), op, parts[4].to_string());
            gates.push(gate);
        }
    }
    let part1 = solve_circuit(&mut wire_values, &gates, n_input);
    println!("Part 1: {}", part1);
    let mut touched = vec![false; gates.len()];
    let mut n_wrong = 0;
    for i in 0..n_input+1 {
        let wire = format!("z{:02}", i);
        let indx_gates = get_upstream(&wire, &gates);
        let mut lines = Vec::<String>::new();
        let mut counter: (usize, usize, usize) = (0,0,0);
        for j in indx_gates {
            if touched[j] {
                continue;
            }
            touched[j] = true;
            let op = match gates[j].2 {
                Operation::And => {
                    counter.0 += 1;
                    "AND"
                },
                Operation::Or => {
                    counter.1 += 1;
                    "OR"
                },
                Operation::Xor => {
                    counter.2 += 1;
                    "XOR"
                }
            };
            lines.push(format!("{} {} {} -> {}", gates[j].0, op, gates[j].1, gates[j].3));
        }
        let wrong = counter.0 != 2 || counter.1 != 1 || counter.2 != 2;
        if i > 1 && i < n_input && wrong {
            println!("Something's wrong wiith wire z{:02}", i);
            println!("{}", lines.join("\n"));
            println!();
            n_wrong += 1;
        }
    }
    if n_wrong == 0 {
        println!("All good !");
    }
}
