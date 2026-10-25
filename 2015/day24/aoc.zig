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

pub fn get_all_groups(
    gpa: std.mem.Allocator,
    found: *std.ArrayList(Group),
    min_size: *u32,
    weights: []const u32,
    target: u32,
    grp: Group,
    idx: usize,
) !void {
    if (idx == weights.len) {
        if (grp.total == target) {
            min_size.* = @min(min_size.*, grp.size);
            try found.append(gpa, grp);
        }
        return;
    }
    try get_all_groups(gpa, found, min_size, weights, target, grp, idx + 1);
    const new_total = grp.total + weights[idx];
    if (new_total <= target and grp.size < min_size.*) {
        const new_word = grp.word | (@as(u32, @intCast(1)) << @as(u5, @intCast(idx)));
        const new_grp = Group{ .total = new_total, .size = grp.size + 1, .word = new_word };
        try get_all_groups(gpa, found, min_size, weights, target, new_grp, idx + 1);
    }
}

pub fn find_best_arrangement(gpa: std.mem.Allocator, weights: []const u32, target: u32) !u64 {
    var found: std.ArrayList(Group) = .empty;
    defer found.deinit(gpa);
    const empty_grp = Group{ .word = 0, .size = 0, .total = 0 };
    var min_size: u32 = @intCast(weights.len);
    try get_all_groups(gpa, &found, &min_size, weights, target, empty_grp, 0);

    const groups = found.items;

    var min_qe: u64 = compute_qe(weights, groups[0]);
    for (groups) |grp| {
        if (grp.size > min_size) continue;
        min_qe = @min(min_qe, compute_qe(weights, grp));
    }
    return min_qe;
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

    var writer = std.Io.File.stdout().writer(init.io, &buf);

    const weights = weights_vec.items;

    const part1 = try find_best_arrangement(gpa, weights, wtot / 3);
    try writer.interface.print("Part 1: {}\n", .{part1});

    const part2 = try find_best_arrangement(gpa, weights, wtot / 4);
    try writer.interface.print("Part 2: {}\n", .{part2});

    try writer.interface.flush();
}
