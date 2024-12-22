use std::fs::File;
use std::io::{BufRead, BufReader};
use std::collections::HashMap;
use std::cmp::min;
use std::env;

fn icon_to_dx(icon: char) -> (i32,i32) {
    match icon {
        '^' => (-1,0),
        'v' => (1,0),
        '<' => (0,-1),
        '>' => (0,1),
        _ => panic!("Invalid icon: {}", icon),
    }
}

#[allow(dead_code)]
fn keys_to_motion(keys: &str, keypad: &HashMap<char,(i32,i32)>, position: &mut (i32,i32), nogo: (i32,i32)) -> String {
    let mut motions = String::new();
    for key in keys.chars() {
        let dst = keypad.get(&key).unwrap();
        while *position != *dst {
            let delta_row = dst.0 - position.0;
            let delta_col = dst.1 - position.1;
            for motion in "^>v<".chars() {
                let (dy, dx) = icon_to_dx(motion);
                if delta_col*dx == 0 && delta_row*dy == 0 {
                    continue;
                }
                if delta_row*dy < 0 || delta_col*dx < 0 {
                    continue;
                }
                let new_pos = (position.0 + dy, position.1 + dx);
                if new_pos != nogo {
                    motions.push(motion);
                    *position = new_pos;
                    break;
                }
            }
        }
        motions.push_str("A");
    }
    motions
}

fn get_all_paths(
    pos_from: (i32,i32), key_to: &char, keypad: &HashMap<char,(i32,i32)>,
    nogo: (i32,i32), prev: &str, motions: &mut Vec<String>
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
        if delta_col*dx == 0 && delta_row*dy == 0 {
            continue;
        }
        if delta_row*dy < 0 || delta_col*dx < 0 {
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

#[allow(dead_code)]
fn motion_to_keys(motions: &str, keypad: &HashMap<char,(i32,i32)>, pos: &mut (i32,i32), nogo: (i32,i32)) -> String {
    let mut keys = String::new();
    for c in motions.chars() {
        match c {
            '^' => pos.0 -= 1,
            'v' => pos.0 += 1,
            '<' => pos.1 -= 1,
            '>' => pos.1 += 1,
            'A' => {
                let key = keypad.iter().find(|&x| *x.1 == *pos).unwrap().0;
                keys.push_str(&key.to_string());
            },
            _ => panic!("Invalid motion: {}", c),
        };
        if *pos == nogo {
            panic!("Invalid position: {:?}", pos);
        }

    }
    keys
}

#[allow(dead_code)]
fn sanity_check(human_moves: &str, numeric_keypad: &HashMap<char,(i32,i32)>, directional_keypad: &HashMap<char,(i32,i32)>){
    let mut pos1 = (3,2);
    let mut pos2 = (0,2);
    let mut pos3 = (0,2);
    println!("Human ({} moves): {}", human_moves.len(), human_moves);
    let moves3 = motion_to_keys(human_moves, &directional_keypad, &mut pos3, (0,0));
    println!("Robot 3 ({} moves): {}", moves3.len(),moves3);
    let moves2 = motion_to_keys(&moves3, &directional_keypad, &mut pos2, (0,0));
    println!("Robot 2 ({} moves): {}", moves2.len(), moves2);
    let keys = motion_to_keys(&moves2, &numeric_keypad, &mut pos1, (3,0));
    println!("keys pressed: {}", keys);
}

fn part1(line: &str, numeric_keypad: &HashMap<char,(i32,i32)>, directional_keypad: &HashMap<char,(i32,i32)>) -> usize {
    let mut options = Vec::<String>::new();
    options.push(line.to_string());
    for idepth in 0..3 {
        let keypad;
        let nogo;
        let start_pos;
        if idepth == 0 {
            keypad = numeric_keypad;
            nogo = (3,0);
            start_pos = (3,2);
        }
        else {
            keypad = directional_keypad;
            nogo = (0,0);
            start_pos = (0,2);
        }
        let mut next_options = Vec::<String>::new();
        for option in options.iter() {
            let mut pos = start_pos;
            let mut prev = Vec::<String>::new();
            prev.push(String::new());
            for c in option.chars() {
                let seed = String::new();
                let mut motions = Vec::<String>::new();
                get_all_paths(pos, &c, keypad, nogo, &seed, &mut motions);
                let mut next = Vec::<String>::new();
                pos = *keypad.get(&c).unwrap();
                for head in prev.iter() {
                    for tail in motions.iter() {
                        next.push(format!("{}{}", head, tail));
                    }
                }
                prev = next;
            }
            for moves in prev.iter() {
                next_options.push(moves.to_string());
            }
        }
        options = next_options;
    }
    let mut best = usize::MAX;
    for option in options {
        best = min(best, option.len());
    }
    println!("{}: {}", line, best);
    let numeric_value = &line[..line.len()-1].parse::<usize>().unwrap();
    return numeric_value * best;
}

fn main() {
    let args: Vec<String> = env::args().collect();
    assert_eq!(args.len(), 2);

    // Create numeric keypad
    let mut numeric_keypad = HashMap::<char,(i32,i32)>::new();
    numeric_keypad.insert('0', (3,1));
    numeric_keypad.insert('A', (3,2));
    numeric_keypad.insert('1', (2,0));
    numeric_keypad.insert('2', (2,1));
    numeric_keypad.insert('3', (2,2));
    numeric_keypad.insert('4', (1,0));
    numeric_keypad.insert('5', (1,1));
    numeric_keypad.insert('6', (1,2));
    numeric_keypad.insert('7', (0,0));
    numeric_keypad.insert('8', (0,1));
    numeric_keypad.insert('9', (0,2));

    // Create directional keypad
    let mut directional_keypad = HashMap::<char,(i32,i32)>::new();
    directional_keypad.insert('^', (0,1));
    directional_keypad.insert('A', (0,2));
    directional_keypad.insert('<', (1,0));
    directional_keypad.insert('v', (1,1));
    directional_keypad.insert('>', (1,2));

    let file = File::open(&args[1]).unwrap();
    let reader = BufReader::new(file);
    let mut total = 0;
    for line in reader.lines() {
        let line = line.unwrap();
        total += part1(&line, &numeric_keypad, &directional_keypad);
    }
    println!("Sum of complexities: {}", total);
}
