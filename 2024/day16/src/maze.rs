use std::collections::BinaryHeap;

#[derive(Clone, PartialEq, Eq)]
pub enum Direction {
    North,
    East,
    South,
    West,
}

pub struct Path {
    score: usize,
    distance: usize,
    heading: Direction,
    positions: Vec<(usize, usize)>,
}

impl Path {
    pub fn last_position(&self) -> (usize, usize) {
        *self.positions.last().unwrap()
    }

    pub fn get_score(&self) -> usize {
        self.score
    }

    pub fn get_cost(&self) -> usize {
        self.score + self.distance
    }

    pub fn new_positions(&self, new_pos: (usize, usize)) -> Vec<(usize, usize)> {
        let mut new_positions = self.positions.clone();
        new_positions.push(new_pos);
        new_positions
    }

    pub fn get_positions(&self) -> &Vec<(usize, usize)> {
        &self.positions
    }
}

impl PartialEq for Path {
    fn eq(&self, other: &Self) -> bool {
        self.last_position() == other.last_position() && self.score == other.score
    }
}

impl Eq for Path {}

impl PartialOrd for Path {
    fn partial_cmp(&self, other: &Self) -> Option<std::cmp::Ordering> {
        Some(self.cmp(other))
    }
}

impl Ord for Path {
    fn cmp(&self, other: &Self) -> std::cmp::Ordering {
        other.get_cost().cmp(&self.get_cost())
    }
}

fn turn_right(dir: &Direction) -> Direction {
    match dir {
        Direction::North => Direction::East,
        Direction::East => Direction::South,
        Direction::South => Direction::West,
        Direction::West => Direction::North,
    }
}

fn turn_left(dir: &Direction) -> Direction {
    match dir {
        Direction::North => Direction::West,
        Direction::West => Direction::South,
        Direction::South => Direction::East,
        Direction::East => Direction::North,
    }
}

fn next_position(pos: (usize, usize), dir: &Direction) -> (usize, usize) {
    let (x, y) = pos;
    match dir {
        Direction::North => (x, y - 1),
        Direction::East => (x + 1, y),
        Direction::South => (x, y + 1),
        Direction::West => (x - 1, y),
    }
}

fn get_distance(pos_a: (usize, usize), pos_b: (usize, usize)) -> usize {
    pos_a.0.abs_diff(pos_b.0) + pos_a.1.abs_diff(pos_b.1)
}

pub fn direction_from_positions(pos_from: (usize, usize), pos_to: (usize, usize)) -> Direction {
    let (x0, y0) = pos_from;
    let (x1, y1) = pos_to;
    if x1 > x0 {
        Direction::East
    } else if x1 < x0 {
        Direction::West
    } else if y1 > y0 {
        Direction::South
    } else {
        Direction::North
    }
}

pub fn find_shortest_path(
    start_pos: (usize, usize),
    end_pos: (usize, usize),
    heading: &Direction,
    start_score: usize,
    is_wall: &[Vec<bool>],
    max_score: Option<usize>,
) -> Option<Path> {
    let mut queue: BinaryHeap<Path> = BinaryHeap::new();
    queue.push(Path {
        score: start_score,
        distance: get_distance(start_pos, end_pos),
        heading: heading.clone(),
        positions: vec![start_pos],
    });
    let mut visited: Vec<Vec<bool>> = vec![vec![false; is_wall[0].len()]; is_wall.len()];
    while !queue.is_empty() {
        let shortest = queue.pop().unwrap();
        if let Some(v_max) = max_score {
            if shortest.score > v_max {
                return None;
            }
        }
        let pos = shortest.last_position();
        if pos == end_pos {
            return Some(shortest);
        }
        let directions = [
            shortest.heading.clone(),
            turn_right(&shortest.heading),
            turn_left(&shortest.heading),
        ];
        for new_heading in directions.iter() {
            let (x, y) = next_position(pos, new_heading);
            if is_wall[y][x] || visited[y][x] {
                continue;
            }
            visited[y][x] = true;
            let mut new_score = shortest.score + 1;
            if new_heading != &shortest.heading {
                new_score += 1000;
            }
            queue.push(Path {
                score: new_score,
                distance: get_distance((x, y), end_pos),
                heading: new_heading.clone(),
                positions: shortest.new_positions((x, y)),
            });
        }
    }
    None
}
