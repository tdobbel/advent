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

struct Swaps {
    swaps: Vec<(usize,usize)>,
    impact: Vec<String>
}

#[derive(Debug,Clone)]
struct Gate {
    operation: Operation,
    input1: String,
    input2: String,
    output: String,
    prev: Vec<usize>,
    next: Vec<usize>
}

fn get_kth_bit(n: u64, k: usize) -> u8 {
    ((n >> k) & 1) as u8
}

fn eval_sol(wire_values: &HashMap<String,Option<u8>>) -> u64 {
    let mut result: u64 = 0;
    for i in 0..46 {
        let wire = format!("z{:02}", i);
        let value = match wire_values.get(&wire).unwrap() {
            Some(v) => *v,
            None => panic!("Wire {} not solved", wire),
        };
        result += (value as u64) << i;
    }
    result
}

fn swap_outputs(gates: &mut Vec<Gate>, index1: usize, index2: usize) {
    let tmp = gates[index1].output.clone();
    gates[index1].output = gates[index2].output.clone();
    gates[index2].output = tmp;
}

fn random_initialization(wire_values: &mut HashMap<String,Option<u8>>) -> (u64,u64) {
    let mut x: u64 = 0;
    let mut y: u64 = 0;
    for i in 0..45 {
        let mut rng = rand::thread_rng();
        let xvar = format!("x{:02}", i);
        let yvar = format!("y{:02}", i);
        let xvalue = if rng.gen::<f64>() > 0.5 {1} else {0};
        let yvalue = if rng.gen::<f64>() > 0.5 {1} else {0};
        x += (xvalue as u64) << i;
        y += (yvalue as u64) << i;
        wire_values.get_mut(&xvar).map(|v| *v = Some(xvalue));
        wire_values.get_mut(&yvar).map(|v| *v = Some(yvalue));
    }
    (x,y)
}


fn build_connectivity(gates: &mut Vec<Gate>) {
    for i in 0..gates.len() {
        for j in 0..gates.len() {
            if i == j {
                continue;
            }
            if gates[i].input1 == gates[j].output || gates[i].input2 == gates[j].output {
                gates[i].prev.push(j);
                gates[j].next.push(i);
            }
        }
    }
}

fn get_impacted(gates: &Vec<Gate>, gate_indx: usize) -> Vec<String> {
    let mut next = gates[gate_indx].next.clone();
    let mut bits = Vec::<String>::new();
    while next.len() > 0 {
        let mut next_ = Vec::<usize>::new();
        for ig in next.iter() {
            let gate = &gates[*ig];
            if gate.next.len() > 0 {
                next_.extend(gate.next.clone());
            }
            else if gate.output.starts_with('z') {
                let bit = gate.output.to_string().clone();
                if !bits.contains(&bit) {
                    bits.push(bit)
                }
            }
        }
        next = next_;
    }
    bits
}

fn solve_gates(gates: &Vec<Gate>, wire_values: &mut HashMap<String,Option<u8>>) -> bool{
    let n_wires = wire_values.len();
    let mut n_values = wire_values.iter().filter(|(_,v)| v.is_some()).count();
    while n_values < n_wires {
        let n_prev = n_values;
        for gate in gates.iter() {
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
            wire_values.entry(gate.output.clone()).and_modify(|v| {
                *v = match gate.operation {
                    Operation::AND => Some(v1 & v2),
                    Operation::OR => Some(v1 | v2),
                    Operation::XOR => Some(v1 ^ v2),
                };
            });
            n_values += 1;
        }
        if n_values == n_prev {
            return false;
        }
    }
    true
}

fn recompute_gates(
    gates: &Vec<Gate>, wire_values: &mut HashMap<String,Option<u8>>, next: &Vec<usize>
) {
    for i in next.iter() {
        let gate = &gates[*i];
        let v1 = wire_values.get(&gate.input1).unwrap().unwrap();
        let v2 = wire_values.get(&gate.input2).unwrap().unwrap();
        wire_values.entry(gate.output.clone()).and_modify(|v| {
            *v = match gate.operation {
                Operation::AND => Some(v1 & v2),
                Operation::OR => Some(v1 | v2),
                Operation::XOR => Some(v1 ^ v2),
            };
        });
        let out_gates = &gates[*i].next;
        recompute_gates(gates, wire_values, out_gates);
    }
}

