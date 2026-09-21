const std = @import("std");

const PuzzleData = struct {
    allocator: std.mem.Allocator,
    count: usize,
    gains: [][]i64,
    arrangement: []usize,
    seated: []bool,

    pub fn init(allocator: std.mem.Allocator, lines: []const u8) !PuzzleData {
        var name_map = std.StringHashMap(usize).init(allocator);
        defer name_map.deinit();
        var line_iter = std.mem.splitScalar(u8, lines, '\n');
        while (line_iter.next()) |line| {
            var word_iter = std.mem.splitScalar(u8, line, ' ');
            const name = word_iter.first();
            const entry = try name_map.getOrPut(name);
            if (!entry.found_existing) {
                entry.value_ptr.* = name_map.count() - 1;
            }
        }
        const n = name_map.count();
        const gains: [][]i64 = try allocator.alloc([]i64, n);
        for (0..n) |i| {
            gains[i] = try allocator.alloc(i64, n);
        }

        line_iter.reset();
        while (line_iter.next()) |line| {
            try parse_line(line, &name_map, gains);
        }

        const arrangement = try allocator.alloc(usize, n);
        const seated = try allocator.alloc(bool, n);
        @memset(seated, false);
        return .{ .allocator = allocator, .count = n, .gains = gains, .arrangement = arrangement, .seated = seated };
    }

    pub fn total_gain(self: *const PuzzleData) i64 {
        var total: i64 = 0;
        for (0..self.count) |i| {
            const a = self.arrangement[i];
            const b = self.arrangement[(i + 1) % self.count];
            total += self.gains[a][b] + self.gains[b][a];
        }
        return total;
    }

    pub fn add_guest(self: *PuzzleData) !void {
        self.count += 1;
        const new_gains: [][]i64 = try self.allocator.alloc([]i64, self.count);
        for (0..self.count - 1) |i| {
            new_gains[i] = try self.allocator.alloc(i64, self.count);
            @memcpy(new_gains[i][0 .. self.count - 1], self.gains[i]);
            new_gains[i][self.count - 1] = 0;
            self.allocator.free(self.gains[i]);
        }
        self.allocator.free(self.gains);
        new_gains[self.count - 1] = try self.allocator.alloc(i64, self.count);
        @memset(new_gains[self.count - 1], 0);
        self.gains = new_gains;
        self.allocator.free(self.seated);
        self.seated = try self.allocator.alloc(bool, self.count);
        @memset(self.seated, false);
        self.allocator.free(self.arrangement);
        self.arrangement = try self.allocator.alloc(usize, self.count);
    }
};

pub fn parse_line(line: []const u8, name_map: *const std.StringHashMap(usize), gains: [][]i64) !void {
    var parts: [11][]const u8 = undefined;
    var word_iter = std.mem.splitScalar(u8, line[0 .. line.len - 1], ' ');
    var i: usize = 0;
    while (word_iter.next()) |word| : (i += 1) {
        parts[i] = word;
    }
    const row = name_map.get(parts[0]).?;
    const col = name_map.get(parts[10]).?;
    var gain: i64 = try std.fmt.parseInt(i64, parts[3], 10);
    if (std.mem.eql(u8, parts[2], "lose")) {
        gain *= -1;
    }
    gains[row][col] = gain;
}

pub fn solve_puzzle(best: *i64, step: usize, puzzle: *const PuzzleData) void {
    if (step == puzzle.count) {
        best.* = @max(best.*, puzzle.total_gain());
        return;
    }
    for (0..puzzle.count) |i| {
        if (puzzle.seated[i]) continue;
        puzzle.arrangement[step] = i;
        puzzle.seated[i] = true;
        solve_puzzle(best, step + 1, puzzle);
        puzzle.seated[i] = false;
    }
}

pub fn main(init: std.process.Init) !void {
    const argv = try init.minimal.args.toSlice(init.arena.allocator());
    if (argv.len < 2) {
        return error.MissingInputFile;
    }
    const file = try std.Io.Dir.cwd().openFile(init.io, argv[1], .{ .mode = .read_only });
    defer file.close(init.io);

    const stats = try file.stat(init.io);

    var arena = std.heap.ArenaAllocator.init(std.heap.c_allocator);
    const allocator = arena.allocator();
    defer arena.deinit();

    const content: []u8 = try std.posix.mmap(null, @intCast(stats.size - 1), .{ .READ = true }, .{ .TYPE = .SHARED }, file.handle, 0);
    defer std.posix.munmap(@alignCast(content));

    var puzzle = try PuzzleData.init(allocator, content);
    var buf: [64]u8 = undefined;
    var writer = std.Io.File.stdout().writer(init.io, &buf);

    var part1: i64 = 0;
    solve_puzzle(&part1, 0, &puzzle);
    try puzzle.add_guest();
    var part2: i64 = 0;
    solve_puzzle(&part2, 0, &puzzle);
    try writer.interface.print("Part 1: {}\nPart 2: {}\n", .{ part1, part2 });
    try writer.flush();
}
