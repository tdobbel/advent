use std::fs::File;
use std::io::{BufRead, BufReader};
use std::collections::{HashMap,HashSet};
use std::env;

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
    depth: usize,
}

fn get_kth_bit(n: u64, k: usize) -> u8 {
    ((n >> k) & 1) as u8
}

fn eval_sol(wire_values: &HashMap<String,Option<u8>>, z_wires: &Vec<String>) -> u64 {
    let mut result: u64 = 0;
    for (i,wire) in z_wires.iter().enumerate() {
        let value = match wire_values.get(wire).unwrap() {
            Some(v) => *v,
            None => panic!("Wire {} not solved", wire),
        };
        result += (value as u64) << i;
    }
    result
}

fn switch_outputs(gates: &mut Vec<Gate>, index1: usize, index2: usize) {
    let tmp = gates[index1].output.clone();
    gates[index1].output = gates[index2].output.clone();
    gates[index2].output = tmp;
}

fn build_connectivity(gates: &Vec<Gate>) -> HashMap<usize,(Vec<usize>, Vec<usize>)>{
    let mut conn = HashMap::<usize,(Vec<usize>, Vec<usize>)>::new();
    for i in 0..gates.len() {
        conn.entry(i).or_insert((Vec::<usize>::new(), Vec::<usize>::new()));
    }
    for (i,gate) in gates.iter().enumerate() {
        for (j,other) in gates.iter().enumerate() {
            if gate.input1 == other.output || gate.input2 == other.output {
                conn.entry(i).and_modify(|v| v.0.push(j));
                conn.entry(j).and_modify(|v| v.1.push(i));
            }
        }
    }
    conn
}

fn attribute_depth(
    gates: &mut Vec<Gate>, conn: &HashMap<usize,(Vec<usize>,Vec<usize>)>, indices: &Vec<usize>, depth: usize
) {
    for ig in indices.iter() {
        gates[*ig].depth = depth;
        let next = conn.get(ig).unwrap().1.clone();
        attribute_depth(gates, conn, &next, depth + 1);
    }
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
    gates: &Vec<Gate>, wire_values: &mut HashMap<String,Option<u8>>,
    connectivity: &HashMap<usize,(Vec<usize>,Vec<usize>)>, next: &Vec<usize>
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
        let c = connectivity.get(i).unwrap();
        recompute_gates(gates, wire_values, connectivity, &c.1);
    }
}