fn find_upstream_gates(
    gates: &Vec<Gate>, wire: &String, wire_values: &HashMap<String,Option<u8>>, expected: u64,
    to_one: &mut HashSet<usize>, to_zero: &mut HashSet<usize>
) {
    let index = gates.iter().position(|g| g.output == *wire).unwrap();
    let mut gate_ids = HashSet::<usize>::new();
    gate_ids.insert(index);
    let mut previous = gates[index].prev.clone();
    while previous.len() > 0 {
        let mut next = Vec::<usize>::new();
        for p in previous.iter() {
            gate_ids.insert(*p);
            next.extend(gates[*p].prev.clone());
        }
        previous = next.clone();
    }
    for start_gate in gate_ids.iter() {
        let mut new_values = wire_values.clone();
        let output_tag = gates[*start_gate].output.clone();
        let value = new_values.get(&output_tag).unwrap().unwrap();
        let iszero = value == 0;
        new_values.entry(output_tag).and_modify(|v| *v = Some(if iszero {1} else {0}));
        let next = &gates[*start_gate].next;
        recompute_gates(gates, &mut new_values, next);
        let bits = get_impacted(gates, *start_gate);
        let bad_bits = check_results(&new_values, expected);
        let mut ok = true;
        for bit in bad_bits.iter() {
            if bits.contains(bit) {
                ok = false;
                break;
            }
        }
        if !ok {
            continue
        }
        if iszero {
            to_one.insert(*start_gate);
        }
        else {
            to_zero.insert(*start_gate);
        }
    }
}

fn test_swaps(gates: &Vec<Gate>, state0: &HashMap<String,Option<u8>>, swaps: &Swaps, n_try: usize) -> Option<usize> {
    let mut gate_ = gates.clone();
    let mut n_correct: usize = 0;
    for (l,r) in swaps.swaps.iter() {
        swap_outputs(&mut gate_, *l, *r);
    }
    for _ in 0..n_try {
        let mut wire_values = state0.clone();
        let (x,y) = random_initialization(&mut wire_values);
        let solved = solve_gates(&gate_, &mut wire_values);
        if !solved {
            return None;
        }
        let bad = check_results(&wire_values, x+y);
        if bad.len() == 0 {
            n_correct += 1;
        }
        for bit in bad.iter() {
            if swaps.impact.contains(bit) {
                return None;
            }
        }
    }
    Some(n_correct)
}

fn combine_swaps(swaps1: &Swaps, swaps2: &Swaps, gates: &Vec<Gate>, state0: &HashMap<String,Option<u8>>) -> Option<Swaps> {
    for (l1,r1) in swaps1.swaps.iter() {
        for (l2,r2) in swaps2.swaps.iter() {
            if *l1 == *l2 || *r1 == *r2 || *l1 == *r2 || *r1 == *l2 {
                return None;
            }
        }
    }
    for bit in swaps1.impact.iter() {
        if swaps2.impact.contains(bit) {
            return None;
        }
    }
    let mut swaps_ = swaps1.swaps.clone();
    swaps_.extend(swaps2.swaps.clone());
    let mut impact = swaps1.impact.clone();
    impact.extend(swaps2.impact.clone());
    let swaps_test = Swaps{swaps: swaps_, impact};
    let n_correct = test_swaps(gates, state0, &swaps_test, 10);
    match n_correct {
        Some(_) => Some(swaps_test),
        None => None
    }
}

