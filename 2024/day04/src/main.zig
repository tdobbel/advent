const std = @import("std");

pub fn isXMAS(word: [4]u8) bool {
    return std.mem.eql(u8, &word, "XMAS") or std.mem.eql(u8, &word, "SAMX");
}

pub fn isMAS(word: [3]u8) bool {
    return std.mem.eql(u8, &word, "MAS") or std.mem.eql(u8, &word, "SAM");
}

pub fn countXMAS(grid: std.ArrayList([]u8), x: usize, y: usize, count: *u32) void {
    var word: [4]u8 = undefined;
    const ny = grid.items.len;
    const nx = grid.items[y].len;
    if (y <= ny - 4) {
        for (0..4) |k| {
            word[k] = grid.items[y + k][x];
        }
        if (isXMAS(word)) count.* += 1;
    }
    if (x <= nx - 4) {
        for (0..4) |k| {
            word[k] = grid.items[y][x + k];
        }
        if (isXMAS(word)) count.* += 1;
    }
    if (x >= 3 and y <= ny - 4) {
        for (0..4) |k| {
            word[k] = grid.items[y + k][x - k];
        }
        if (isXMAS(word)) count.* += 1;
    }
    if (x <= nx - 4 and y <= ny - 4) {
        for (0..4) |k| {
            word[k] = grid.items[y + k][x + k];
        }
        if (isXMAS(word)) count.* += 1;
    }
}

pub fn isCrossMax(grid: std.ArrayList([]u8), x: usize, y: usize) bool {
    if (x > grid.items[y].len - 3 or y > grid.items.len - 3) {
        return false;
    }
    var diag1: [3]u8 = undefined;
    var diag2: [3]u8 = undefined;
    for (0..3) |k| {
        diag1[k] = grid.items[y + k][x + k];
        diag2[k] = grid.items[y + k][x + 2 - k];
    }
    return isMAS(diag1) and isMAS(diag2);
}

pub fn main() !void {
    if (std.os.argv.len != 2) {
        return error.InvalidArgument;
    }
    const cwd = std.fs.cwd();
    const file_name: [:0]const u8 = std.mem.span(std.os.argv[1]);
    const file = try cwd.openFile(file_name, .{});
    defer file.close();
    var buffer: [1024]u8 = undefined;
    var buf_reader = std.io.bufferedReader(file.reader());
    var in_stream = buf_reader.reader();
    const allocator = std.heap.page_allocator;
    var grid = std.ArrayList([]u8).init(allocator);
    defer grid.deinit();
    while (try in_stream.readUntilDelimiterOrEof(&buffer, '\n')) |line| {
        const line_copy = try allocator.dupe(u8, line);
        errdefer allocator.free(line_copy);
        try grid.append(line_copy);
    }
    var part1: u32 = 0;
    var part2: u32 = 0;
    for (0..grid.items.len) |y| {
        for (0..grid.items[y].len) |x| {
            countXMAS(grid, x, y, &part1);
            if (isCrossMax(grid, x, y)) part2 += 1;
        }
    }
    std.debug.print("Part 1: {}\n", .{part1});
    std.debug.print("Part 2: {}\n", .{part2});
}
