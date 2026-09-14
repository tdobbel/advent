use anyhow::Result;
use serde_json::Value;
use std::{env, fs};

fn count_total(obj: &Value, total: &mut i64, ignore_red: bool) {
    match &obj {
        &Value::Array(v) => {
            for item in v.iter() {
                count_total(item, total, ignore_red);
            }
        }
        &Value::Number(n) => {
            *total += n.as_i64().unwrap();
        }
        &Value::String(_) => {}
        &Value::Object(map) => {
            if ignore_red {
                for v in map.values() {
                    if let Value::String(s) = v
                        && s == "red"
                    {
                        return;
                    }
                }
            }
            for v in map.values() {
                count_total(v, total, ignore_red);
            }
        }
        _ => panic!("Unexpected data type"),
    }
}

fn main() -> Result<()> {
    let filename = env::args().nth(1).expect("Missing input file");
    let content = fs::read_to_string(filename)?;
    let data: Value = serde_json::from_str(content.as_str())?;
    let mut part1: i64 = 0;
    let mut part2: i64 = 0;
    count_total(&data, &mut part1, false);
    count_total(&data, &mut part2, true);
    println!("Part 1: {}", part1);
    println!("Part 2: {}", part2);
    Ok(())
}
