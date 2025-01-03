use std::fs::File;
use std::io::{BufRead, BufReader};
use std::env;

#[derive(Debug)]
struct Button {
    x: i64,
    y: i64
}

fn parse_button(line: &str) -> Button {
    let mut ix = 0;
    let mut iy = 0;
    let mut isep = 0;
    for (i,c) in line.chars().enumerate() {
        if c == 'X' {
            ix = i+2;
        }
        if c == 'Y' {
            iy = i+2;
        }
        if c == ',' {
            isep = i;
        }
    }
    let x = &line[ix..isep];
    let y = &line[iy..];
    Button {
        x: x.parse::<i64>().unwrap(),
        y: y.parse::<i64>().unwrap()
    }
}

fn compute_tokens(a: &Button, b: &Button, price: &Button) -> i64 {
    let det  = a.x*b.y - a.y*b.x;
    if det == 0 {
        return 0;
    }
    let na = (b.y*price.x - b.x*price.y) / det;
    let nb = (a.x*price.y - a.y*price.x) / det;
    if na < 0 || nb < 0 {
        return 0;
    }
    if na*a.x + nb*b.x != price.x || na*a.y + nb*b.y != price.y {
        return 0;
    }
    return na*3+nb;
}

fn main() {
    let args: Vec<String> = env::args().collect();
    assert_eq!(args.len(), 2);
    let file = File::open(&args[1]).unwrap();
    let reader = BufReader::new(file);
    let mut buttons = Vec::<Button>::new();
    let mut tokens1 = 0;
    let mut tokens2 = 0;
    for (i,line) in reader.lines().enumerate() {
        let line = line.unwrap();
        if (i+1) % 4 == 0 {
            buttons = Vec::<Button>::new();
            continue
        }
        let button = parse_button(&line);
        if line.contains("Prize") {
            tokens1 += compute_tokens(&buttons[0], &buttons[1], &button);
            let price2 = Button { x: button.x + 10000000000000, y: button.y + 10000000000000 };
            tokens2 += compute_tokens(&buttons[0], &buttons[1], &price2);
        } else {
            buttons.push(button);
        }
    }
    println!("Total tokens: {}", tokens1);
    println!("Total tokens: {}", tokens2);
}
