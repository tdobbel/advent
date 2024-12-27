use std::fs::File;
use std::io::{BufRead, BufReader};
use std::collections::{HashMap,HashSet};
use std::cmp::{min,max};
use std::env;

#[derive(Debug)]
enum Operation {
    AND,
    OR,
    XOR,
}

#[derive(Debug)]
struct Gate {
    operation: Operation,
    input1: String,
    input2: String,
    output: String,
    depth: usize,
}

fn get_kth_bit(n: u64, k: usize) -> u8 {
    ((n >> k) & 1) as u8
}

fn switch_outputs(gates: &mut Vec<Gate>, index1: usize, index2: usize) {
    let tmp = gates[index1].output.clone();
    gates[index1].output = gates[index2].output.clone();
    gates[index2].output = tmp;
}

fn solve_gates(gates: &mut Vec<Gate>, wire_values: &mut HashMap<String,Option<u8>>) -> bool{
    let n_wires = wire_values.len();
    let mut n_values = wire_values.iter().filter(|(_,v)| v.is_some()).count();
    let mut depth = 0;
    while n_values < n_wires {
        let prev = n_values;
        for gate in gates.iter_mut() {
            match wire_values.get(&gate.output).unwrap() {
                Some(_) => continue,
                None => (),
            };
            let v1 = match wire_values.get(&gate.input1).unwrap() {
                Some(v) => *v,
                None => continue,
            };
            let v2 = match wire_values.get(&gate.input2).unwrap() {
                Some(v) => *v,
                None => continue,
            };
            gate.depth = depth;
            wire_values.entry(gate.output.clone()).and_modify(|v| {
                *v = match gate.operation {
                    Operation::AND => Some(v1 & v2),
                    Operation::OR => Some(v1 | v2),
                    Operation::XOR => Some(v1 ^ v2),
                };
            });
            n_values += 1;
        }
        depth += 1;
        if n_values == prev {
            return false;
        }
    }
    true
}

fn find_upstream_gates(gates: &Vec<Gate>, wire: &String, found: &mut Vec<usize>) {
    for (i,gate) in gates.iter().enumerate() {
        if gate.output != *wire {
            continue;
        }
        if found.contains(&i) {
            continue;
        }
        found.push(i);
        find_upstream_gates(gates, &gate.input1, found);
        find_upstream_gates(gates, &gate.input2, found);
        return
    }
}

fn check_results(wire_values: &HashMap<String,Option<u8>>, expected: u64) -> Vec<String> {
    let z_wires = wire_values.keys().filter(|k| k.starts_with('z')).map(|v| v.clone()).collect::<Vec<String>>();
    let mut bad = Vec::<String>::new();
    for (i,wire) in z_wires.iter().enumerate() {
        let value = wire_values.get(wire).unwrap().unwrap();
        if value == get_kth_bit(expected, i) {
            continue;
        }
        bad.push(wire.clone());
    }
    bad
}

//fn solve_switch(
//    gates: &mut Vec<Gate>, initial_state: &HashMap<String,Option<u8>>, switches: Vec<(usize,usize)>, expected: u64
//) -> Option<Vec<(usize,usize)>> {
//    let mut wire_values = initial_state.clone();
//    solve_gates(gates, &mut wire_values);
//    let bad_bits = check_results(&wire_values, expected);
//    if switches.len() == 4 {
//        return if bad_bits.len() == 0 { Some(switches) } else { None };
//    }
//    let mut bad_wires = HashMap::<usize,usize>::new();
//    for wire in bad_bits.iter() {
//        let mut found = Vec::<usize>::new();
//        find_upstream_gates(gates, wire, &mut found);
//        for i in found {
//            bad_wires.entry(i).and_modify(|v| *v += 1).or_insert(1);
//        }
//    }
//    None
//}

