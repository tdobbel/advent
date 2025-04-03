use anyhow::Result;
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

pub struct Path {
    score: usize,
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
        Path {
            score: 0,
            direction: Direction::Right,
            positions,
        }
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

fn compute_score(maze: &Maze, path: &Path) -> usize {
    let pos = path.head();
    path.score + maze.end.0.abs_diff(pos.0) + maze.end.1.abs_diff(pos.1)
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

fn next_moves(maze: &mut Maze, path: &Path, paths: &mut Vec<Path>) {
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
            direction: dir.clone(),
            positions,
        });
    }
}

pub fn solve_maze(
    maze: &mut Maze,
    paths: &mut Vec<Path>,
    max_score: Option<usize>,
) -> Option<Path> {
    while !paths.is_empty() {
        let mut best_score = usize::MAX;
        let mut ibest = 0;
        for (i, path) in paths.iter().enumerate() {
            let score = compute_score(maze, path);
            if score < best_score {
                best_score = score;
                ibest = i;
            }
        }
        if let Some(v_max) = max_score {
            if best_score > v_max {
                return None;
            }
        }
        let best_path = paths.remove(ibest);
        if best_path.head() == maze.end {
            return Some(best_path);
        }
        next_moves(maze, &best_path, paths);
    }
    None
}

pub fn find_all_best_paths(maze: &mut Maze, ref_path: &Path) -> usize {
    let mut start = maze.first_cell();
    let (ny, nx) = maze.shape();
    let ref_score = ref_path.get_score();
    let n = ref_path.positions.len();
    let mut best_cells = vec![vec![false; nx]; ny];
    ref_path
        .positions
        .iter()
        .for_each(|p| best_cells[p.1][p.0] = true);
    let mut iremove = 0;
    for pos in ref_path.positions.iter().take(n - 2).skip(1) {
        maze.reset_visited();
        let mut paths: Vec<Path> = Vec::new();
        next_moves(maze, &start, &mut paths);
        for (i, p) in paths.iter().enumerate() {
            if p.head() == *pos {
                iremove = i;
                break;
            }
        }
        start = paths.remove(iremove);
        if paths.is_empty() {
            continue;
        }
        let (y, x) = start.head();
        maze.visited[y][x] = true;
        if let Some(new_path) = solve_maze(maze, &mut paths, Some(ref_score)) {
            new_path
                .positions
                .iter()
                .for_each(|p| best_cells[p.1][p.0] = true);
        }
    }
    let mut result = 0;
    for row in best_cells.iter() {
        result += row.iter().filter(|&x| *x).count()
    }
    result
}
