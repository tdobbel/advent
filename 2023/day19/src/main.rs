use regex::Regex;
use std::collections::HashMap;
use std::env;
use std::fs::File;
use std::io::{BufRead, BufReader};

#[derive(Debug)]
struct Part {
    x: usize,
    m: usize,
    a: usize,
    s: usize,
}

#[derive(Debug, Clone)]
struct PartBounds {
    x: (usize, usize),
    m: (usize, usize),
    a: (usize, usize),
    s: (usize, usize),
}

impl Part {
    fn get_category(&self, c: char) -> usize {
        match c {
            'x' => self.x,
            'm' => self.m,
            'a' => self.a,
            's' => self.s,
            _ => panic!("Unkown category: {}", c),
        }
    }

    fn from_string(s: &str) -> Part {
        let mut part = Part {
            x: 0,
            m: 0,
            a: 0,
            s: 0,
        };
        let re = Regex::new(r"([xmas])=(\d+)").unwrap();
        for cap in re.captures_iter(s) {
            let category = cap.get(1).unwrap().as_str();
            let value = cap.get(2).unwrap().as_str().parse::<usize>().unwrap();
            match category {
                "x" => part.x = value,
                "m" => part.m = value,
                "a" => part.a = value,
                "s" => part.s = value,
                _ => panic!("Unkown category: {}", category),
            }
        }
        part
    }

    fn sum(&self) -> usize {
        self.x + self.m + self.a + self.s
    }
}

impl PartBounds {
    fn create() -> PartBounds {
        PartBounds {
            x: (1, 4000),
            m: (1, 4000),
            a: (1, 4000),
            s: (1, 4000),
        }
    }

    fn get_bound(&self, c: char) -> (usize, usize) {
        match c {
            'x' => self.x,
            'm' => self.m,
            'a' => self.a,
            's' => self.s,
            _ => panic!("Unkown category: {}", c),
        }
    }

    fn set_bounds(self, c: char, value: (usize, usize)) -> PartBounds {
        let mut result = self.clone();
        match c {
            'x' => result.x = value,
            'm' => result.m = value,
            'a' => result.a = value,
            's' => result.s = value,
            _ => panic!("Unkown category: {}", c),
        }
        result
    }

    fn volume(&self) -> usize {
        let mut volume = 1;
        for c in ['x', 'm', 'a', 's'].iter() {
            let (min, max) = self.get_bound(*c);
            volume *= max - min + 1;
        }
        volume
    }

    fn intersection(&self, other: PartBounds) -> Option<PartBounds> {
        let mut result = PartBounds::create();
        for c in ['x', 'm', 'a', 's'].iter() {
            let (min, max) = self.get_bound(*c);
            let (omin, omax) = other.get_bound(*c);
            let (imin, imax) = (min.max(omin), max.min(omax));
            if imin > imax {
                return None;
            }
            result = result.set_bounds(*c, (imin, imax));
        }
        Some(result)
    }
}

fn sum_intersection(
    bounds: &[PartBounds],
    start_index: usize,
    current: Option<PartBounds>,
    n_set: usize,
    total: &mut usize,
) {
    if n_set == 0 {
        if let Some(curr) = current {
            *total += curr.volume();
        }
        return;
    }
    for i in start_index..bounds.len() - n_set + 1 {
        let other = bounds.get(i).unwrap();
        match current {
            None => sum_intersection(bounds, i + 1, Some(other.clone()), n_set - 1, total),
            Some(ref curr) => {
                if let Some(inter) = curr.intersection(other.clone()) {
                    sum_intersection(bounds, i + 1, Some(inter.clone()), n_set - 1, total);
                }
            }
        }
    }
}

#[derive(Debug)]
enum Comparator {
    Greater,
    Lower,
}

#[derive(Debug)]
enum Rule {
    Return(String),
    Compare(char, Comparator, usize, String),
}

impl Rule {
    fn apply(&self, part: &Part) -> Option<&str> {
        match self {
            Rule::Return(result) => Some(result),
            Rule::Compare(category, op, v, result) => {
                let x = part.get_category(*category);
                match op {
                    Comparator::Greater => {
                        if x > *v {
                            Some(result)
                        } else {
                            None
                        }
                    }
                    Comparator::Lower => {
                        if x < *v {
                            Some(result)
                        } else {
                            None
                        }
                    }
                }
            }
        }
    }
}