fn main() {
    let args: Vec<String> = env::args().collect();
    let file = File::open(&args[1]).unwrap();
    let reader = BufReader::new(file);
    let mut wire_values = HashMap::<String,Option<u8>>::new();
    let mut gates = Vec::<Gate>::new();
    let mut n_values = 0;
    let mut zwires = Vec::<String>::new();
    let mut x: u64 = 0;
    let mut ix = 0;
    let mut y: u64 = 0;
    let mut iy = 0;
    for line in reader.lines() {
        let line = line.unwrap();
        if line.len() == 0 {
            continue;
        }
        if line.contains(':') {
            let parts: Vec<&str> = line.split(": ").collect();
            let name = parts[0];
            let value = parts[1].parse::<u8>().unwrap();
            wire_values.insert(name.to_string(), Some(value));
            n_values += 1;
            if name.starts_with('x') {
                x += (value as u64) << ix;
                ix += 1;
            }
            else {
                y += (value as u64) << iy;
                iy += 1;
            }
        }
        else {
            let parts = line.split_whitespace().collect::<Vec<&str>>();
            let op = match parts[1] {
                "AND" => Operation::AND,
                "OR" => Operation::OR,
                "XOR" => Operation::XOR,
                _ => panic!("Unknown operation"),
            };
            let gate = Gate {
                operation: op,
                input1: parts[0].to_string(),
                input2: parts[2].to_string(),
                output: parts[4].to_string(),
                depth: 0,
            };
            let wires = vec![gate.input1.clone(), gate.input2.clone(), gate.output.clone()];
            for wire in wires {
                if wire.starts_with('z') && !zwires.contains(&wire) {
                    zwires.push(wire.clone());
                }
                wire_values.entry(wire.clone()).or_insert(None);
            }
            gates.push(gate);
        }
    }
    zwires.sort();
    let initial_state = wire_values.clone();
    solve_gates(&mut gates, &mut wire_values);
    let expected = x + y;
    let mut result = 0;
    for (i,wire) in zwires.iter().enumerate() {
        let value = wire_values.get(wire).unwrap().unwrap();
        result += (value as u64) << i;
    }
    println!("Result: {}", result);
    // Part 2
    //let bad_bits = check_results(&wire_values, expected);
    //println!("Bad bits: {:?}", bad_bits);
    //let mut bad_gates = HashMap::<usize,usize>::new();
    //for wire in bad_bits.iter() {
    //    let mut found = Vec::<usize>::new();
    //    find_upstream_gates(&gates, wire, &mut found);
    //    for i in found {
    //        bad_gates.entry(i).and_modify(|v| *v += 1).or_insert(1);
    //    }
    //}
    //let max_count = *bad_gates.values().min().unwrap();
    //let indx_gate = bad_gates.iter().filter(|(_,v)| **v == max_count).map(|(k,_)| *k).collect::<Vec<usize>>();
    //println!("{}: {:?}", max_count, indx_gate);
    //let mut best: i32 = 0;
    //let mut best_switch = (0,0);
    //for i in 0..indx_gate.len()-1 {
    //    for j in i+1..indx_gate.len() {
    //        let gate1 = indx_gate[i];
    //        let gate2 = indx_gate[j];
    //        switch_outputs(&mut gates, indx_gate[i], indx_gate[j]);
    //        let mut wire_values = initial_state.clone();
    //        let solved = solve_gates(&mut gates, &mut wire_values);
    //        if !solved {
    //            switch_outputs(&mut gates, indx_gate[i], indx_gate[j]);
    //            continue;
    //        }
    //        let new_bad = check_results(&wire_values, expected).len();
    //        let save = bad_bits.len() as i32 - new_bad as i32;
    //        println!("{}", save);
    //        if save > best {
    //            best = save;
    //            best_switch = (min(gate1,gate2), max(gate1,gate2));
    //        }
    //        switch_outputs(&mut gates, indx_gate[i], indx_gate[j]);
    //    }
    //}
    //println!("Best switch: {:?}", best_switch);
}
