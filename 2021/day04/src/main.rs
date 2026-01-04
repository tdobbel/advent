use anyhow::Result;
use std::env;
use std::fs::File;
use std::io::{BufRead, BufReader};

struct BingoBoard {
    board: [[u16; 5]; 5],
    marked: [[bool; 5]; 5],
}

impl BingoBoard {
    fn new(board: &[[u16; 5]; 5]) -> Self {
        BingoBoard {
            board: *board,
            marked: Default::default(),
        }
    }

    fn mark_number(&mut self, num: u16) {
        for i in 0..5 {
            for j in 0..5 {
                if self.board[i][j] == num {
                    self.marked[i][j] = true;
                }
            }
        }
    }

    fn winning_row(&self, row: usize) -> bool {
        for col in 0..5 {
            if !self.marked[row][col] {
                return false;
            }
        }
        true
    }

    fn winning_col(&self, col: usize) -> bool {
        for row in 0..5 {
            if !self.marked[row][col] {
                return false;
            }
        }
        true
    }

    fn winning_board(&self) -> bool {
        for i in 0..5 {
            if self.winning_row(i) || self.winning_col(i) {
                return true;
            }
        }
        false
    }

    fn unmarked_sum(&self) -> u16 {
        let mut sum = 0;
        for i in 0..5 {
            for j in 0..5 {
                if !self.marked[i][j] {
                    sum += self.board[i][j];
                }
            }
        }
        sum
    }
}

fn main() -> Result<()> {
    let args = env::args().nth(1).expect("Please provide an input file");
    let file = File::open(args)?;
    let reader = BufReader::new(file);
    let mut first = true;
    let mut numbers: Vec<u16> = vec![];
    let mut row: usize = 0;
    let mut bingo_boards: Vec<BingoBoard> = Vec::new();
    let mut board: [[u16; 5]; 5] = Default::default();
    for line in reader.lines() {
        let line = line?;
        if first {
            numbers = line.split(",").map(|x| x.parse().unwrap()).collect();
            first = false;
            continue;
        }
        if line.is_empty() {
            row = 0;
            continue;
        }
        let splitted = line.split_whitespace();
        for (col, elem) in splitted.enumerate() {
            board[row][col] = elem.parse().unwrap();
        }
        row += 1;
        if row == 5 {
            bingo_boards.push(BingoBoard::new(&board));
        }
    }

    let mut part1 = None;
    let mut part2 = 0;

    for num in numbers.iter() {
        let n = bingo_boards.len();
        if n == 0 {
            break;
        }
        for i in (0..n).rev() {
            bingo_boards[i].mark_number(*num);
            if bingo_boards[i].winning_board() {
                part2 = *num * bingo_boards[i].unmarked_sum();
                if part1.is_none() {
                    part1 = Some(part2);
                }
                bingo_boards.remove(i);
            }
        }
    }

    println!("Part 1: {}", part1.unwrap());
    println!("Part 2: {part2}");

    Ok(())
}