fn parse_workflow(line: &str, workflows: &mut HashMap<String, Vec<Rule>>) {
    let mut line = line.split('{');
    let name = line.next().unwrap().trim();
    let rules = line.next().unwrap();
    let rules = &rules[..rules.len() - 1].split(',').collect::<Vec<&str>>();
    let mut workflow: Vec<Rule> = Vec::new();
    let re = Regex::new(r"([xmas])([<>])(\d+):(\w+)").unwrap();
    for rule in rules.iter() {
        if re.is_match(rule) {
            let caps = re.captures(rule).unwrap();
            let category = caps.get(1).unwrap().as_str().chars().next().unwrap();
            let comparator = caps.get(2).unwrap().as_str().chars().next().unwrap();
            let value = caps.get(3).unwrap().as_str().parse::<usize>().unwrap();
            let result = caps.get(4).unwrap().as_str();
            let op = match comparator {
                '>' => Comparator::Greater,
                '<' => Comparator::Lower,
                _ => panic!("Unkown comparator: {}", comparator),
            };
            workflow.push(Rule::Compare(category, op, value, result.to_string()));
        } else {
            workflow.push(Rule::Return(rule.to_string()));
        }
    }
    workflows.insert(name.to_string(), workflow);
}

fn process_part(part: &Part, workflows: &HashMap<String, Vec<Rule>>, name: &str) -> bool {
    let workflow = workflows.get(name).unwrap();
    for rule in workflow.iter() {
        if let Some(result) = rule.apply(part) {
            match result {
                "A" => return true,
                "R" => return false,
                _ => return process_part(part, workflows, result),
            }
        }
    }
    panic!("No rule matched for part: {:?}", part);
}

fn process_bounds(
    part_bounds: PartBounds,
    workflows: &HashMap<String, Vec<Rule>>,
    name: &str,
    rule_index: usize,
    processed: &mut Vec<PartBounds>,
) {
    let workflow = workflows.get(name).unwrap();
    let rule = workflow.get(rule_index).unwrap();
    match rule {
        Rule::Return(result) => {
            match result.as_str() {
                "A" => processed.push(part_bounds),
                "R" => (),
                _ => process_bounds(part_bounds.clone(), workflows, result, 0, processed),
            };
        }
        Rule::Compare(cat, op, value, label) => match op {
            Comparator::Lower => {
                let (vmin, vmax) = part_bounds.get_bound(*cat);
                let bounds_lower = part_bounds.clone().set_bounds(*cat, (vmin, *value - 1));
                match label.as_str() {
                    "A" => processed.push(bounds_lower.clone()),
                    "R" => (),
                    _ => process_bounds(bounds_lower.clone(), workflows, label, 0, processed),
                };
                if rule_index < workflow.len() - 1 {
                    let bounds_upper = part_bounds.set_bounds(*cat, (*value, vmax));
                    process_bounds(bounds_upper, workflows, name, rule_index + 1, processed);
                }
            }
            Comparator::Greater => {
                let (vmin, vmax) = part_bounds.get_bound(*cat);
                let bounds_upper = part_bounds.clone().set_bounds(*cat, (*value + 1, vmax));
                match label.as_str() {
                    "A" => processed.push(bounds_upper.clone()),
                    "R" => (),
                    _ => process_bounds(bounds_upper.clone(), workflows, label, 0, processed),
                };
                if rule_index < workflow.len() - 1 {
                    let bounds_lower = part_bounds.set_bounds(*cat, (vmin, *value));
                    process_bounds(bounds_lower, workflows, name, rule_index + 1, processed);
                }
            }
        },
    }
}

fn main() {
    let args = env::args().nth(1).expect("Please provide an input file");
    let file = File::open(args).unwrap();
    let reader = BufReader::new(file);
    let mut workflows: HashMap<String, Vec<Rule>> = HashMap::new();
    let mut parts: Vec<Part> = Vec::new();
    let mut is_workflow = true;
    for line in reader.lines() {
        let line = line.unwrap();
        if line.is_empty() {
            is_workflow = false;
            continue;
        }
        if is_workflow {
            parse_workflow(&line, &mut workflows);
        } else {
            parts.push(Part::from_string(&line));
        }
    }
    let mut part1 = 0;
    for part in parts.iter() {
        if process_part(part, &workflows, "in") {
            part1 += part.sum();
        }
    }
    println!("Part 1: {} accepted parts", part1);
    let part_bounds = PartBounds::create();
    let mut processed: Vec<PartBounds> = Vec::new();
    process_bounds(part_bounds, &workflows, "in", 0, &mut processed);
    let mut part2 = processed.iter().map(|b| b.volume()).sum::<usize>();
    for n in 2..=processed.len() {
        let mut incr = 0;
        sum_intersection(&processed, 0, None, n, &mut incr);
        if incr == 0 {
            break;
        }
        if n % 2 == 0 {
            part2 -= incr;
        } else {
            part2 += incr;
        }
    }
    println!("Part 2: {}", part2);
}
