const std = @import("std");

pub fn solve_part1(input: []const u8) !usize {
    var i: usize = 0;
    var cntr: usize = 0;
    const trimmed = input[1 .. input.len - 1];
    while (i < trimmed.len) : (cntr += 1) {
        if (trimmed[i] != '\\') {
            i += 1;
        } else {
            switch (trimmed[i + 1]) {
                '"' => i += 2,
                '\\' => i += 2,
                'x' => {
                    if (i > trimmed.len - 4) return error.CouilleDansLePotage;
                    i += 4;
                },
                else => i += 1,
            }
        }
    }
    return input.len - cntr;
}

pub fn solve_part2(input: []const u8) usize {
    var cntr = input.len + 2;
    for (input) |c| {
        if (c == '\\' or c == '"') cntr += 1;
    }
    return cntr - input.len;
}

pub fn main(init: std.process.Init) !void {
    const args = try init.minimal.args.toSlice(init.arena.allocator());
    if (args.len < 2) return error.MissingInputFile;
    const file = try std.Io.Dir.cwd().openFile(init.io, args[1], .{ .mode = .read_only });
    defer file.close(init.io);
    var buf: [1024]u8 = undefined;
    var reader = file.reader(init.io, &buf);
    var part1: usize = 0;
    var part2: usize = 0;
    while (try reader.interface.takeDelimiter('\n')) |line| {
        part1 += try solve_part1(line);
        part2 += solve_part2(line);
    }
    var writer = std.Io.File.stdout().writer(init.io, &buf);
    try writer.interface.print("Part 1: {}\n", .{part1});
    try writer.interface.print("Part 2: {}\n", .{part2});
    try writer.flush();
}
