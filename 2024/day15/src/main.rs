use anyhow::Result;
use std::env::args;
use std::fs::File;
use std::io::{BufRead, BufReader};

#[derive(PartialEq)]
enum Direction {
    Up,
    Down,
    Left,
    Right,
}

#[derive(Clone)]
enum ObjectType {
    Wall,
    Box,
}

struct Object {
    object_type: ObjectType,
    x: usize,
    y: usize,
    width: usize,
}

impl Object {
    pub fn double_width(&self) -> Object {
        Object {
            object_type: self.object_type.clone(),
            x: self.x * 2,
            y: self.y,
            width: self.width * 2,
        }
    }
}

fn parse_direction(c: char) -> Direction {
    match c {
        '^' => Direction::Up,
        'v' => Direction::Down,
        '<' => Direction::Left,
        '>' => Direction::Right,
        _ => panic!("Unexpected character: '{}'", c),
    }
}

fn create_grid(objects: &[Object], nx: usize, ny: usize) -> Vec<Vec<Option<usize>>> {
    let mut grid = vec![vec![None; nx]; ny];
    for (i, o) in objects.iter().enumerate() {
        for j in 0..o.width {
            grid[o.y][o.x + j] = Some(i);
        }
    }
    grid
}

fn next_pos(x: usize, y: usize, direction: &Direction) -> (usize, usize) {
    match *direction {
        Direction::Left => (x - 1, y),
        Direction::Down => (x, y + 1),
        Direction::Up => (x, y - 1),
        Direction::Right => (x + 1, y),
    }
}

fn find_moved_objects(
    grid: &[Vec<Option<usize>>],
    objects: &[Object],
    pos: (usize, usize),
    direction: &Direction,
    todo: &mut Vec<usize>,
    blocked: &mut bool,
) {
    let (x, y) = next_pos(pos.0, pos.1, direction);
    let iobj = match grid[y][x] {
        Some(i) => i,
        None => return,
    };
    let o = &objects[iobj];
    match o.object_type {
        ObjectType::Wall => {
            *blocked = true;
            return;
        }
        ObjectType::Box => (),
    }
    if todo.contains(&iobj) {
        return;
    }
    todo.push(iobj);
    if direction == &Direction::Left || direction == &Direction::Right {
        let pos = if o.width == 1 {
            (x, y)
        } else {
            next_pos(x, y, direction)
        };
        find_moved_objects(grid, objects, pos, direction, todo, blocked);
    } else {
        for j in 0..o.width {
            find_moved_objects(grid, objects, (o.x + j, o.y), direction, todo, blocked);
        }
    }
}

fn move_robot(
    grid: &mut [Vec<Option<usize>>],
    objects: &mut [Object],
    robot: &mut (usize, usize),
    direction: &Direction,
) {
    let mut todo: Vec<usize> = Vec::new();
    let mut blocked = false;
    find_moved_objects(grid, objects, *robot, direction, &mut todo, &mut blocked);
    if blocked {
        return;
    }
    for i in todo.iter() {
        let obj = &objects[*i];
        for j in 0..obj.width {
            grid[obj.y][obj.x + j] = None;
        }
    }
    for i in todo.iter() {
        let obj = &mut objects[*i];
        (obj.x, obj.y) = next_pos(obj.x, obj.y, direction);
        for j in 0..obj.width {
            grid[obj.y][obj.x + j] = Some(*i);
        }
    }
    (robot.0, robot.1) = next_pos(robot.0, robot.1, direction);
}

fn gps_score(objects: &[Object]) -> usize {
    objects
        .iter()
        .map(|o| match o.object_type {
            ObjectType::Wall => 0,
            ObjectType::Box => 100 * o.y + o.x,
        })
        .sum()
}

fn main() -> Result<()> {
    let filename = args().nth(1).expect("Missing input file");
    let file = File::open(filename)?;
    let reader = BufReader::new(file);
    let mut lines = reader.lines();
    let mut objects: Vec<Object> = Vec::new();
    let mut x_start = usize::MAX;
    let mut y_start = usize::MAX;
    let mut ny = 0;
    let mut nx = 0;
    while let Some(line) = lines.next() {
        let line = line?;
        if line.is_empty() {
            break;
        }
        nx = line.len();
        for (x, c) in line.chars().enumerate() {
            match c {
                '@' => (x_start, y_start) = (x, ny),
                '.' => continue,
                '#' => objects.push(Object {
                    object_type: ObjectType::Wall,
                    x,
                    y: ny,
                    width: 1,
                }),
                'O' => objects.push(Object {
                    object_type: ObjectType::Box,
                    x,
                    y: ny,
                    width: 1,
                }),
                _ => panic!("Unexpected character '{}'", c),
            }
        }
        ny += 1;
    }

    let mut robot = (x_start, y_start);
    let mut grid = create_grid(&objects, nx, ny);
    let mut robot2 = (x_start * 2, y_start);
    let mut objects2: Vec<Object> = objects.iter().map(|o| o.double_width()).collect();
    let mut grid2 = create_grid(&objects2, nx * 2, ny);

    while let Some(line) = lines.next() {
        let line = line?;
        for c in line.chars() {
            let dir = parse_direction(c);
            move_robot(&mut grid, &mut objects, &mut robot, &dir);
            move_robot(&mut grid2, &mut objects2, &mut robot2, &dir);
        }
    }

    println!("Part 1: {}", gps_score(&objects));
    println!("Part 2: {}", gps_score(&objects2));

    Ok(())
}
