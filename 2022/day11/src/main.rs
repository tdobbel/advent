use anyhow::{Result, anyhow};
use std::env;
use std::fs;

#[derive(Clone, Copy, Debug)]
enum Operator {
    Product,
    Sum,
}

#[derive(Clone, Copy, Debug)]
struct MonkeyTest {
    divider: u64,
    monkey_true: usize,
    monkey_false: usize,
}

#[derive(Clone, Copy, Debug)]
struct MonkeyOperation {
    operator: Operator,
    rhs: Option<u64>,
}

#[derive(Clone, Debug)]
struct Monkey {
    worry_levels: Vec<u64>,
    operation: MonkeyOperation,
    test: MonkeyTest,
    counter: u64,
}

fn main() -> Result<()> {
    let filename = env::args().nth(1).expect("Missing input file");
    let content = fs::read_to_string(filename)?;
    let lines: Vec<&str> = content.split("\n").collect();
    let mut ptr: usize = 0;

    let mut global_div: u64 = 1;

    let mut monkeys: Vec<Monkey> = Vec::new();
    let mut monkeys_copy: Vec<Monkey> = Vec::new();
    while ptr < lines.len() {
        let monkey = Monkey::from_desc(&lines[ptr..], &mut global_div)?;
        monkeys_copy.push(monkey.clone());
        monkeys.push(monkey);
        ptr += 7;
    }
    for _ in 0..20 {
        next_round(&mut monkeys, None);
    }
    println!("Part 1: {}", monkey_business(&monkeys));

    for _ in 0..10000 {
        next_round(&mut monkeys_copy, Some(global_div));
    }
    println!("Part 2: {}", monkey_business(&monkeys_copy));

    Ok(())
}

impl MonkeyTest {
    fn apply(&self, x: u64) -> usize {
        if x.is_multiple_of(self.divider) {
            self.monkey_true
        } else {
            self.monkey_false
        }
    }

    fn from_desc(lines: &[&str], global_div: &mut u64) -> Result<MonkeyTest> {
        let divider = parse_last_number(lines[0])?;
        *global_div *= divider;
        Ok(MonkeyTest {
            divider,
            monkey_true: parse_last_number(lines[1])? as usize,
            monkey_false: parse_last_number(lines[2])? as usize,
        })
    }
}

impl MonkeyOperation {
    fn apply(&self, old: u64) -> u64 {
        let rhs = match self.rhs {
            Some(x) => x,
            None => old,
        };
        match self.operator {
            Operator::Sum => old + rhs,
            Operator::Product => old * rhs,
        }
    }

    fn from_desc(line: &str) -> Result<MonkeyOperation> {
        let words: Vec<&str> = line.split_whitespace().collect();
        let last_word = words.last().unwrap();
        let rhs = if last_word == &"old" {
            None
        } else {
            Some(last_word.parse::<u64>()?)
        };
        let operator = match *words.iter().nth_back(1).unwrap() {
            "+" => Operator::Sum,
            "*" => Operator::Product,
            _ => return Err(anyhow!("Invalid operator symbol")),
        };
        Ok(MonkeyOperation { operator, rhs })
    }
}

impl Monkey {
    fn apply_test(&self, x: u64) -> usize {
        self.test.apply(x)
    }

    fn apply_operation(&self, old: u64) -> u64 {
        self.operation.apply(old)
    }

    fn from_desc(lines: &[&str], global_div: &mut u64) -> Result<Monkey> {
        let operation = MonkeyOperation::from_desc(lines[2])?;
        let test = MonkeyTest::from_desc(&lines[3..], global_div)?;
        let (_, rhs) = lines[1].split_once(": ").unwrap();
        let worry_levels: Vec<u64> = rhs.split(", ").map(|w| w.parse::<u64>().unwrap()).collect();

        Ok(Monkey {
            worry_levels,
            operation,
            test,
            counter: 0,
        })
    }
}

fn parse_last_number(line: &str) -> Result<u64> {
    let last_word = line.split_whitespace().last().unwrap();
    Ok(last_word.parse::<u64>()?)
}

fn next_round(monkeys: &mut [Monkey], global_div: Option<u64>) {
    for i in 0..monkeys.len() {
        while let Some(old) = monkeys[i].worry_levels.pop() {
            let (index, new_worry) = {
                let monkey = &mut monkeys[i];
                let mut new_worry = monkey.apply_operation(old);
                if let Some(div) = global_div {
                    new_worry %= div;
                } else {
                    new_worry /= 3;
                }
                let index = monkey.apply_test(new_worry);
                monkey.counter += 1;
                (index, new_worry)
            };
            monkeys[index].worry_levels.push(new_worry);
        }
    }
}

fn monkey_business(monkeys: &[Monkey]) -> u64 {
    let n = monkeys.len();
    let mut counters: Vec<u64> = monkeys.iter().map(|monkey| monkey.counter).collect();
    counters.sort();
    counters[n - 1] * counters[n - 2]
}
