const std = @import("std");

const GainMap = std.AutoHashMap(u32, usize);

pub fn mix(x: u64, y: u64) u64 {
    return x ^ y;
}

fn prune(x: u64) u64 {
    return x & ((1 << 24) - 1);
}

fn next_number(x: u64) u64 {
    var res: u64 = prune(mix(x << 6, x));
    res = prune(mix(res >> 5, res));
    res = prune(mix(res << 11, res));
    return res;
}

fn ones_digit(x: u64) usize {
    const res: usize = @intCast(x % 10);
    return res;
}

fn get_diff(x: usize, y: *usize) i8 {
    const x_: i8 = @intCast(x);
    const y_: i8 = @intCast(y.*);
    y.* = x;
    return x_ - y_;
}

fn decode_sequence(seq: u32) [4]i8 {
    var res = [4]i8{ 0, 0, 0, 0 };
    var encoded = seq;
    for (0..4) |i| {
        const num: i8 = @intCast(encoded & 0x1f);
        res[3 - i] = num - 9;
        encoded = encoded >> 5;
    }
    return res;
}

pub fn main() !void {
    if (std.os.argv.len != 2) return error.InvalidArgument;

    const allocator = std.heap.c_allocator;

    const cwd = std.fs.cwd();
    const file_name: [:0]const u8 = std.mem.span(std.os.argv[1]);
    const file = try cwd.openFile(file_name, .{});
    defer file.close();

    var buffer: [64]u8 = undefined;
    var reader = file.reader(&buffer);

    var seq: u32 = 0;
    var prev: usize = undefined;
    var price: usize = undefined;
    var num: u32 = undefined;
    var part1: u64 = 0;
    var part2: usize = 0;
    var best_seq: u32 = undefined;
    var gains = GainMap.init(allocator);
    defer gains.deinit();
    while (reader.interface.takeDelimiterExclusive('\n')) |line| {
        var secret = try std.fmt.parseInt(u64, line, 10);
        prev = ones_digit(secret);
        var buyer = GainMap.init(allocator);
        defer buyer.deinit();
        seq = 0;
        for (0..2000) |i| {
            secret = next_number(secret);
            price = ones_digit(secret);
            num = @intCast(get_diff(price, &prev) + 9);
            seq = ((seq << 5) | num) & 0xfffff;
            if (i < 3) continue;
            const entry = try buyer.getOrPut(seq);
            if (!entry.found_existing) {
                entry.value_ptr.* = price;
            }
        }
        part1 += secret;
        var it = buyer.iterator();
        while (it.next()) |entry| {
            const gain_entry = try gains.getOrPut(entry.key_ptr.*);
            if (!gain_entry.found_existing) {
                gain_entry.value_ptr.* = entry.value_ptr.*;
            } else {
                gain_entry.value_ptr.* += entry.value_ptr.*;
            }
            if (gain_entry.value_ptr.* > part2) {
                part2 = gain_entry.value_ptr.*;
                best_seq = entry.key_ptr.*;
            }
        }
    } else |err| if (err != error.EndOfStream) {
        return err;
    }

    std.debug.print("Part 1: {d}\n", .{part1});
    const decoded = decode_sequence(best_seq);
    std.debug.print("Part 2: {d} ({}, {}, {}, {})\n", .{ part2, decoded[0], decoded[1], decoded[2], decoded[3] });
}
