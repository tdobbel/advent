const std = @import("std");

pub fn find_max(vmax: *i64, x: []i64, c: []const [5]i64, pos: usize, check_cal: bool) void {
    var nmax: i64 = 100;
    for (0..pos) |i| {
        nmax -= x[i];
    }
    if (pos == x.len - 1) {
        x[pos] = nmax;
        if (check_cal) {
            var total_cal: i64 = 0;
            for (0..x.len) |i| {
                total_cal += x[i] * c[i][4];
            }
            if (total_cal != 500) return;
        }
        var total: i64 = 1;
        for (0..4) |j| {
            var dot_product: i64 = 0;
            for (0..x.len) |i| {
                dot_product += c[i][j] * x[i];
            }
            total *= @max(dot_product, 0);
            if (total == 0) break;
        }
        vmax.* = @max(vmax.*, total);
        return;
    }
    for (0..@as(usize, @intCast(nmax + 1))) |v| {
        x[pos] = @intCast(v);
        find_max(vmax, x, c, pos + 1, check_cal);
    }
}

pub fn main(init: std.process.Init) !void {
    const args = try init.minimal.args.toSlice(init.arena.allocator());
    if (args.len < 2) {
        return error.MissingInputFile;
    }

    var file = try std.Io.Dir.cwd().openFile(init.io, args[1], .{ .mode = .read_only });
    defer file.close(init.io);

    var buf: [128]u8 = undefined;
    var reader = file.reader(init.io, &buf);

    var arena = std.heap.ArenaAllocator.init(std.heap.page_allocator);
    defer arena.deinit();
    const allocator = arena.allocator();

    var coeffs: std.ArrayList([5]i64) = .empty;
    defer coeffs.deinit(allocator);

    while (try reader.interface.takeDelimiter('\n')) |line| {
        var iter = std.mem.splitScalar(u8, line, ',');
        var row: [5]i64 = undefined;
        var i: usize = 0;
        while (iter.next()) |slice| : (i += 1) {
            var indx: usize = slice.len - 1;
            while (slice[indx] != ' ') : (indx -= 1) {}
            row[i] = try std.fmt.parseInt(i64, slice[indx + 1 ..], 10);
        }
        try coeffs.append(allocator, row);
    }

    buf = undefined;
    var writer = std.Io.File.stdout().writer(init.io, &buf);

    const x: []i64 = try allocator.alloc(i64, coeffs.items.len);
    var part1: i64 = 0;
    find_max(&part1, x, coeffs.items, 0, false);
    try writer.interface.print("Part 1: {}\n", .{part1});

    var part2: i64 = 0;
    find_max(&part2, x, coeffs.items, 0, true);
    try writer.interface.print("Part 2: {}\n", .{part2});

    try writer.flush();
}
