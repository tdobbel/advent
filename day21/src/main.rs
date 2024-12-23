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

fn keys_to_motions(keys: &str, key_pad: &HashMap<char,(i32,i32)>, pos: &mut (i32,i32), nogo: (i32,i32)) -> String {
    let mut motions = String::new();
    for c in keys.chars() {
        let pos_to = key_pad.get(&c).unwrap();
        while pos != pos_to {
            let delta_row = pos_to.0 - pos.0;
            let delta_col = pos_to.1 - pos.1;
            let mut options = Vec::<char>::new();
            if delta_row != 0 {
                options.push(if delta_row > 0 {'v'} else {'^'});
            }
            if delta_col != 0 {
                options.push(if delta_col > 0 {'>'} else {'<'});
            }
            for motion in options.iter() {
                let (dy, dx) = icon_to_dx(*motion);
                let new_pos = (pos.0 + dy, pos.1 + dx);
                if new_pos != nogo {
                    motions.push(*motion);
                    *pos = new_pos;
                    break;
                }
            }
        }
        motions.push('A');
    }
    motions
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

fn min_distance(
    char_from: char, char_to: char, keypad: &HashMap<char,(i32,i32)> , depth: i32,
    cache: &mut HashMap<(char,char,i32),i32>, paths: &HashMap<(char,char),Vec<String>>
) -> i32 {
    if depth == 0 {
        let pfrom = *keypad.get(&char_from).unwrap();
        let pto = *keypad.get(&char_to).unwrap();
        return (pfrom.0 - pto.0).abs() + (pfrom.1 - pto.1).abs() + 1;
    }
    let paths_ = paths.get(&(char_from,char_to)).unwrap();
    let mut shortest = i32::MAX;
    for path in paths_.iter() {
        let mut kfrom = 'A';
        let mut path_length = 0;
        for c in path.chars() {
            let d = match cache.get(&(kfrom, c, depth-1)) {
                Some(&v) => v,
                None => {
                    let v = min_distance(kfrom, c, keypad, depth-1, cache, paths);
                    cache.insert((kfrom, c, depth-1), v);
                    v
                },
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

#[allow(dead_code)]
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
                let mut cutoff = usize::MAX;
                for tail in motions.iter() {
                    cutoff = min(tail.len(), cutoff);
                }
                pos = *keypad.get(&c).unwrap();
                for head in prev.iter() {
                    for tail in motions.iter() {
                        if tail.len() > cutoff{
                            continue
                        }
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

    let mut cache = HashMap::<(char,char,i32),i32>::new();
    let mut paths = HashMap::<(char,char),Vec<String>>::new();
    for (key_from, pos_from) in directional_keypad.iter() {
        for (key_to,_) in directional_keypad.iter() {
            let mut motions = Vec::<String>::new();
            get_all_paths(*pos_from, key_to, &directional_keypad, (0,0), "", &mut motions);
            paths.insert((*key_from,*key_to), motions);
        }
    }
    let file = File::open(&args[1]).unwrap();
    let reader = BufReader::new(file);
    for line in reader.lines() {
        let line = line.unwrap();
        let moves = keys_to_motions(&line, &numeric_keypad, &mut (3,2), (3,0));
        let mut total = 0;
        for i in 0..moves.len()-1 {
            let key_from = moves.chars().nth(i).unwrap();
            let key_to = moves.chars().nth(i+1).unwrap();
            total += min_distance(key_from, key_to, &directional_keypad, 2, &mut cache, &paths);
        }
        println!("{}: {}", line, total);
    }
    //println!("Sum of complexities: {}", total);
}
