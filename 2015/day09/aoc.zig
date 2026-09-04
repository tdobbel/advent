const std = @import("std");

const Edge = struct {
    a: usize,
    b: usize,
    weight: u32,
};

const NameMap = struct {
    allocator: std.mem.Allocator,
    hm: std.StringHashMap(usize),

    pub fn init(allocator: std.mem.Allocator) NameMap {
        return NameMap{ .allocator = allocator, .hm = std.StringHashMap(usize).init(allocator) };
    }

    pub fn size(self: *const NameMap) usize {
        return self.hm.count();
    }

    pub fn get(self: *NameMap, name: []const u8) !usize {
        const val = self.hm.get(name);
        if (val != null) return val.?;
        const key = try self.allocator.dupe(u8, name);
        const n = self.hm.count();
        try self.hm.put(key, n);
        return n;
    }
};

pub fn find_shortest(shortest: *u32, n_edges: usize, wtot: u32, edges: []const Edge, counter: []u8) void {
    if (n_edges == counter.len - 1) {
        shortest.* = @min(shortest.*, wtot);
        return;
    }
    if (edges.len == 0) return;
    const edge = edges[0];
    if (counter[edge.a] < 2 and counter[edge.b] < 2) {
        counter[edge.a] += 1;
        counter[edge.b] += 1;
        find_shortest(shortest, n_edges + 1, wtot + edge.weight, edges[1..], counter);
        counter[edge.a] -= 1;
        counter[edge.b] -= 1;
    }
    find_shortest(shortest, n_edges, wtot, edges[1..], counter);
}

pub fn main(init: std.process.Init) !void {
    const args = try init.minimal.args.toSlice(init.arena.allocator());
    if (args.len < 2) return error.MissingInputFile;

    const file = try std.Io.Dir.cwd().openFile(init.io, args[1], .{ .mode = .read_only });
    defer file.close(init.io);
    var arena = std.heap.ArenaAllocator.init(std.heap.page_allocator);
    defer arena.deinit();
    const allocator = arena.allocator();

    var buf: [1024]u8 = undefined;
    var reader = file.reader(init.io, &buf);

    var name_map = NameMap.init(allocator);
    var edges: std.ArrayList(Edge) = .empty;

    var parts: [5][]const u8 = undefined;
    var vmax: u32 = 0;
    while (try reader.interface.takeDelimiter('\n')) |line| {
        var iter = std.mem.splitScalar(u8, line, ' ');
        var ip: usize = 0;
        while (iter.next()) |p| : (ip += 1) {
            parts[ip] = p;
        }
        const a = try name_map.get(parts[0]);
        const b = try name_map.get(parts[2]);
        const weight = try std.fmt.parseUnsigned(u32, parts[4], 10);
        vmax += weight;
        try edges.append(allocator, Edge{ .a = a, .b = b, .weight = weight });
    }

    const counter = try allocator.alloc(u8, name_map.size());
    @memset(counter, 0);
    var part1 = vmax;
    find_shortest(&part1, 0, 0, edges.items, counter);

    var writer = std.Io.File.stdout().writer(init.io, &buf);
    try writer.interface.print("Part 1: {}\n", .{part1});
    try writer.flush();

}
