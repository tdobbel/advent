use anyhow::Result;
use std::env;
use std::fs;

fn main() -> Result<()> {
    let filepath = env::args().nth(1).expect("Please provide an input file");
    let file_content = fs::read_to_string(filepath)?;
    let lines: Vec<&str> = file_content.split('\n').collect();
    let timestamp: u64 = lines[0].parse()?;
    let mut min_wait_time = u64::MAX;
    let mut part1 = 0;
    let mut constraints: Vec<(u64, u64)> = Vec::new();
    for (i, bus) in lines[1].split(',').enumerate() {
        if bus == "x" {
            continue;
        }
        let bus_period: u64 = bus.parse()?;
        constraints.push((bus_period, i as u64));
        if timestamp.is_multiple_of(bus_period) {
            part1 = 0;
            break;
        }
        let wait_time = bus_period - timestamp % bus_period;
        if wait_time < min_wait_time {
            min_wait_time = wait_time;
            part1 = bus_period * wait_time;
        }
    }
    println!("Part 1: {}", part1);
    let mut step = constraints[0].0;
    let mut part2 = step;
    for (per, dt) in constraints.iter().skip(1) {
        while !(part2 + *dt).is_multiple_of(*per) {
            part2 += step;
        }
        // Here we assume that all the bus periods are prime numbers (otherwise lcm should be computed)
        step *= *per;
    }

    println!("Part 2: {}", part2);

    Ok(())
}
