const std = @import("std");

pub fn count_letters(word: []const u8) [26]u5 {
    var counter: [26]u5 = undefined;
    @memset(&counter, 0);
    for (word) |letter| {
        const lc = std.ascii.toLower(letter);
        if (lc < 'a' or lc > 'z') continue;
        counter[@intCast(lc - 'a')] += 1;
    }
    return counter;
}

pub fn possible_word(ref_counter: [26]u5, word: []u8) bool {
    const letter_count = count_letters(word);
    for (letter_count, 0..) |lc, i| {
        if (lc > ref_counter[i]) return false;
    }
    return true;
}

pub fn main(init: std.process.Init) !void {
    const cwd = std.Io.Dir.cwd();
    const file = try cwd.openFile(init.io, "/usr/share/dict/words", .{ .mode = .read_only });
    defer file.close(init.io);

    var buffer: [128]u8 = undefined;
    var reader = file.reader(init.io, &buffer);

    const puzzle = "cryestmotolsns";
    const refcount = count_letters(puzzle[0..]);

    const allocator = std.heap.c_allocator;
    var candidates = std.array_list.Managed([]u8).init(allocator);
    defer {
        for (candidates.items) |word| {
            allocator.free(word);
        }
        candidates.deinit();
    }

    while (try reader.interface.takeDelimiter('\n')) |line| {
        if (possible_word(refcount, line)) {
            const word = try allocator.dupe(u8, line);
            try candidates.append(word);
        }
    }

    const stdout = std.Io.File.stdout();
    var buf: [4096]u8 = undefined;
    var writer = stdout.writer(init.io, &buf);

    const nword = candidates.items.len;
    var new_counter: [26]u5 = undefined;
    for (0..nword - 1) |i| {
        @memcpy(&new_counter, &refcount);
        const word1 = candidates.items[i];
        const cntr1 = count_letters(word1);
        for (cntr1, 0..) |n, k| {
            new_counter[k] -= n;
        }
        for (i + 1..nword) |j| {
            const word2 = candidates.items[j];
            const cntr2 = count_letters(word2);
            if (std.mem.eql(u5, &new_counter, &cntr2)) {
                try writer.interface.print("{s} {s}\n", .{ word1, word2 });
            }
        }
        try writer.interface.flush();
    }
}
