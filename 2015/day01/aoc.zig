const std = @import("std");

pub fn main(init: std.process.Init) !void {
    const argv = try init.minimal.args.toSlice(init.arena.allocator());
    if (argv.len < 2) {
        return error.MissingInputFile;
    }
    const cwd = std.Io.Dir.cwd();
    const file = try cwd.openFile(init.io, argv[1], .{ .mode = .read_only });
    defer file.close(init.io);

    const stat = try file.stat(init.io);
    const content = try std.posix.mmap(null, @intCast(stat.size), .{ .READ = true }, .{ .TYPE = .SHARED }, file.handle, 0);
    var floor: i32 = 0;
    var part2: ?usize = null;
    for (content, 0..) |c, i| {
        switch (c) {
            '(' => {
                floor += 1;
            },
            ')' => {
                if (floor == 0 and part2 == null) part2 = i + 1;
                floor -= 1;
            },
            else => break,
        }
    }

    var buf: [4096]u8 = undefined;
    var writer = std.Io.File.stdout().writer(init.io, &buf);

    try writer.interface.print("Part 1: {}\n", .{floor});
    try writer.interface.print("Part 2: {}\n", .{part2.?});
    try writer.flush();
}
