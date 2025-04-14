use std::collections::HashMap;
use std::env;
use std::fs::File;
use std::io::{BufRead, BufReader};

fn icon_to_dx(icon: char) -> (i32, i32) {
    match icon {
        '^' => (-1, 0),
        'v' => (1, 0),
        '<' => (0, -1),
        '>' => (0, 1),
        _ => panic!("Invalid icon: {}", icon),
    }
}

fn get_all_paths(
    pos_from: (i32, i32),
    key_to: &char,
    keypad: &HashMap<char, (i32, i32)>,
    nogo: (i32, i32),
    prev: &str,
    motions: &mut Vec<String>,
) {
    let pos_to = keypad.get(key_to).unwrap();
    if pos_from == *pos_to {
        motions.push(format!("{}A", prev));
        return;
    }
    let delta_row = pos_to.0 - pos_from.0;
    let delta_col = pos_to.1 - pos_from.1;
    for motion in "<v^>".chars() {
        let (dy, dx) = icon_to_dx(motion);
        if delta_col * dx == 0 && delta_row * dy == 0 {
            continue;
        }
        if delta_row * dy < 0 || delta_col * dx < 0 {
            continue;
        }
        let new_pos = (pos_from.0 + dy, pos_from.1 + dx);
        if new_pos == nogo {
            continue;
        }
        let keys = format!("{}{}", prev, motion);
        get_all_paths(new_pos, key_to, keypad, nogo, &keys, motions);
    }
}

fn get_all_num_paths(line: &str, num_keypad: &HashMap<char, (i32, i32)>) -> Vec<String> {
    let mut seqs = vec![String::new()];
    let nogo = (3, 0);
    let mut pos = (3, 2);
    for key in line.chars() {
        let mut next_seqs = Vec::<String>::new();
        let mut motions = Vec::<String>::new();
        get_all_paths(pos, &key, num_keypad, nogo, "", &mut motions);
        for head in seqs.iter() {
            for tail in motions.iter() {
                next_seqs.push(format!("{}{}", head, tail));
            }
        }
        seqs = next_seqs;
        pos = *num_keypad.get(&key).unwrap();
    }
    seqs
}

fn build_paths(
    keypad: &HashMap<char, (i32, i32)>,
    nogo: (i32, i32),
) -> HashMap<(char, char), Vec<String>> {
    let mut paths = HashMap::<(char, char), Vec<String>>::new();
    for (key_from, pos_from) in keypad.iter() {
        for (key_to, _) in keypad.iter() {
            let mut motions = Vec::<String>::new();
            get_all_paths(*pos_from, key_to, keypad, nogo, "", &mut motions);
            paths.insert((*key_from, *key_to), motions);
        }
    }
    paths
}

fn min_distance(
    char_from: char,
    char_to: char,
    keypad: &HashMap<char, (i32, i32)>,
    depth: i32,
    cache: &mut HashMap<(char, char, i32), u64>,
    paths: &HashMap<(char, char), Vec<String>>,
) -> u64 {
    if depth == 1 {
        let pfrom = *keypad.get(&char_from).unwrap();
        let pto = *keypad.get(&char_to).unwrap();
        let dist = (pfrom.0 - pto.0).abs() + (pfrom.1 - pto.1).abs() + 1;
        return dist as u64;
    }
    let seqs = paths.get(&(char_from, char_to)).unwrap();
    let mut shortest = u64::MAX;
    for path in seqs.iter() {
        let mut kfrom = 'A';
        let mut path_length: u64 = 0;
        for c in path.chars() {
            let d = match cache.get(&(kfrom, c, depth - 1)) {
                Some(&v) => v,
                None => {
                    let v = min_distance(kfrom, c, keypad, depth - 1, cache, paths);
                    cache.insert((kfrom, c, depth - 1), v);
                    v
                }
            };
            path_length += d;
            kfrom = c;
        }
        if path_length < shortest {
            shortest = path_length;
        }
    }
    shortest
}

fn main() {
    let args: Vec<String> = env::args().collect();
    assert_eq!(args.len(), 2);

    // Create numeric keypad
    let mut numeric_keypad = HashMap::<char, (i32, i32)>::new();
    numeric_keypad.insert('0', (3, 1));
    numeric_keypad.insert('A', (3, 2));
    numeric_keypad.insert('1', (2, 0));
    numeric_keypad.insert('2', (2, 1));
    numeric_keypad.insert('3', (2, 2));
    numeric_keypad.insert('4', (1, 0));
    numeric_keypad.insert('5', (1, 1));
    numeric_keypad.insert('6', (1, 2));
    numeric_keypad.insert('7', (0, 0));
    numeric_keypad.insert('8', (0, 1));
    numeric_keypad.insert('9', (0, 2));

    // Create directional keypad
    let mut directional_keypad = HashMap::<char, (i32, i32)>::new();
    directional_keypad.insert('^', (0, 1));
    directional_keypad.insert('A', (0, 2));
    directional_keypad.insert('<', (1, 0));
    directional_keypad.insert('v', (1, 1));
    directional_keypad.insert('>', (1, 2));

    let dir_seqs = build_paths(&directional_keypad, (0, 0));
    let mut cache = HashMap::<(char, char, i32), u64>::new();

    let file = File::open(&args[1]).unwrap();
    let reader = BufReader::new(file);

    let mut total_complexity: u64 = 0;
    for line in reader.lines() {
        let line = line.unwrap();
        let seqs = get_all_num_paths(&line, &numeric_keypad);
        let mut optimal = u64::MAX;
        for seq in seqs.iter() {
            let mut keyfrom = 'A';
            let mut length = 0;
            for keyto in seq.chars() {
                length += min_distance(
                    keyfrom,
                    keyto,
                    &directional_keypad,
                    25,
                    &mut cache,
                    &dir_seqs,
                );
                keyfrom = keyto;
            }
            if length < optimal {
                optimal = length;
            }
        }
        println!("{}: {}", line, optimal);
        let num_value = &line[..line.len() - 1].parse::<i32>().unwrap();
        total_complexity += (*num_value as u64) * optimal;
    }
    println!("Sum of complexities: {}", total_complexity);
}
