const std = @import("std");

pub fn main(init: std.process.Init) !void {
    const argv = try init.minimal.args.toSlice(init.arena.allocator());
    if (argv.len < 2) {
        return error.MissingInputFile;
    }

    var file = try std.Io.Dir.cwd().openFile(init.io, argv[1], .{ .mode = .read_only });
    defer file.close(init.io);

    var arena = std.heap.ArenaAllocator.init(std.heap.c_allocator);
    defer arena.deinit();
    const gpa = arena.allocator();

    var buf: [64]u8 = undefined;
    var xyz: [3]i32 = undefined;
    var reader = file.reader(init.io, &buf);

    var cubes: std.ArrayList([3]i32) = .empty;
    while (try reader.interface.takeDelimiter('\n')) |line| {
        var iter = std.mem.splitScalar(u8, line, ',');
        var i: usize = 0;
        while (iter.next()) |w| : (i += 1) {
            xyz[i] = try std.fmt.parseInt(i32, w, 10);
        }
        try cubes.append(gpa, xyz);
    }

    const part1 = count_exposed_faces(cubes.items);
    std.debug.print("Part 1: {}\n", .{part1});
}

pub fn manhattan_distance(xyz0: [3]i32, xyz1: [3]i32) u32 {
    var d: u32 = 0;
    for (0..3) |k| {
        d += @abs(xyz0[k] - xyz1[k]);
    }
    return d;
}

pub fn count_exposed_faces(cubes: []const [3]i32) usize {
    var total = cubes.len * 6;
    for (0..cubes.len - 1) |i| {
        const xyz0 = cubes[i];
        for (i + 1..cubes.len) |j| {
            const xyz1 = cubes[j];
            const d = manhattan_distance(xyz0, xyz1);
            if (d == 1) {
                // we know these two are neighbors
                total -= 2;
            }
        }
    }
    return total;
}
