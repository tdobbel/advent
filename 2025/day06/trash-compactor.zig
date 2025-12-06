const std = @import("std");

pub fn process_column_v1(nums: []const []const u8, op: u8, start: usize, end: usize) !u64 {
    var result: u64 = 0;
    if (op == '*') result = 1;
    for (nums) |line| {
        const num_slice = std.mem.trim(u8, line[start..end], " ");
        const num = try std.fmt.parseInt(u64, num_slice, 10);
        if (op == '+') {
            result += num;
        } else {
            result *= num;
        }
    }
    return result;
}

pub fn process_column_v2(nums: []const []const u8, op: u8, start: usize, end: usize) !u64 {
    var result: u64 = 0;
    if (op == '*') result = 1;
    for (start..end) |i| {
        var num: u64 = 0;
        for (nums) |line| {
            if (line[i] == ' ') {
                continue;
            }
            num = num * 10 + line[i] - '0';
        }
        if (op == '*') {
            result *= num;
        } else {
            result += num;
        }
    }
    return result;
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
    var nums = std.array_list.Managed([]u8).init(allocator);
    defer {
        for (nums.items) |line| {
            allocator.free(line);
        }
        nums.deinit();
    }

    while (try reader.interface.takeDelimiter('\n')) |line| {
        const line_dup = try allocator.dupe(u8, line);
        try nums.append(line_dup);
    }
    const lastrow = nums.pop().?;
    var ops = std.array_list.Managed(u8).init(allocator);
    defer ops.deinit();
    var col_start = std.array_list.Managed(usize).init(allocator);
    defer col_start.deinit();
    for (lastrow, 0..) |c, i| {
        if (c == ' ') {
            continue;
        }
        try ops.append(c);
        try col_start.append(i);
    }
    try col_start.append(lastrow.len + 1);
    allocator.free(lastrow);

    var part1: u64 = 0;
    var part2: u64 = 0;
    for (ops.items, 0..) |op, i| {
        part1 += try process_column_v1(nums.items, op, col_start.items[i], col_start.items[i + 1] - 1);
        part2 += try process_column_v2(nums.items, op, col_start.items[i], col_start.items[i + 1] - 1);
    }

    std.debug.print("Part1: {}\n", .{part1});
    std.debug.print("Part2: {}\n", .{part2});
}
