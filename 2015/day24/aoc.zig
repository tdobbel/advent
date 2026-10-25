const std = @import("std");

const Group = struct { word: u32, total: u32, size: u32 };

pub fn compute_qe(weights: []const u32, grp: Group) u64 {
    var qe: u64 = 1;
    for (weights, 0..) |w, i| {
        const flag: u32 = @as(u32, @intCast(1)) << @as(u5, @intCast(i));
        if (grp.word & flag > 0) {
            qe *= @intCast(w);
        }
    }
    return qe;
}

pub fn group_less_than(_: void, grpA: Group, grpB: Group) bool {
    return grpA.size < grpB.size;
}

pub fn get_all_groups(
    gpa: std.mem.Allocator,
    found: *std.ArrayList(Group),
    weights: []const u32,
    target: u32,
    grp: Group,
    idx: usize,
) !void {
    if (idx == weights.len) {
        if (grp.total == target) {
            try found.append(gpa, grp);
        }
        return;
    }
    try get_all_groups(gpa, found, weights, target, grp, idx + 1);
    const new_total = grp.total + weights[idx];
    if (new_total <= target) {
        const new_word = grp.word | (@as(u32, @intCast(1)) << @as(u5, @intCast(idx)));
        const new_grp = Group{ .total = new_total, .size = grp.size + 1, .word = new_word };
        try get_all_groups(gpa, found, weights, target, new_grp, idx + 1);
    }
}

pub fn main(init: std.process.Init) !void {
    const args = try init.minimal.args.toSlice(init.arena.allocator());
    if (args.len < 2) {
        return error.MissingInputFile;
    }

    var file = try std.Io.Dir.cwd().openFile(init.io, args[1], .{ .mode = .read_only });
    defer file.close(init.io);
    var buf: [256]u8 = undefined;
    var reader = file.reader(init.io, &buf);

    var arena = std.heap.ArenaAllocator.init(std.heap.c_allocator);
    defer arena.deinit();
    const gpa = arena.allocator();

    var weights_vec: std.ArrayList(u32) = .empty;
    var wtot: u32 = 0;

    while (try reader.interface.takeDelimiter('\n')) |line| {
        const weight = try std.fmt.parseUnsigned(u32, line, 10);
        wtot += weight;
        try weights_vec.append(gpa, weight);
    }

    const weights = weights_vec.items;
    var found: std.ArrayList(Group) = .empty;
    const grp = Group{ .word = 0, .size = 0, .total = 0 };
    try get_all_groups(gpa, &found, weights, wtot / 3, grp, 0);

    const groups = found.items;
    std.mem.sort(Group, groups, {}, group_less_than);

    const min_size: u32 = groups[0].size;
    var part1: u64 = compute_qe(weights, groups[0]);
    var idx: usize = 1;
    while (groups[idx].size == min_size) : (idx += 1) {
        part1 =  @min(part1, compute_qe(weights, groups[idx]));

    }

    // const partition = try gpa.alloc(u8, weights.len);
    // @memset(partition, 0);
    // var part1: u32 = 1 << 31;
    // var min_size: u32 = @intCast(weights.len);
    // solve_part1(&part1, &min_size, partition, 0, weights, wtot / 3, 0, 0);

    std.debug.print("Part 1: {}\n", .{part1});
}