fn find_upstream_gates(
    gates: &Vec<Gate>, wire: &String, wire_values: &HashMap<String,Option<u8>>,
    bad_bits: &Vec<String>, connectivity: &HashMap<usize,(Vec<usize>,Vec<usize>)>,
    to_one: &mut HashMap<usize,Vec<String>>, to_zero: &mut HashMap<usize,Vec<String>>
) {
    let index = gates.iter().position(|g| g.output == *wire).unwrap();
    let mut gate_ids = HashSet::<usize>::new();
    gate_ids.insert(index);
    let conn = connectivity.get(&index).unwrap();
    let mut previous = conn.0.clone();
    while previous.len() > 0 {
        let mut next = Vec::<usize>::new();
        for p in previous.iter() {
            gate_ids.insert(*p);
            let c = connectivity.get(p).unwrap();
            next.extend(c.0.clone());
        }
        previous = next.clone();
    }
    for start_gate in gate_ids.iter() {
        let mut new_values = wire_values.clone();
        let output_tag = gates[*start_gate].output.clone();
        let value = new_values.get(&output_tag).unwrap().unwrap();
        let iszero = value == 0;
        new_values.entry(output_tag).and_modify(|v| *v = Some(if iszero {1} else {0}));
        let next = connectivity.get(start_gate).unwrap();
        recompute_gates(gates, &mut new_values, connectivity, &next.1);
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

fn find_switches(
    saves: &HashMap<(usize,usize),Vec<String>>, start_index: i32, left: Vec<usize>, right: Vec<usize>,
    fixed_bits: Vec<String>, gates: &Vec<Gate>, initial_state: &HashMap<String,Option<u8>>,
    z_wires: &Vec<String>, expected: u64, results: &mut Vec<String>
) {
    if left.len() == 4 {
        if fixed_bits.len() == 14 {
            let result: Vec<(usize,usize)> = (0..4).map(|i| (left[i],right[i])).collect();
            let mut test_gates = gates.clone();
            for (l,r) in result.iter() {
                switch_outputs(&mut test_gates, *l, *r);
            }
            let mut wire_values = initial_state.clone();
            solve_gates(&test_gates, &mut wire_values);
            let bad_bits = check_results(&wire_values, z_wires, expected);
            if bad_bits.len() > 0 {
                return;
            }
            let mut switches = Vec::<&str>::new();
            for (l,r) in result.iter() {
                switches.push(gates[*l].output.as_str());
                switches.push(gates[*r].output.as_str());
            }
            switches.sort();
            println!("{}", switches.join(","));
            results.push(switches.join(","));
        }
        return
    }
    let mut index: i32 = -1;
    for (pair,saved) in saves.iter() {
        index += 1;
        if index < start_index {
            continue;
        }
        let (lhs, rhs) = pair;
        if left.contains(lhs) || right.contains(rhs) {
            continue;
        }
        let mut add = true;
        for bit in saved.iter() {
            if fixed_bits.contains(bit) {
                add = false;
                break;
            }
        }
        if !add {
            continue;
        }
        let mut fixed = fixed_bits.clone();
        fixed.extend(saved.clone());
        let mut left_ = left.clone();
        left_.push(*lhs);
        let mut right_ = right.clone();
        right_.push(*rhs);
        find_switches(saves, index, left_, right_, fixed, gates, initial_state, z_wires, expected, results);
    }
}

fn check_results(wire_values: &HashMap<String,Option<u8>>, z_wires: &Vec<String>, expected: u64) -> Vec<String> {
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

fn main() {
    let args: Vec<String> = env::args().collect();
    let file = File::open(&args[1]).unwrap();
    let reader = BufReader::new(file);
    let mut wire_values = HashMap::<String,Option<u8>>::new();
    let mut gates = Vec::<Gate>::new();
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
    let connectivity = build_connectivity(&gates);
    let mut indx0 = Vec::<usize>::new();
    for (i,c) in connectivity.iter() {
        if c.0.len() == 0 {
            indx0.push(*i);
        }
    }
    attribute_depth(&mut gates, &connectivity, &indx0, 0);
    let mut z_wires = wire_values.keys().filter(|k| k.starts_with('z')).map(|v| v.clone()).collect::<Vec<String>>();
    z_wires.sort();
    let initial_state = wire_values.clone();
    solve_gates(&gates, &mut wire_values);
    let expected = x + y;
    let result = eval_sol(&wire_values, &z_wires);
    println!("Result: {}", result);
    // Part 2
    let bad_bits = check_results(&wire_values, &z_wires, expected);
    let mut to_one = HashMap::<usize,Vec<String>>::new();
    let mut to_zero = HashMap::<usize,Vec<String>>::new();
    for wire in bad_bits.iter() {
        find_upstream_gates(&gates, wire, &mut wire_values, &bad_bits, &connectivity, &mut to_one, &mut to_zero);
    }
    let mut saves = HashMap::<(usize,usize),Vec<String>>::new();
    for (indx_left,_) in to_one.iter() {
        for (indx_right,_) in to_zero.iter() {
            if gates[*indx_left].depth != gates[*indx_right].depth {
                continue;
            }
            switch_outputs(&mut gates, *indx_left, *indx_right);
            let mut wire_values = initial_state.clone();
            let solved = solve_gates(&gates, &mut wire_values);
            switch_outputs(&mut gates, *indx_left, *indx_right);
            if !solved {
                continue;
            }
            let new_bad = check_results(&wire_values, &z_wires, expected);
            // If more broken bits or no save, skip
            if new_bad.len() >= bad_bits.len() {
                continue
            }
            let mut ok = true;
            // If any of the new broken bits are not in the original set, skip
            for wire in new_bad.iter() {
                if !bad_bits.contains(wire) {
                    ok = false;
                    break;
                }
            }
            if !ok {
                continue
            }
            let mut saved_bits = Vec::<String>::new();
            for wire in bad_bits.iter() {
                if !new_bad.contains(wire) {
                    saved_bits.push(wire.clone());
                }
            }
            if ok {
                //println!("({} {}) -> {:?}", indx_left, indx_right, saved_bits);
                saves.insert((*indx_left,*indx_right), saved_bits);
            }
        }
    }
    println!("Switches found: {}", saves.len());
    println!("Inch allah");
    let mut results = Vec::<String>::new();
    find_switches(
        &saves, 0, Vec::<usize>::new(), Vec::<usize>::new(), Vec::<String>::new(),
        &gates, &initial_state, &z_wires, expected, &mut results
    );
}
