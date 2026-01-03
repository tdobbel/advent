const std = @import("std");

const NotSolvedError = error{NotSolved};

const BingoBoard = struct {
    board: [5][5]u8,
    marked: [5][5]bool,

    pub fn new(board: [5][5]u8) BingoBoard {
        var marked: [5][5]bool = undefined;
        for (0..5) |i| {
            @memset(&marked[i], false);
        }
        return BingoBoard{ .board = board, .marked = marked };
    }

    pub fn display(self: *const BingoBoard) void {
        for (0..5) |i| {
            for (0..5) |j| {
                if (self.marked[i][j]) {
                    std.debug.print(" >{}< ", .{self.board[i][j]});
                } else {
                    std.debug.print(" {} ", .{self.board[i][j]});
                }
            }
            std.debug.print("\n", .{});
        }
        std.debug.print("\n", .{});
    }

    pub fn mark_board(self: *BingoBoard, number: u8) void {
        for (0..5) |i| {
            for (0..5) |j| {
                if (self.board[i][j] == number) {
                    self.marked[i][j] = true;
                }
            }
        }
    }

    pub fn winning_row(self: *const BingoBoard, i: usize) bool {
        for (0..5) |j| {
            if (!self.marked[i][j]) return false;
        }
        return true;
    }

    pub fn winning_col(self: *const BingoBoard, j: usize) bool {
        for (0..5) |i| {
            if (!self.marked[i][j]) return false;
        }
        return true;
    }

    pub fn wins(self: *const BingoBoard) bool {
        for (0..5) |i| {
            if (self.winning_row(i) or self.winning_col(i)) {
                return true;
            }
        }
        return false;
    }

    pub fn unmarked_sum(self: *const BingoBoard) u16 {
        var sum: u16 = 0;
        for (0..5) |i| {
            for (0..5) |j| {
                if (!self.marked[i][j]) {
                    sum += @intCast(self.board[i][j]);
                }
            }
        }
        return sum;
    }
};

pub fn parse_numbers(allocator: std.mem.Allocator, line: []u8) ![]u8 {
    var num_iter = std.mem.tokenizeAny(u8, line, ",");
    var numbers = std.array_list.Managed(u8).init(allocator);
    while (num_iter.next()) |val| {
        const n = try std.fmt.parseInt(u8, val, 10);
        try numbers.append(n);
    }
    return try numbers.toOwnedSlice();
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
    var bingo_boards = std.array_list.Managed(BingoBoard).init(allocator);
    defer bingo_boards.deinit();
    var numbers: []u8 = undefined;

    while (try reader.interface.takeDelimiter('\n')) |line| {
        if (line.len == 0) break;
        numbers = try parse_numbers(allocator, line);
    }
    defer allocator.free(numbers);

    var board: [5][5]u8 = undefined;
    var i: usize = 0;
    while (try reader.interface.takeDelimiter('\n')) |line| {
        if (i == 5) {
            i = 0;
            continue;
        }
        var j: usize = 0;
        var iter = std.mem.tokenizeAny(u8, line, " ");
        while (iter.next()) |elem| {
            board[i][j] = try std.fmt.parseInt(u8, elem, 10);
            j += 1;
        }
        i += 1;
        if (i == 5) {
            const bb = BingoBoard.new(board);
            try bingo_boards.append(bb);
        }
    }

    var part1: ?u16 = null;
    var part2: u16 = undefined;
    for (numbers) |number| {
        const n = bingo_boards.items.len;
        if (n == 0) {
            break;
        }
        for (0..n) |k| {
            const index = n - k - 1;
            var bb = &bingo_boards.items[index];
            bb.mark_board(number);
            if (bb.wins()) {
                part2 = @as(u16, number) * bb.unmarked_sum();
                _ = bingo_boards.orderedRemove(index);
                if (part1 == null) {
                    part1 = part2;
                }
            }
        }
    }

    std.debug.print("Part 1: {}\n", .{part1.?});
    std.debug.print("Part 2: {}\n", .{part2});
}
