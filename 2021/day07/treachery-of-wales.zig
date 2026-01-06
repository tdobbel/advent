const std = @import("std");

pub fn compute_cost(positions: []const u32, x: u32) u32 {
    var cost: u32 = 0;
    for (positions) |pos| {
        if (x >= pos) {
            cost += x - pos;
        } else {
            cost += pos - x;
        }
    }
    return cost;
}

pub fn find_minimum(positions: []const u32) u32 {
    var a: u32 = std.math.maxInt(u32);
    var b: u32 = 0;
    for (positions) |pos| {
        if (pos < a) a = pos; 
        if (pos > b) b = pos;
    }
    var x: u32 = undefined;
    var slope: i32 = undefined;
    while (b - a > 1) {
        x = (b + a) / 2;
        slope = 0;
        for (positions) |pos| {
            const ipos: i32 = @intCast(pos);
            if (ipos > x) {
                slope -= 1;
            } else if (ipos < x) {
                slope += 1;
            }
        }
        if (slope == 0) return compute_cost(positions, x);
        if (slope < 0) {
            a = x;
        } else if (slope > 0) {
            b = x;
        }

    }
    const cost_a = compute_cost(positions, a);
    const cost_b = compute_cost(positions, b);
    if (cost_a < cost_b) {
        return cost_a;
    }
    return cost_b;
}

pub fn main() !void {
    if (std.os.argv.len != 2) {
        return error.InvalidArgument;
    }
    const cwd = std.fs.cwd();
    const file_name: [:0]const u8 = std.mem.span(std.os.argv[1]);
    const file = try cwd.openFile(file_name, .{});
    defer file.close();

    var buffer: [4096]u8 = undefined;
    var reader = file.reader(&buffer);

    const allocator = std.heap.c_allocator;
    var positions = std.array_list.Managed(u32).init(allocator);
    defer positions.deinit();
    if (try reader.interface.takeDelimiter('\n')) |line| {
        var iter = std.mem.tokenizeAny(u8, line, ",");
        while (iter.next()) |elem| {
            const num = try std.fmt.parseInt(u32, elem, 10);
            try positions.append(num);
        }
    }

    const part1 = find_minimum(positions.items);
    std.debug.print("Part 1: {}\n", .{part1});
}
