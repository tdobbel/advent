use anyhow::Result;
use std::env;
mod maze;

fn main() -> Result<()> {
    let args = env::args().nth(1).expect("No input file provided");
    let mut puzzle = maze::Maze::from_file(&args)?;

    let mut moves = vec![puzzle.first_cell()];
    let path = maze::solve_maze(&mut puzzle, &mut moves, None).unwrap();
    println!("Part 1: {}", path.get_score());

    println!("Part 2: {}", maze::find_all_best_paths(&mut puzzle, &path));

    Ok(())
}
