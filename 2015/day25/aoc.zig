const std = @import("std");

pub fn get_code(row: usize, col: usize) u64 {
    const diag = row + col;

    var n: u64 = 0;
    for (0..diag) |i| {
        n += i + 1;
    }

    n += col;
    var code: u64 = 20151125;
    for (0..n) |_| {
        code = (code * 252533) % 33554393;
    }
    return code;
}

pub fn main(init: std.process.Init) !void {
    const row: usize = 2978 - 1;
    const col: usize = 3083 - 1;
    const part1 = get_code(row, col);

    var buf: [256]u8 = undefined;
    var writer = std.Io.File.stdout().writer(init.io, &buf);
    try writer.interface.print("Part1: {}\n", .{part1});
    try writer.interface.flush();
}
