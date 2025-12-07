const std = @import("std");

const Point = struct {
    x: usize,
    y: usize,
};

const Splitter = struct {
    coords: Point,
    counter: u64,
    n_children: usize,
    children: [2]?*Splitter,

    pub fn new(x: usize, y: usize) Splitter {
        return Splitter{
            .coords = Point{ .x = x, .y = y },
            .counter = 0,
            .n_children = 0,
            .children = .{ null, null },
        };
    }

    pub fn add_children(self: *Splitter, child: *Splitter) void {
        self.children[self.n_children] = child;
        self.n_children += 1;
    }
};

pub fn find_splits(allocator: std.mem.Allocator, splitter: *Splitter, manifold: []const []const u8, splitter_map: *std.AutoHashMap(Point, *Splitter), left: bool) !void {
    var x = splitter.coords.x;
    var y = splitter.coords.y;
    if (left) {
        if (x == 0) return;
        x -= 1;
    } else {
        if (x >= manifold[0].len - 1) return;
        x += 1;
    }
    while (y < manifold.len and manifold[y][x] != '^') {
        y += 1;
    }
    if (y == manifold.len) return;
    const point = Point{ .x = x, .y = y };
    const entry = try splitter_map.getOrPut(point);
    if (entry.found_existing) {
        splitter.add_children(entry.value_ptr.*);
        return;
    }
    const new_splitter = try allocator.create(Splitter);
    new_splitter.* = Splitter.new(x, y);
    splitter.add_children(new_splitter);
    entry.value_ptr.* = new_splitter;
    try find_splits(allocator, new_splitter, manifold, splitter_map, true);
    try find_splits(allocator, new_splitter, manifold, splitter_map, false);
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
    var manifold = std.array_list.Managed([]u8).init(allocator);
    defer {
        for (manifold.items) |line| {
            allocator.free(line);
        }
        manifold.deinit();
    }

    var nx: usize = undefined;
    var ny: usize = 0;
    var startX: usize = undefined;
    var startY: usize = undefined;
    while (try reader.interface.takeDelimiter('\n')) |line| {
        const line_dup = try allocator.dupe(u8, line);
        nx = line.len;
        for (line, 0..) |c, x| {
            if (c == 'S') {
                startX = x;
                startY = ny;
            }
        }
        try manifold.append(line_dup);
        ny += 1;
    }
    while (manifold.items[startY][startX] != '^') {
        startY += 1;
    }
    var root = Splitter.new(startX, startY);
    var split_map = std.AutoHashMap(Point, *Splitter).init(allocator);
    defer {
        var it = split_map.valueIterator();
        while (it.next()) |value| {
            const p = value.*.coords;
            if (p.x == startX and p.y == startY) {
                continue;
            }
            allocator.destroy(value.*);
        }
        split_map.deinit();
    }
    try split_map.put(root.coords, &root);
    try find_splits(allocator, &root, manifold.items, &split_map, true);
    try find_splits(allocator, &root, manifold.items, &split_map, false);

    var part1: u32 = 0;
    var it = split_map.keyIterator();
    while (it.next()) |_| {
        part1 += 1;
    }
    std.debug.print("Part 1: {}\n", .{part1});

    var y = ny - 1;
    while (y > 0) : (y -= 1) {
        for (0..nx) |x| {
            if (manifold.items[y][x] != '^') {
                continue;
            }
            if (split_map.get(Point{ .x = x, .y = y })) |split_ptr| {
                for (0..2) |i| {
                    if (split_ptr.children[i]) |child| {
                        split_ptr.counter += child.counter;
                    } else {
                        split_ptr.counter += 1;
                    }
                }
            }
        }
    }
    std.debug.print("Part 2: {}\n", .{root.counter});
}
