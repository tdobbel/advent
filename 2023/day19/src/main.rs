use regex::Regex;
use std::collections::HashMap;
use std::env;
use std::fs::File;
use std::io::{BufRead, BufReader};

#[derive(Debug)]
struct Part {
    x: u32,
    m: u32,
    a: u32,
    s: u32,
}

impl Part {
    fn get_category(&self, c: char) -> u32 {
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
        let re = Regex::new(r"(\w+)=(\d+)").unwrap();
        for cap in re.captures_iter(s) {
            let category = cap.get(1).unwrap().as_str();
            let value = cap.get(2).unwrap().as_str().parse::<u32>().unwrap();
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

    fn sum(&self) -> u32 {
        self.x + self.m + self.a + self.s
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
    Compare(char, Comparator, u32, String),
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
            let value = caps.get(3).unwrap().as_str().parse::<u32>().unwrap();
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
        if let Some(result) = rule.apply(&part) {
            match result {
                "A" => return true,
                "R" => return false,
                _ => return process_part(part, workflows, result),
            }
        }
    }
    panic!("No rule matched for part: {:?}", part);
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
}
