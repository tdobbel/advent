use std::fs::File;
use std::io::{BufRead, BufReader};
use std::env;

enum Move {
    Left,
    Right,
    Up,
    Down
}

#[derive(Debug)]
enum Object {
    Wall{x: i32, y: i32},
    Box2{x1: i32, x2: i32, y: i32},
}

struct Robot {
    x: i32,
    y: i32,
}

fn get_object(objects: &Vec<Object>, x: i32, y: i32) -> Option<usize> {
    for (i,o) in objects.iter().enumerate() {
        match o {
            Object::Wall{x: x_, y: y_} => {
                if x == *x_ && y == *y_ {
                    return Some(i)
                }
            },
            Object::Box2{x1: x1_, x2: x2_, y: y_} => {
                if x == *x1_ && y == *y_ {
                    return Some(i)
                }
                if x == *x2_ && y == *y_ {
                    return Some(i)
                }
            }
        }
    }
    None
}

fn get_all_obstacles(pos: &(i32, i32), shift: &(i32, i32), objects: &Vec<Object>, indices: &mut Vec<usize>) {
    let xnew = pos.0 + shift.0;
    let ynew = pos.1 + shift.1;
    let index = match get_object(&objects, xnew, ynew) {
        Some(i) => i,
        None => return
    };
    if indices.contains(&index) {
        return
    }
    indices.push(index);
    match &objects[index] {
        Object::Wall{x:_, y:_} => return,
        Object::Box2{x1: x1_, x2:x2_, y:y_} => {
            if shift.0 == -1 {
                get_all_obstacles(&(*x1_, *y_), shift, objects, indices);
            }
            else if shift.0 == 1 {
                get_all_obstacles(&(*x2_, *y_), shift, objects, indices);
            }
            else {
                get_all_obstacles(&(*x1_, *y_), shift, objects, indices);
                get_all_obstacles(&(*x2_, *y_), shift, objects, indices);
            }
        }
    };
}

fn plot_grid(robot: &Robot, objects: &Vec<Object> , nx: i32, ny: i32) {
    for y in 0..ny {
        let mut x = 0;
        while x < nx {
            if x == robot.x && y == robot.y {
                print!("@");
            }
            else {
                let index = get_object(&objects, x, y);
                if index.is_some() {
                    let o = &objects[index.unwrap()];
                    match o {
                        Object::Wall{x:_, y:_} => print!("#"),
                        Object::Box2{x1:_, x2:_, y:_} => { print!("[]"); x += 1; },
                    }
                }
                else {
                    print!(".");
                }
            }
            x += 1;
        }
        println!();
    }
}

fn move_robot(robot: &mut Robot, dx: &Move, objects: &mut Vec<Object>) {
    let (shiftx, shifty) = match dx {
        Move::Left => (-1, 0),
        Move::Right => (1, 0),
        Move::Up => (0, -1),
        Move::Down => (0, 1),
    };
    let mut indices = Vec::<usize>::new();
    get_all_obstacles(&(robot.x, robot.y), &(shiftx, shifty), &objects, &mut indices);
    for i in indices.iter() {
        match &objects[*i] {
            Object::Wall{x:_, y:_} => return,
            Object::Box2{x1:_, x2:_, y:_} => continue
        }
    }
    robot.x += shiftx;
    robot.y += shifty;
    for i in indices.iter() {
        let o = &mut objects[*i];
        match o {
            Object::Box2{x1:x1_, x2:x2_, y:y_} => {
                *x1_ += shiftx;
                *x2_ += shiftx;
                *y_ += shifty;
            },
            _ => continue
        }
    }
}

fn main() {
    let args: Vec<String> = env::args().collect();
    assert_eq!(args.len(), 2);
    let file = File::open(&args[1]).unwrap();
    let reader = BufReader::new(file);
    let mut robot: Robot = Robot{x: 0, y: 0};
    let mut objects: Vec<Object> = Vec::new();
    let mut y = 0;
    let mut nx = 0;
    let mut lines = reader.lines();
    // Parse the grid
    while let Some(line) = lines.next() {
        let line_ = line.unwrap();
        if !line_.contains("#") {
            break
        }
        nx = 2*line_.len();
        for (x,c) in line_.chars().enumerate() {
            let pos = (2*x as i32, y);
            if c == '.' {
                continue
            }
            if c == '#' {
                objects.push(Object::Wall{x:pos.0, y:pos.1});
                objects.push(Object::Wall{x:pos.0+1, y:pos.1});
            }
            else if c == 'O' {
                objects.push(Object::Box2{x1:pos.0, x2:pos.0+1, y:pos.1});
            }
            else if c == '@' {
                robot = Robot { x: pos.0, y: pos.1 };
            }
        }
        y += 1;
    }
    println!("Initial state:");
    plot_grid(&robot, &objects, nx as i32, y);
    // Move the robot
    while let Some(line) = lines.next() {
        let line_ = line.unwrap();
        for c in line_.chars() {
            match c {
                '<' => move_robot(&mut robot, &Move::Left, &mut objects),
                '>' => move_robot(&mut robot, &Move::Right, &mut objects),
                '^' => move_robot(&mut robot, &Move::Up, &mut objects),
                'v' => move_robot(&mut robot, &Move::Down, &mut objects),
                _ => continue
            }
        }
    }
    println!("Final state:");
    plot_grid(&robot, &objects, nx as i32, y);
    let mut gps_sum = 0;
    for o in objects.iter(){
        match o {
            Object::Box2{x1:x_, x2:_, y:y_} => gps_sum += x_ + 100*y_,
            _ => continue
        }

    }
    println!("GPS sum: {}", gps_sum);
}
