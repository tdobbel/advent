const std = @import("std");

pub fn displace(move: u8, x: *i32, y: *i32) void {
    switch (move) {
        '>' => x.* += 1,
        '<' => x.* -= 1,
        '^' => y.* += 1,
        'v' => y.* -= 1,
        else => unreachable,
    }
}

pub fn part1(allocator: std.mem.Allocator, moves: []u8) !usize {
    var x: i32 = 0;
    var y: i32 = 0;
    var houses = std.hash_map.AutoHashMap([2]i32, void).init(allocator);
    defer houses.deinit();
    try houses.put(.{ x, y }, {});
    for (moves) |c| {
        displace(c, &x, &y);
        try houses.put(.{ x, y }, {});
    }
    return houses.count();
}

pub fn part2(allocator: std.mem.Allocator, moves: []u8) !usize {
    var x1: i32 = 0;
    var y1: i32 = 0;
    var x2: i32 = 0;
    var y2: i32 = 0;
    var houses = std.hash_map.AutoHashMap([2]i32, void).init(allocator);
    defer houses.deinit();
    try houses.put(.{ x1, y1 }, {});
    for (moves, 0..) |c, i| {
        if (i % 2 == 0) {
            displace(c, &x1, &y1);
            try houses.put(.{ x1, y1 }, {});
        } else {
            displace(c, &x2, &y2);
            try houses.put(.{ x2, y2 }, {});
        }
    }
    return houses.count();
}

pub fn main(init: std.process.Init) !void {
    const argv = try init.minimal.args.toSlice(init.arena.allocator());
    if (argv.len < 2) {
        return error.MissingInputFile;
    }
    const file = try std.Io.Dir.cwd().openFile(init.io, argv[1], .{ .mode = .read_only });
    const stat = try file.stat(init.io);
    const moves: []u8 = try std.posix.mmap(null, @intCast(stat.size - 1), .{ .READ = true }, .{ .TYPE = .SHARED }, file.handle, 0);
    var arena = std.heap.ArenaAllocator.init(std.heap.page_allocator);
    defer arena.deinit();
    const allocator = arena.allocator();

    var buf: [2048]u8 = undefined;
    var writer = std.Io.File.stdout().writer(init.io, &buf);
    try writer.interface.print("Part 1: {}\n", .{try part1(allocator, moves)});
    try writer.interface.print("Part 2: {}\n", .{try part2(allocator, moves)});
    try writer.flush();
}
