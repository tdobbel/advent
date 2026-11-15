use anyhow::Result;
use serde_json::Value;
use std::cmp::Ordering;
use std::{env, fs};

struct Packet {
    id: usize,
    value: Value,
}

fn main() -> Result<()> {
    let filename = env::args().nth(1).expect("Missing input file\n");
    let content = fs::read_to_string(filename)?;
    let lines: Vec<&str> = content.split("\n").collect();
    let mut ptr: usize = 0;

    let mut packets: Vec<Packet> = Vec::new();

    let mut part1 = 0;
    while ptr < lines.len() {
        let left: Value = serde_json::from_str(lines[ptr + 0])?;
        let right: Value = serde_json::from_str(lines[ptr + 1])?;
        if compare_values(&left, &right) == Ordering::Less {
            part1 += (ptr / 3) + 1;
        }
        add_packet(&mut packets, left);
        add_packet(&mut packets, right);
        ptr += 3;
    }

    println!("Part 1: {}", part1);

    let div1: Value = serde_json::from_str("[[2]]")?;
    let idx1 = add_packet(&mut packets, div1);
    let div2: Value = serde_json::from_str("[[6]]")?;
    let idx2 = add_packet(&mut packets, div2);

    packets.sort();
    let mut part2 = 1;
    for (i, packet) in packets.iter().enumerate() {
        if packet.id == idx1 || packet.id == idx2 {
            part2 *= i + 1;
        }
    }

    println!("Part 2: {}", part2);

    Ok(())
}

fn add_packet(packets: &mut Vec<Packet>, value: Value) -> usize {
    let packet_id = packets.len();
    packets.push(Packet {
        id: packet_id,
        value,
    });
    packet_id
}

fn compare_values(left: &Value, right: &Value) -> Ordering {
    match (left, right) {
        (Value::Number(x), Value::Number(y)) => {
            let left_num = x.as_u64().unwrap();
            let right_num = y.as_u64().unwrap();
            left_num.cmp(&right_num)
        }
        (Value::Number(_), Value::Array(_)) => {
            let left_vec: Vec<Value> = vec![left.to_owned()];
            let left_value = Value::Array(left_vec);
            compare_values(&left_value, right)
        }
        (Value::Array(_), Value::Number(_)) => {
            let right_vec: Vec<Value> = vec![right.to_owned()];
            let right_value = Value::Array(right_vec);
            compare_values(left, &right_value)
        }
        (Value::Array(left_vec), Value::Array(right_vec)) => {
            let n = left_vec.len().max(right_vec.len());
            for i in 0..n {
                match (left_vec.get(i), right_vec.get(i)) {
                    (None, _) => {
                        return Ordering::Less;
                    }
                    (_, None) => {
                        return Ordering::Greater;
                    }
                    (Some(x), Some(y)) => {
                        let c = compare_values(x, y);
                        match c {
                            Ordering::Equal => continue,
                            _ => return c,
                        }
                    }
                }
            }
            Ordering::Equal
        }
        _ => unreachable!(),
    }
}

impl PartialEq for Packet {
    fn eq(&self, other: &Self) -> bool {
        return self.id == other.id;
    }
}

impl Eq for Packet {}

impl PartialOrd for Packet {
    fn partial_cmp(&self, other: &Self) -> Option<Ordering> {
        Some(self.cmp(other))
    }
}
impl Ord for Packet {
    fn cmp(&self, other: &Self) -> Ordering {
        compare_values(&self.value, &other.value)
    }
}
