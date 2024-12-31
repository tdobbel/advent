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
    gates: &Vec<Gate>, wire: &String, wire_values: &HashMap<String,Option<u8>>, bad_bits: &Vec<String>,
    to_one: &mut HashMap<usize,Vec<String>>, to_zero: &mut HashMap<usize,Vec<String>>
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
        let mut ok = true;
        let mut changed = Vec::<String>::new();
        for (tag, value) in new_values.iter() {
            if !tag.starts_with('z') {
                continue;
            }
            if wire_values[tag].unwrap() != value.unwrap() {
                if !bad_bits.contains(tag) {
                    ok = false;
                    break;
                }
                changed.push(tag.clone());
            }
        }
        if !ok || changed.len() == 0 {
            continue;
        }
        if iszero {
            let taglist = to_one.entry(*start_gate).or_insert(Vec::<String>::new());
            for tag in changed.iter() {
                if !taglist.contains(tag) {
                    taglist.push(tag.clone());
                }
            }
        }
        else {
            let taglist = to_zero.entry(*start_gate).or_insert(Vec::<String>::new());
            for tag in changed.iter() {
                if !taglist.contains(tag) {
                    taglist.push(tag.clone());
                }
            }
        }
    }
}

fn find_swaps(
    gates: &Vec<Gate>, state0: &HashMap<String,Option<u8>>, swaps: &Vec<(usize,usize)>,
    impacted: &Vec<Vec<String>>, index: usize, accum: Vec<(usize,usize)>, bits: Vec<String>
) -> Option<Vec<(usize,usize)>>{
    if accum.len() == 4 {
        let mut wire_values = state0.clone();
        let mut gates_ = gates.clone();
        for (l,r) in accum.iter() {
            swap_outputs(&mut gates_, *l, *r);
        }
        let mut ok = true;
        for _ in 0..100 {
            let (x,y) = random_initialization(&mut wire_values);
            let expected = x+y;
            let sol = solve_gates(&gates_, &mut wire_values);
            if !sol {
                ok = false;
                break;
            }
            let bad_bits = check_results(&wire_values, expected);
            if bad_bits.len()  > 0{
                ok = false;
                break
            }
        }
        return if ok {Some(accum)} else {None};
    }
    for i in index..swaps.len() {
        let (l,r) = swaps.get(i).unwrap();
        let mut touched = false;
        for sw in accum.iter() {
            if *l == sw.0 || *l == sw.1 || *r == sw.0 || *r == sw.1 {
                touched = true;
                break;
            }
        }
        if touched {
            continue
        }
        let mut skip = false;
        let mut bits_ = bits.clone();
        for bit in impacted[*l].iter() {
            if bits.contains(bit) {
                skip = true;
                break
            }
            bits_.push(bit.clone());
        }
        for bit in impacted[*r].iter() {
            if bits.contains(bit) {
                skip = true;
                break
            }
            bits_.push(bit.clone());
        }
        if skip {
            continue
        }
        let mut swaps_ = accum.clone();
        swaps_.push((*l,*r));
        let sol = find_swaps(gates, state0, swaps, impacted, i, swaps_, bits_);
        match sol {
            Some(v) => return Some(v),
            None => continue
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
    let mut swaps = Vec::<(usize,usize)>::new();
    let mut tested = Vec::<(usize,usize)>::new();
    for _ in 0..100 {
        let (x,y) = random_initialization(&mut wire_values);
        let expected = x + y;
        solve_gates(&gates, &mut wire_values);
        let bad_bits = check_results(&wire_values, expected);
        let mut to_one = HashMap::<usize,Vec<String>>::new();
        let mut to_zero = HashMap::<usize,Vec<String>>::new();
        for wire in bad_bits.iter() {
            find_upstream_gates(&gates, wire, &mut wire_values, &bad_bits, &mut to_one, &mut to_zero);
        }
        for (i,_) in to_one.iter() {
            for (j,_) in to_zero.iter() {
                let swap = (min(*i,*j), max(*i,*j));
                if tested.contains(&swap) {
                    continue
                }
                tested.push(swap);
                let mut bits = Vec::<String>::new();
                for bit in zbits[*i].iter() {
                    if !bits.contains(bit) {
                        bits.push(bit.clone())
                    }
                }
                for bit in zbits[*j].iter() {
                    if !bits.contains(bit) {
                        bits.push(bit.clone());
                    }
                }
                swap_outputs(&mut gates, *i, *j);
                let mut ok = true;
                for _ in 0..100 {
                //wire_values = state0.clone();
                    wire_values = initial_state.clone();
                    let (x,y) = random_initialization(&mut wire_values);
                    let expected = x+y;
                    let solved = solve_gates(&gates, &mut wire_values);
                    if !solved {
                        ok = false;
                        break;
                    }
                    let bad_ = check_results(&wire_values, expected);
                    for bit in bad_.iter() {
                        if bits.contains(bit) {
                            ok = false;
                            break;
                        }
                    }
                    if !ok {
                        break
                    }
                }
                swap_outputs(&mut gates, *i, *j);
                if ok {
                    swaps.push(swap);
                }
            }
        }
    }
    println!("{} swaps candidates", swaps.len());
    let solution = find_swaps(
        &gates, &initial_state, &swaps, &zbits, 0, Vec::<(usize,usize)>::new(), Vec::<String>::new()
    );
    match solution {
        Some(v) => println!("{:?}", v),
        None => println!("Couille in the potage :("),
    };
}
