use anyhow::Result;
use std::collections::BinaryHeap;
use std::fs::File;
use std::io::{BufRead, BufReader};

#[derive(Clone)]
enum Direction {
    Up,
    Down,
    Left,
    Right,
}

pub struct Maze {
    start: (usize, usize),
    end: (usize, usize),
    wall: Vec<Vec<bool>>,
    visited: Vec<Vec<bool>>,
}

#[derive(Clone)]
pub struct Path {
    score: usize,
    distance: usize,
    direction: Direction,
    positions: Vec<(usize, usize)>,
}

impl Path {
    pub fn head(&self) -> (usize, usize) {
        *self.positions.last().unwrap()
    }

    pub fn get_score(&self) -> usize {
        self.score
    }
}

impl PartialEq for Path {
    fn eq(&self, other: &Self) -> bool {
        self.head() == other.head() && self.score == other.score
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
        let self_score = self.score + self.distance;
        let other_score = other.score + other.distance;
        other_score.cmp(&self_score)
    }
}

impl Maze {
    pub fn from_file(filename: &str) -> Result<Self> {
        let file = File::open(filename)?;
        let mut ny = 0;
        let mut nx = 0;
        let reader = BufReader::new(file);
        let mut start = (usize::MAX, usize::MAX);
        let mut end = (usize::MAX, usize::MAX);
        let mut grid: Vec<Vec<bool>> = Vec::new();
        for line in reader.lines() {
            let line = line?;
            nx = line.len();
            let mut row = vec![false; nx];
            for (x, c) in line.chars().enumerate() {
                match c {
                    'S' => start = (x, ny),
                    'E' => end = (x, ny),
                    '#' => row[x] = true,
                    '.' => continue,
                    _ => panic!("Invalid character"),
                }
            }
            ny += 1;
            grid.push(row);
        }
        Ok(Maze {
            start,
            end,
            wall: grid,
            visited: vec![vec![false; nx]; ny],
        })
    }

    pub fn first_cell(&self) -> Path {
        let positions = vec![self.start];
        let distance = self.start.0.abs_diff(self.end.0) + self.start.1.abs_diff(self.end.1);
        Path {
            score: 0,
            distance,
            direction: Direction::Right,
            positions,
        }
    }

    pub fn distance_to_end(&self, pos: (usize, usize)) -> usize {
        pos.0.abs_diff(self.end.0) + pos.1.abs_diff(self.end.1)
    }

    fn shape(&self) -> (usize, usize) {
        (self.wall.len(), self.wall[0].len())
    }

    pub fn reset_visited(&mut self) {
        let (ny, nx) = self.shape();
        self.visited = vec![vec![false; nx]; ny];
        // for ele in self.visited.iter_mut() {
        //     ele.iter_mut().for_each(|x| *x = false);
        // }
    }
}

#[allow(dead_code)]
fn compute_score(maze: &Maze, path: &Path) -> usize {
    let pos = path.head();
    path.score + maze.distance_to_end(pos)
}

fn turn_right(dir: &Direction) -> Direction {
    match dir {
        Direction::Up => Direction::Right,
        Direction::Right => Direction::Down,
        Direction::Down => Direction::Left,
        Direction::Left => Direction::Up,
    }
}

fn turn_left(dir: &Direction) -> Direction {
    match dir {
        Direction::Up => Direction::Left,
        Direction::Left => Direction::Down,
        Direction::Down => Direction::Right,
        Direction::Right => Direction::Up,
    }
}

fn next_position(pos: (usize, usize), dir: &Direction) -> (usize, usize) {
    let (x, y) = pos;
    match dir {
        Direction::Up => (x, y - 1),
        Direction::Right => (x + 1, y),
        Direction::Down => (x, y + 1),
        Direction::Left => (x - 1, y),
    }
}

fn next_moves(maze: &mut Maze, path: &Path, paths: &mut BinaryHeap<Path>) {
    let directions = [
        path.direction.clone(),
        turn_right(&path.direction),
        turn_left(&path.direction),
    ];
    for (i, dir) in directions.iter().enumerate() {
        let (x, y) = next_position(path.head(), dir);
        if maze.wall[y][x] || maze.visited[y][x] {
            continue;
        }
        let mut positions = path.positions.clone();
        positions.push((x, y));
        let score = path.score + (if i == 0 { 1 } else { 1001 });
        maze.visited[y][x] = true;
        paths.push(Path {
            score,
            distance: maze.distance_to_end((x, y)),
            direction: dir.clone(),
            positions,
        });
    }
}

pub fn solve_maze(
    maze: &mut Maze,
    paths: &mut BinaryHeap<Path>,
    max_score: Option<usize>,
) -> Option<Path> {
    while !paths.is_empty() {
        let current = paths.pop().unwrap();
        if let Some(v_max) = max_score {
            if current.score > v_max {
                return None;
            }
        }
        if current.head() == maze.end {
            return Some(current);
        }
        next_moves(maze, &current, paths);
    }
    None
}

fn set_best(best_cells: &mut [Vec<bool>], path: &[(usize, usize)]) {
    for pos in path.iter() {
        best_cells[pos.1][pos.0] = true;
    }
}

pub fn find_all_best_paths(maze: &mut Maze, ref_path: &Path) -> usize {
    let mut start = maze.first_cell();
    let (ny, nx) = maze.shape();
    let ref_score = ref_path.get_score();
    let n = ref_path.positions.len();
    let mut best_cells = vec![vec![false; nx]; ny];
    set_best(&mut best_cells, &ref_path.positions);
    for pos in ref_path.positions.iter().take(n - 2).skip(1) {
        let mut paths: BinaryHeap<Path> = BinaryHeap::new();
        maze.reset_visited();
        next_moves(maze, &start, &mut paths);
        let mut next = paths.into_vec();
        let irm = next.iter().position(|p| p.head() == *pos).unwrap();
        start = next.remove(irm);
        if next.is_empty() {
            continue;
        }
        paths = next.into();
        let (y, x) = start.head();
        maze.visited[y][x] = true;
        if let Some(new_path) = solve_maze(maze, &mut paths, Some(ref_score)) {
            set_best(&mut best_cells, &new_path.positions);
        }
    }
    let mut result = 0;
    for row in best_cells.iter() {
        result += row.iter().filter(|&x| *x).count()
    }
    result
}
