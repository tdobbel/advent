use anyhow::Result;
use regex::Regex;
use std::env;
use std::fs::File;
use std::io::{BufRead, BufReader};

struct Machine {
    n_bit: usize,
    diagram: u32,
    buttons: Vec<Vec<u8>>,
    requirements: Vec<u16>,
}

fn button_as_u32(button: &[u8], n_bit: usize) -> u32 {
    let mut num = 0;
    for x in button.iter() {
        num |= 1 << (n_bit - (*x as usize) - 1);
    }
    num
}

impl Machine {
    fn from_line(line: &str) -> Self {
        let (head, line) = line.split_once("]").unwrap();
        let mut diagram = 0;
        let mut n_bit = 0;
        for c in head[1..].chars() {
            diagram <<= 1;
            n_bit += 1;
            if c == '#' {
                diagram += 1;
            }
        }
        let mut buttons: Vec<Vec<u8>> = Vec::new();
        let pattern = Regex::new(r"\(([0-9,]+)\)").unwrap();
        for cap in pattern.captures_iter(line) {
            let button_inner = cap.get(1).unwrap().as_str();
            let button: Vec<u8> = button_inner
                .split(",")
                .map(|x| x.parse().unwrap())
                .collect();
            buttons.push(button);
        }
        let (_, line) = line.split_once('{').unwrap();
        let n = line.len();
        let requirements: Vec<u16> = line[..n - 1]
            .split(',')
            .map(|x| x.parse().unwrap())
            .collect();
        Self {
            n_bit,
            diagram,
            buttons,
            requirements,
        }
    }

    fn upper_bound(&self, button_index: usize) -> usize {
        let mut upper = u16::MAX;
        for i in self.buttons[button_index].iter() {
            upper = upper.min(self.requirements[*i as usize]);
        }
        upper as usize
    }

    fn check_solution(&self, x: &[f32]) -> Option<u32> {
        let n_col = self.buttons.len();
        let mut ix: Vec<u16> = vec![0; n_col];
        for (i, &val) in x.iter().enumerate() {
            let rounded = val.round();
            if rounded < 0.0 {
                return None;
            }
            ix[i] = rounded as u16;
        }
        let mut sol: Vec<u16> = vec![0; self.requirements.len()];
        for (i, button) in self.buttons.iter().enumerate() {
            for &k in button.iter() {
                sol[k as usize] += ix[i];
            }
        }
        for (i, &num) in sol.iter().enumerate() {
            if num != self.requirements[i] {
                return None;
            }
        }
        Some(ix.iter().map(|&x| x as u32).sum())
    }
}

struct System {
    a: Vec<Vec<f32>>,
    b: Vec<f32>,
    x: Vec<f32>,
    free: Vec<bool>,
}

impl System {
    fn from_machine(machine: &Machine) -> Self {
        let n_rows = machine.requirements.len();
        let n_cols = machine.buttons.len();
        let b = machine.requirements.iter().map(|x| *x as f32).collect();
        let mut a = vec![vec![0.0; n_cols]; n_rows];
        let x = vec![0.0; n_cols];
        for (j, button) in machine.buttons.iter().enumerate() {
            for i in button.iter() {
                a[*i as usize][j] = 1.0;
            }
        }
        Self {
            a,
            b,
            x,
            free: vec![true; n_cols],
        }
    }

    fn gaussian_elimination(&mut self) {
        let mut h: usize = 0;
        let mut k: usize = 0;
        let n_rows = self.a.len();
        let n_cols = self.a[0].len();
        while h < n_rows && k < n_cols {
            let mut imax: usize = h;
            for i in h + 1..n_rows {
                if self.a[i][k].abs() > self.a[imax][k].abs() {
                    imax = i;
                }
            }
            if self.a[imax][k].abs() < f32::EPSILON {
                k += 1;
                continue;
            }
            self.a.swap(imax, h);
            self.b.swap(imax, h);
            for i in h + 1..n_rows {
                let fac = self.a[i][k] / self.a[h][k];
                self.a[i][k] = 0.0;
                self.b[i] -= fac * self.b[h];
                for j in k + 1..n_cols {
                    self.a[i][j] -= fac * self.a[h][j];
                }
            }
            h += 1;
            k += 1;
        }
        let mut j = 0;
        for i in 0..n_rows {
            while j < n_cols && self.a[i][j].abs() < f32::EPSILON {
                j += 1;
            }
            if j == n_cols {
                break;
            }
            self.free[j] = false;
        }
    }

    fn solve(&mut self) {
        let n = self.free.iter().filter(|&x| !x).count();
        let n_cols = self.a[0].len();
        let mut j = n_cols;
        for i in (1..n + 1).rev() {
            j -= 1;
            while self.free[j] && j > 0 {
                j -= 1;
            }
            self.x[j] = self.b[i - 1];
            for k in j + 1..n_cols {
                if self.free[k] {
                    continue;
                }
                self.x[j] -= self.a[i - 1][k] * self.x[k];
            }
            self.x[j] /= self.a[i - 1][j];
        }
    }
}

fn solve_part1(machine: &Machine, word: u32, cost: u16, button_index: usize, min_cost: &mut u16) {
    if word == machine.diagram {
        *min_cost = cost.min(*min_cost);
        return;
    }
    if cost >= *min_cost || button_index == machine.buttons.len() {
        return;
    }
    let mask = button_as_u32(&machine.buttons[button_index], machine.n_bit);
    solve_part1(machine, word, cost, button_index + 1, min_cost);
    solve_part1(machine, word ^ mask, cost + 1, button_index + 1, min_cost);
}

fn solve_part2(machine: &Machine, sys: &mut System, button_index: usize, min_cost: &mut u32) {
    let mut col = button_index;
    let n_cols = machine.buttons.len();
    while col < n_cols && !sys.free[col] {
        col += 1;
    }
    if col == n_cols {
        sys.solve();
        if let Some(cost) = machine.check_solution(&sys.x) {
            *min_cost = cost.min(*min_cost);
        }
        return;
    }
    let b = sys.b.clone();
    for val in 0..machine.upper_bound(col) {
        sys.x[col] = val as f32;
        for (i, &bval) in b.iter().enumerate() {
            sys.b[i] = bval - sys.a[i][col] * sys.x[col];
        }
        solve_part2(machine, sys, col + 1, min_cost);
    }
}

fn main() -> Result<()> {
    let filename = env::args().nth(1).expect("Missing input file");
    let file = File::open(filename)?;
    let reader = BufReader::new(file);
    let mut part1: u16 = 0;
    let mut part2: u32 = 0;
    for line in reader.lines() {
        let line = line?;
        let machine = Machine::from_line(&line);
        let mut sol1 = u16::MAX;
        solve_part1(&machine, 0, 0, 0, &mut sol1);
        part1 += sol1;

        let mut sys = System::from_machine(&machine);
        let mut sol2 = u32::MAX;
        sys.gaussian_elimination();
        solve_part2(&machine, &mut sys, 0, &mut sol2);
        part2 += sol2;
    }
    println!("Part 1: {}", part1);
    println!("Part 1: {}", part2);
    Ok(())
}
