const std = @import("std");

const total_duration: usize = 2503;

const Reindeer = struct {
    max_speed: u64,
    fly_time: u64,
    rest_time: u64,
    score: u64,

    pub fn from_description(desc: []const u8) !Reindeer {
        var parts: [15][]const u8 = undefined;
        var iter = std.mem.splitScalar(u8, desc, ' ');
        var i: usize = 0;
        while (iter.next()) |word| : (i += 1) {
            parts[i] = word;
        }
        const max_speed = try std.fmt.parseUnsigned(u64, parts[3], 10);
        const fly_time = try std.fmt.parseUnsigned(u64, parts[6], 10);
        const rest_time = try std.fmt.parseUnsigned(u64, parts[13], 10);
        return Reindeer{ .max_speed = max_speed, .fly_time = fly_time, .rest_time = rest_time, .score = 0 };
    }

    pub fn traveled_distance(self: *const Reindeer, duration: u64) u64 {
        const period = self.fly_time + self.rest_time;
        const n = duration / period;
        return (n * self.fly_time + @min(self.fly_time, duration - n * period)) * self.max_speed;
    }
};

pub fn main(init: std.process.Init) !void {
    const argv = try init.minimal.args.toSlice(init.arena.allocator());
    if (argv.len < 2) {
        return error.MissingInputFile;
    }

    const alloc = std.heap.page_allocator;
    var reindeers: std.ArrayList(Reindeer) = .empty;
    defer reindeers.deinit(alloc);

    var buf: [4096]u8 = undefined;
    const file = try std.Io.Dir.cwd().openFile(init.io, argv[1], .{ .mode = .read_only });
    var reader = file.reader(init.io, &buf);

    var part1: u64 = 0;
    while (try reader.interface.takeDelimiter('\n')) |line| {
        const r = try Reindeer.from_description(line);
        try reindeers.append(alloc, r);
        part1 = @max(part1, r.traveled_distance(total_duration));
    }

    buf = undefined;
    var writer = std.Io.File.stdout().writer(init.io, &buf);
    try writer.interface.print("Part 1: {}\n", .{part1});

    for (1..total_duration + 1) |d| {
        var max_dist: u64 = 0;
        for (reindeers.items) |r| {
            max_dist = @max(max_dist, r.traveled_distance(d));
        }
        for (0..reindeers.items.len) |i| {
            var r = &reindeers.items[i];
            if (r.traveled_distance(d) == max_dist) r.score += 1;
        }
    }

    var part2: u64 = 0;
    for (reindeers.items) |r| {
        part2 = @max(part2, r.score);
    }

    try writer.interface.print("Part 2: {}\n", .{part2});
    try writer.flush();
}
