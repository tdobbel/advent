const std = @import("std");

pub fn received_presents(total: *u64, factors: []const [2]u64, acc: u64) void {
    if (factors.len == 0) {
        total.* += 10 * acc;
        return;
    }
    var factor: u64 = 1;
    const base: u64 = factors[0][0];
    const exp: u64 = factors[0][1];
    for (0..exp + 1) |_| {
        received_presents(total, factors[1..], acc * factor);
        factor *= base;
    }
}

pub fn received_presents_v2(total: *u64, house_number: u64, factors: []const [2]u64, acc: u64) void {
    if (factors.len == 0) {
        if (house_number / acc <= 50) {
            total.* += 11 * acc;
        }
        return;
    }
    var factor: u64 = 1;
    const base: u64 = factors[0][0];
    const exp: u64 = factors[0][1];
    for (0..exp + 1) |_| {
        received_presents_v2(total, house_number, factors[1..], acc * factor);
        factor *= base;
    }
}

pub fn prime_factorization(gpa: std.mem.Allocator, factors: *std.ArrayList([2]u64), n: u64, primes: *std.ArrayList(u64)) !void {
    factors.clearRetainingCapacity();
    var x: usize = n;
    for (primes.items) |base| {
        var exp: u64 = 0;
        while (x % base == 0) {
            exp += 1;
            x /= base;
        }
        if (exp > 0) {
            try factors.append(gpa, .{ base, exp });
        }
        if (x == 1) return;
    }
    try primes.append(gpa, n);
    try factors.append(gpa, .{ n, 1 });
}

pub fn main(init: std.process.Init) !void {
    const target: u64 = 34000000;

    var arena = std.heap.ArenaAllocator.init(std.heap.page_allocator);
    defer arena.deinit();
    const gpa = arena.allocator();

    var primes: std.ArrayList(u64) = .empty;
    try primes.append(gpa, 2);
    try primes.append(gpa, 3);
    try primes.append(gpa, 5);

    var factors: std.ArrayList([2]u64) = .empty;

    var n_house: u64 = 6;
    var part1: ?u64 = null;
    var part2: ?u64 = null;
    while (part1 == null or part2 == null) : (n_house += 1) {
        try prime_factorization(gpa, &factors, n_house, &primes);
        var n_present: u64 = undefined;
        if (part1 == null) {
            n_present = 0;
            received_presents(&n_present, factors.items, 1);
            if (n_present >= target) {
                part1 = n_house;
            }
        }
        if (part2 == null) {
            n_present = 0;
            received_presents_v2(&n_present, n_house, factors.items, 1);
            if (n_present >= target) {
                part2 = n_house;
            }
        }
    }

    var buf: [256]u8 = undefined;
    var writer = std.Io.File.stdout().writer(init.io, &buf);

    try writer.interface.print("Part 1: {}\n", .{part1.?});
    try writer.interface.print("Part 2: {}\n", .{part2.?});
    try writer.flush();
}
