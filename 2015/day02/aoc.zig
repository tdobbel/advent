const std = @import("std");

pub fn main(init: std.process.Init) !void {
    const argv = try init.minimal.args.toSlice(init.arena.allocator());
    if (argv.len < 2) {
        return error.MissingInputFile;
    }

    const cwd = std.Io.Dir.cwd();
    const file = try cwd.openFile(init.io, argv[1], .{ .mode = .read_only });
    defer file.close(init.io);
    var buffer: [4096]u8 = undefined;
    var reader = file.reader(init.io, &buffer);

    var dims: [3]u32 = undefined;
    var part1: u32 = 0;
    var part2: u32 = 0;
    while (try reader.interface.takeDelimiter('\n')) |line| {
        var i: usize = 0;
        var it = std.mem.splitScalar(u8, line, 'x');
        while (it.next()) |slice| : (i += 1) {
            dims[i] = try std.fmt.parseUnsigned(u32, slice, 10);
        }
        std.mem.sort(u32, &dims, {}, std.sort.asc(u32));
        part2 += 2 * (dims[0] + dims[1]) + dims[0] * dims[1] * dims[2];
        const area1 = dims[0] * dims[1];
        const area2 = dims[1] * dims[2];
        const area3 = dims[2] * dims[0];
        part1 += 3 * area1 + 2 * area2 + 2 * area3;
    }

    var buf: [4096]u8 = undefined;
    var writer = std.Io.File.stdout().writer(init.io, &buf);
    try writer.interface.print("Part 1: {}\n", .{part1});
    try writer.interface.print("Part 2: {}\n", .{part2});
    try writer.flush();
}
