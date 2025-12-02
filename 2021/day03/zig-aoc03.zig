const std = @import("std");

pub fn free_array(comptime T: anytype, allocator: std.mem.Allocator, array: *std.array_list.Managed([]T)) void {
    for (array.items) |row| {
        allocator.free(row);
    }
    array.deinit();
}

pub fn get_rating(report: [][]u8, index: usize, flag_oxygen: bool) !u32 {
    const n = report.len;
    if (n == 1) {
        return std.fmt.parseInt(u32, report[0], 2);
    }
    var i: usize = 0;
    var j: usize = 0;
    while (i < n) {
        while (i < n and report[i][index] == '0') {
            i += 1;
        }
        j = i + 1;
        while (j < n and report[j][index] == '1') {
            j += 1;
        }
        if (j == n) break;
        std.mem.swap([]u8, &report[i], &report[j]);
        i += 1;
    }
    const n0 = i;
    const n1 = n - i;
    if (flag_oxygen ^ (n0 > n1)) {
        return get_rating(report[0..n0], index + 1, flag_oxygen);
    }
    return get_rating(report[n0..], index + 1, flag_oxygen);
}

pub fn main() !void {
    if (std.os.argv.len != 2) {
        return error.InvalidArgument;
    }
    const cwd = std.fs.cwd();
    const file_name: [:0]const u8 = std.mem.span(std.os.argv[1]);
    const file = try cwd.openFile(file_name, .{});
    defer file.close();

    var buffer: [16]u8 = undefined;
    var reader = file.reader(&buffer);
    const allocator = std.heap.c_allocator;
    var report = std.array_list.Managed([]u8).init(allocator);
    defer free_array(u8, allocator, &report);
    var counter = std.array_list.Managed(usize).init(allocator);
    defer counter.deinit();

    while (try reader.interface.takeDelimiter('\n')) |line| {
        if (counter.items.len == 0) {
            for (0..line.len) |_| {
                try counter.append(0);
            }
        }
        for (line, 0..) |c, i| {
            counter.items[i] += c - '0';
        }
        const line_dup = try allocator.dupe(u8, line[0..line.len]);
        try report.append(line_dup);
    }
    var gamma: u32 = 0;
    var epsilon: u32 = 0;
    for (counter.items) |cnt| {
        gamma <<= 1;
        epsilon <<= 1;
        if (cnt > report.items.len / 2) {
            gamma += 1;
        } else {
            epsilon += 1;
        }
    }
    const oxygen_rating = try get_rating(report.items, 0, true);
    const co2_rating = try get_rating(report.items, 0, false);

    std.debug.print("Part 1: {}\n", .{gamma * epsilon});
    std.debug.print("Part 2: {}\n", .{oxygen_rating * co2_rating});
}
