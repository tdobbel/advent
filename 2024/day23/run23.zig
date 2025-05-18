const std = @import("std");

const allocator = std.heap.c_allocator;
const AdjacencyList = std.AutoHashMap(usize, std.ArrayList(usize));
const TriangleSet = std.AutoHashMap([3]usize, void);

const Network = struct {
    n_node: usize,
    nodes: [][2]u8,
    adjacency: AdjacencyList,

    pub fn free(self: *Network) void {
        allocator.free(self.nodes);
        var it = self.adjacency.iterator();
        while (it.next()) |entry| {
            entry.value_ptr.deinit();
        }
        self.adjacency.deinit();
    }
};

pub fn add_to_key(adj: *AdjacencyList, key: usize, value: usize) !void {
    var entry = try adj.getOrPut(key);
    if (!entry.found_existing) {
        entry.value_ptr.* = std.ArrayList(usize).init(allocator);
    }
    try entry.value_ptr.append(value);
}

pub fn create_network(node_map: *std.AutoHashMap([2]u8, usize), adj: AdjacencyList) !Network {
    defer node_map.deinit();
    const n = node_map.count();
    var nodes = try allocator.alloc([2]u8, n);
    var it = node_map.iterator();
    while (it.next()) |entry| {
        nodes[entry.value_ptr.*] = entry.key_ptr.*;
    }
    return Network{
        .n_node = n,
        .nodes = nodes,
        .adjacency = adj,
    };
}

pub fn contains(comptime T: anytype, arr: []T, value: T) bool {
    for (arr) |item| {
        if (item == value) return true;
    }
    return false;
}

pub fn count_triangles_node(network: *Network, depth: usize, nodes: [3]usize, triangles: *TriangleSet) !void {
    const node1 = nodes[3-depth];
    const neighbors = network.adjacency.get(node1).?;
    if (depth == 1) {
        if (contains(usize, neighbors.items, nodes[0])) {
            var nodes_ = [_]usize{ nodes[0], nodes[1], nodes[2] };
            std.mem.sort(usize, &nodes_, {}, comptime std.sort.asc(usize));
            try triangles.put(nodes_, {});
        }
        return;
    }
    for (neighbors.items) |node2| {
        var nodes_ = nodes;
        nodes_[3-depth+1] = node2;
        try count_triangles_node(network, depth - 1, nodes_, triangles);
    }
}

pub fn find_max_clique(network: *Network) !void {
    const n = network.n_node;
    var node_ids = try allocator.alloc(usize, n);
    defer allocator.free(node_ids);
    for (0..network.n_node) |i| {
        node_ids[i] = i;
    }
    var largest: usize = 0;
    var best: []usize = undefined;
    var start: usize = 0; 
    var stop: usize = 1;
    while (stop < n) {
        var added: bool = false;
        for (stop..n) |i| {
            const node1 = node_ids[i];
            var add: bool = true;
            for (start..stop) |j| {
                const node2 = node_ids[j];
                const neighbors = network.adjacency.get(node2).?;
                if (!contains(usize, neighbors.items, node1)) {
                    add = false;
                    break;
                }
            }
            if (add) {
                std.mem.swap(usize, &node_ids[stop], &node_ids[i]);
                stop += 1;
                added = true;
                break;
            }
        }
        if (!added or stop == n) {
            if (stop - start > largest) {
                best = node_ids[start..stop];
                largest = stop - start;
            }
            start = stop;
            stop = start + 1;
        }
    }
    var names = try allocator.alloc([2]u8, largest);
    defer allocator.free(names);
    for (best, 0..) |node, i| {
        names[i] = network.nodes[node];
    }
    std.mem.sort([2]u8, names, {}, struct {
        pub fn lessThan(_: void, a: [2]u8, b: [2]u8) bool {
            return std.mem.order(u8, &a, &b) == .lt;
        }
    }.lessThan);
    std.debug.print("Part 2: {} ", .{largest});
    for (names, 0..) |name, i| {
        std.debug.print("{s}", .{name});
        if ( i != largest - 1) {
            std.debug.print(",", .{});
        }
    }
    std.debug.print("\n", .{});
}

pub fn main() !void {
    if (std.os.argv.len != 2) {
        return error.InvalidArgument;
    }
    const cwd = std.fs.cwd();
    const file_name: [:0]const u8 = std.mem.span(std.os.argv[1]);
    const file = try cwd.openFile(file_name, .{});
    defer file.close();

    var buffer: [5096]u8 = undefined;
    var buf_reader = std.io.bufferedReader(file.reader());
    var in_stream = buf_reader.reader();

    var node_map = std.AutoHashMap([2]u8, usize).init(allocator);
    var adj = AdjacencyList.init(allocator);
    var cntr: usize = 0;
    while (try in_stream.readUntilDelimiterOrEof(&buffer, '\n')) |line| {
        const node1 = [_]u8{ line[0], line[1] };
        const node2 = [_]u8{ line[3], line[4] };
        if (!node_map.contains(node1)) {
            try node_map.put(node1, cntr);
            cntr += 1;
        }
        if (!node_map.contains(node2)) {
            try node_map.put(node2, cntr);
            cntr += 1;
        }
        const i = node_map.get(node1).?;
        const j = node_map.get(node2).?;
        try add_to_key(&adj, i, j);
        try add_to_key(&adj, j, i);
    }
    var network = try create_network(&node_map, adj);
    defer network.free();

    var triangles = TriangleSet.init(allocator);
    defer triangles.deinit();
    for (0..network.n_node) |i| {
        const nodes = [_]usize{ i, 0, 0 };
        try count_triangles_node(&network, 3, nodes, &triangles);
    }
    var it = triangles.iterator();
    var part1: usize = 0;
    while (it.next()) |entry| {
        for (entry.key_ptr.*) |i| {
            if (network.nodes[i][0] == 't') {
                part1 += 1;
                break;
            }
        }
    }
    std.debug.print("Part 1: {}\n", .{part1});
    try find_max_clique(&network);
}