fn find_swaps(
    swaps: &Vec<Swaps>, gates: &Vec<Gate>, state0: &HashMap<String,Option<u8>>,
) -> Option<Vec<(usize,usize)>>{
    for i in 0..swaps.len()-1 {
        for j in i+1..swaps.len() {
            let combined = combine_swaps(&swaps[i], &swaps[j], &gates, &state0);
            match combined {
                Some(sw) => {
                    match test_swaps(&gates, &state0, &sw, 50)  {
                        Some(v) => {
                            if v == 50 {
                                return Some(sw.swaps);
                            }
                            else {
                                continue
                            }
                        },
                        None => continue
                    }
                },
                None => continue
            }
        }
    }
    None
}

fn check_results(wire_values: &HashMap<String,Option<u8>>, expected: u64) -> Vec<String> {
    let mut bad = Vec::<String>::new();
    for i in 0..46 {
        let bitname = format!("z{:02}", i);
        let bitvalue = wire_values.get(&bitname).unwrap().unwrap();
        if bitvalue == get_kth_bit(expected, i) {
            continue
        }
        bad.push(bitname.clone());
    }
    bad
}

fn main() {
    let args: Vec<String> = env::args().collect();
    let file = File::open(&args[1]).unwrap();
    let reader = BufReader::new(file);
    let mut wire_values = HashMap::<String,Option<u8>>::new();
    let mut gates = Vec::<Gate>::new();
    let mut zwires = Vec::<String>::new();
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
                prev: Vec::<usize>::new(),
                next: Vec::<usize>::new()
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
    build_connectivity(&mut gates);
    let mut zbits = Vec::<Vec<String>>::new();
    for i in 0..gates.len() {
        zbits.push(get_impacted(&gates, i));
    }
    let initial_state = wire_values.clone();
    solve_gates(&gates, &mut wire_values);
    let result = eval_sol(&wire_values);
    println!("Result: {}", result);
    // Part 2
    let mut swaps = Vec::<Swaps>::new();
    let mut tested = Vec::<(usize,usize)>::new();
    for itest in 0..1 {
        let (x,y) = random_initialization(&mut wire_values);
        let expected = x + y;
        solve_gates(&gates, &mut wire_values);
        let bad_bits = check_results(&wire_values, expected);
        let mut to_one = HashSet::<usize>::new();
        let mut to_zero = HashSet::<usize>::new();
        for wire in bad_bits.iter() {
            find_upstream_gates(&gates, wire, &mut wire_values, expected, &mut to_one, &mut to_zero);
        }
        for i in to_one.iter() {
            for j in to_zero.iter() {
                let swap = (min(*i,*j), max(*i,*j));
                if tested.contains(&swap) {
                    continue
                }
                tested.push(swap);
                let mut bits = zbits[*i].clone();
                let mut ok = true;
                for bit in zbits[*j].iter() {
                    if bits.contains(bit) {
                        ok = false;
                        break;
                    }
                }
                if !ok {
                    continue
                }
                bits.extend(zbits[*j].clone());
                let swap_object = Swaps{swaps: vec![swap], impact: bits.clone()};
                let ok = test_swaps(&gates, &initial_state, &swap_object, 20);
                match ok {
                    Some(_) => swaps.push(swap_object),
                    None => continue
                }
            }
        }
    }
    let mut swap_pairs = Vec::<Swaps>::new();
    println!("{} swaps", swaps.len());
    for i in 0..swaps.len()-1{
        println!("{}/{}", i+1, swaps.len());
        for j in i+1..swaps.len() {
            let swap = combine_swaps(&swaps[i], &swaps[j], &gates, &initial_state);
            match swap {
                Some(v) => swap_pairs.push(v),
                None => continue
            }
        }
    }
    println!("{} swap pairs", swap_pairs.len());
    let solution = find_swaps(&swap_pairs, &gates, &initial_state);
    let solvalue = match solution {
        Some(v) => v,
        None => panic!("No solution found"),
    };
    let mut names = Vec::<&str>::new();
    for (l,r) in solvalue.iter() {
        names.push(&gates[*l].output);
        names.push(&gates[*r].output);
    }
    names.sort();
    println!("{}", names.join(","));
}
