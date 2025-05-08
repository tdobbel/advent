const std = @import("std");

const Operation = enum {
    add,
    multiply,
    concatenate,
};

pub fn parseNumbers(line: []const u8, array: []u64, size: *usize) !void {
    size.* = 0;
    var it = std.mem.tokenizeAny(u8, line, " ");
    while (it.next()) |part| {
        const num = try std.fmt.parseInt(u64, part, 10);
        if (size.* >= array.len) return error.ArrayTooSmall;
        array[size.*] = num;
        size.* += 1;
    }
}

pub fn concatenate(lhs: u64, rhs: u64) u64 {
    var result = lhs;
    var c = rhs;
    while (c > 0) {
        result *= 10;
        c /= 10;
    }
    return result + rhs;
}

pub fn applyOperation(op: Operation, lhs: u64, rhs: u64) u64 {
    return switch (op) {
        Operation.add => lhs + rhs,
        Operation.multiply => lhs * rhs,
        Operation.concatenate => concatenate(lhs, rhs),
    };
}

pub fn ispossible(result: u64, array: []const u64, operations: []const Operation, value: u64) bool {
    if (array.len == 0) return value == result;
    for (operations) |op| {
        const new_value = applyOperation(op, value, array[0]);
        if (new_value > result) continue;
        if (ispossible(result, array[1..], operations, new_value)) {
            return true;
        }
    }
    return false;
}

pub fn main() !void {
    if (std.os.argv.len != 2) {
        return error.InvalidArgument;
    }
    const cwd = std.fs.cwd();
    const file_name: [:0]const u8 = std.mem.span(std.os.argv[1]);
    const file = try cwd.openFile(file_name, .{});
    defer file.close();
    var buffer: [1024]u8 = undefined;
    var buf_reader = std.io.bufferedReader(file.reader());
    var in_stream = buf_reader.reader();
    var numbers: [64]u64 = undefined;
    var size: usize = 0;
    var part1: u64 = 0;
    var part2: u64 = 0;
    const op1 = [_]Operation{Operation.add, Operation.multiply};
    const op2 = [_]Operation{Operation.add, Operation.multiply, Operation.concatenate};
    while (try in_stream.readUntilDelimiterOrEof(&buffer, '\n')) |line| {
        const isep = std.mem.indexOf(u8, line, ":").?;
        const result = try std.fmt.parseInt(u64, line[0..isep], 10);
        try parseNumbers(line[isep + 1 ..], &numbers, &size);
        if (ispossible(result, numbers[1..size], &op1, numbers[0])) {
            part1 += result;
            part2 += result;
        }
        else if (ispossible(result, numbers[1..size], &op2, numbers[0])) {
            part2 += result;
        }
    }
    std.debug.print("Part 1: {}\n", .{part1});
    std.debug.print("Part 2: {}\n", .{part2});
}
