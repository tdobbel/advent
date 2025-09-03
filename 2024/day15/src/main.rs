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

struct ObjectBase {
    object_type: ObjectType,
    x: usize,
    y: usize,
}

struct Object {
    object_type: ObjectType,
    x: usize,
    y: usize,
    width: usize,
}

impl Object {
    fn from_object_base(base: &ObjectBase, width: usize) -> Self {
        Self {
            object_type: base.object_type.clone(),
            x: base.x * width,
            y: base.y,
            width,
        }
    }
}

fn next_pos(x: usize, y: usize, direction: &Direction) -> (usize, usize) {
    match *direction {
        Direction::Left => (x - 1, y),
        Direction::Down => (x, y + 1),
        Direction::Up => (x, y - 1),
        Direction::Right => (x + 1, y),
    }
}

struct Warehouse {
    objects: Vec<Object>,
    grid: Vec<Vec<Option<usize>>>,
}

impl Warehouse {
    fn new(object_bases: &[ObjectBase], width: usize, nx: usize, ny: usize) -> Self {
        let mut grid = vec![vec![None; nx * width]; ny];
        let mut objects: Vec<Object> = Vec::with_capacity(object_bases.len());
        for (i, base) in object_bases.iter().enumerate() {
            let obj = Object::from_object_base(base, width);
            for iw in 0..obj.width {
                grid[obj.y][obj.x + iw] = Some(i);
            }
            objects.push(obj);
        }
        Warehouse { objects, grid }
    }

    fn summed_gps_score(&self) -> usize {
        self.objects
            .iter()
            .map(|o| match o.object_type {
                ObjectType::Wall => 0,
                ObjectType::Box => 100 * o.y + o.x,
            })
            .sum()
    }

    fn update_grid(&mut self, moved_indices: &[usize], direction: &Direction) {
        for i in moved_indices.iter() {
            let obj = &mut self.objects[*i];
            for iw in 0..obj.width {
                self.grid[obj.y][obj.x + iw] = None;
            }
            let (x_new, y_new) = next_pos(obj.x, obj.y, direction);
            obj.x = x_new;
            obj.y = y_new;
        }
        for i in moved_indices.iter() {
            let obj = &self.objects[*i];
            for iw in 0..obj.width {
                self.grid[obj.y][obj.x + iw] = Some(*i);
            }
        }
    }

    fn move_robot(&mut self, robot_x: &mut usize, robot_y: &mut usize, direction: &Direction) {
        let (x, y) = next_pos(*robot_x, *robot_y, direction);
        let object_index = match self.grid[y][x] {
            Some(i) => i,
            None => {
                *robot_x = x;
                *robot_y = y;
                return;
            }
        };
        let mut queue: Vec<usize> = vec![object_index];
        let mut todo: Vec<usize> = Vec::new();
        while let Some(i) = queue.pop() {
            let obj = &self.objects[i];
            match obj.object_type {
                ObjectType::Box => (),
                ObjectType::Wall => return,
            }
            if todo.contains(&i) {
                continue;
            }
            todo.push(i);
            match *direction {
                Direction::Up | Direction::Down => {
                    for iw in 0..obj.width {
                        let (x_next, y_next) = next_pos(obj.x + iw, obj.y, direction);
                        if let Some(i_next) = self.grid[y_next][x_next] {
                            queue.push(i_next);
                        }
                    }
                }
                Direction::Left => {
                    let (x_next, y_next) = next_pos(obj.x, obj.y, direction);
                    if let Some(i_next) = self.grid[y_next][x_next] {
                        queue.push(i_next);
                    }
                }
                Direction::Right => {
                    let (x_next, y_next) = next_pos(obj.x + obj.width - 1, obj.y, direction);
                    if let Some(i_next) = self.grid[y_next][x_next] {
                        queue.push(i_next);
                    }
                }
            }
        }
        self.update_grid(&todo, direction);
        *robot_x = x;
        *robot_y = y;
    }

    #[allow(dead_code)]
    fn show(&self, x_robot: usize, y_robot: usize) {
        let ny = self.grid.len();
        let nx = self.grid[0].len();
        for y in 0..ny {
            for x in 0..nx {
                if x == x_robot && y == y_robot {
                    print!("@");
                    continue;
                }
                if let Some(i) = self.grid[y][x] {
                    let obj = &self.objects[i];
                    match obj.object_type {
                        ObjectType::Wall => print!("#"),
                        ObjectType::Box => {
                            if obj.width == 1 {
                                print!("O");
                            } else if x == obj.x {
                                print!("[");
                            } else {
                                print!("]");
                            }
                        }
                    }
                } else {
                    print!(".");
                }
            }
            println!();
        }
    }
}

fn parse_direction(c: char) -> Result<Direction> {
    match c {
        '^' => Ok(Direction::Up),
        'v' => Ok(Direction::Down),
        '<' => Ok(Direction::Left),
        '>' => Ok(Direction::Right),
        _ => Err(anyhow::anyhow!("Invalid direction character: {}", c)),
    }
}

fn solve_puzzle(
    base: &[ObjectBase],
    nx: usize,
    ny: usize,
    start_x: usize,
    start_y: usize,
    width: usize,
    moves: &[Direction],
) -> usize {
    let mut x_robot = start_x * width;
    let mut y_robot = start_y;
    let mut warehouse = Warehouse::new(base, width, nx, ny);
    for direction in moves.iter() {
        warehouse.move_robot(&mut x_robot, &mut y_robot, direction);
    }
    warehouse.summed_gps_score()
}

fn main() -> Result<()> {
    let filename = args().nth(1).expect("Missing input file");
    let file = File::open(filename)?;
    let reader = BufReader::new(file);
    let mut object_positions: Vec<ObjectBase> = Vec::new();
    let mut x_start = usize::MAX;
    let mut y_start = usize::MAX;
    let mut parse_grid = true;
    let mut moves: Vec<Direction> = Vec::new();
    let mut ny = 0;
    let mut nx = 0;
    for line in reader.lines() {
        let line = line?;
        if line.is_empty() {
            parse_grid = false;
            continue;
        }
        if parse_grid {
            nx = line.len();
            for (x, c) in line.chars().enumerate() {
                match c {
                    '@' => (x_start, y_start) = (x, ny),
                    '.' => continue,
                    '#' => object_positions.push(ObjectBase {
                        object_type: ObjectType::Wall,
                        x,
                        y: ny,
                    }),
                    'O' => object_positions.push(ObjectBase {
                        object_type: ObjectType::Box,
                        x,
                        y: ny,
                    }),
                    _ => return Err(anyhow::anyhow!("Invalid character in input: {}", c)),
                }
            }
            ny += 1;
        } else {
            for c in line.chars() {
                moves.push(parse_direction(c)?);
            }
        }
    }

    let part1 = solve_puzzle(&object_positions, nx, ny, x_start, y_start, 1, &moves);
    println!("Part 1: {}", part1);
    let part2 = solve_puzzle(&object_positions, nx, ny, x_start, y_start, 2, &moves);
    println!("Part 2: {}", part2);

    Ok(())
}
