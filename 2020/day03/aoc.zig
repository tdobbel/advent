const std = @import("std");

pub fn count_trees(grid: []const []const u8, n_right: usize, n_down: usize) u64 {
    const nx = grid[0].len;
    var x: usize = 0;
    var y: usize = 0;
    var nTree: u32 = 0;
    while (y < grid.len) {
        if (grid[y][x] == '#') {
            nTree += 1;
        }
        x = (x + n_right) % nx;
        y += n_down;
    }
    return nTree;
}

pub fn main() !void {
    if (std.os.argv.len != 2) {
        return error.InvalidArgument;
    }
    const cwd = std.fs.cwd();
    const file_name: [:0]const u8 = std.mem.span(std.os.argv[1]);
    const file = try cwd.openFile(file_name, .{});
    defer file.close();

    var buffer: [128]u8 = undefined;
    var reader = file.reader(&buffer);

    var arena: std.heap.ArenaAllocator = .init(std.heap.c_allocator);
    defer arena.deinit();
    const allocator = arena.allocator();

    var grid_array = std.array_list.Managed([]u8).init(allocator);

    while (try reader.interface.takeDelimiter('\n')) |line| {
        const line_dup = try allocator.dupe(u8, line);
        try grid_array.append(line_dup);
    }
    const grid = try grid_array.toOwnedSlice();
    const part1 = count_trees(grid, 3, 1);
    std.debug.print("Part 1: {} trees encountered\n", .{part1});

    const rights = [5]usize{ 1, 3, 5, 7, 1 };
    const downs = [5]usize{ 1, 1, 1, 1, 2 };

    var part2: u64 = 1;
    for (0..5) |i| {
        part2 *= count_trees(grid, rights[i], downs[i]);
    }
    std.debug.print("Part 2: {}\n", .{part2});
}
