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

pub fn main() !void {
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
    var n_present: u64 = 120;
    while (n_present < target) : (n_house += 1) {
        try prime_factorization(gpa, &factors, n_house, &primes);
        n_present = 0;
        received_presents(&n_present, factors.items, 1);
    }

    std.debug.print("Part 1: {}\n", .{n_house - 1});
}
