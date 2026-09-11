const std = @import("std");

pub fn get_next(pwd: []u8) bool {
    var i: usize = pwd.len;
    while (i > 0) : (i -= 1) {
        if (pwd[i - 1] == 'z') {
            pwd[i - 1] = 'a';
            continue;
        }
        pwd[i - 1] += 1;
        return true;
    }
    return false;
}

pub fn has_forbidden_letters(pwd: []const u8) bool {
    for (pwd) |c| {
        if (c == 'i' or c == 'o' or c == 'l') return true;
    }
    return false;
}

pub fn has_increasing_triplet(pwd: []const u8) bool {
    if (pwd.len < 3) return false;
    for (0..pwd.len - 2) |i| {
        if (pwd[i + 1] == pwd[i] + 1 and pwd[i + 2] == pwd[i + 1] + 1) return true;
    }
    return false;
}

pub fn has_non_overlapping_pairs(pwd: []const u8, n_pair: usize) bool {
    if (pwd.len < 2) return false;
    for (0..pwd.len - 1) |i| {
        if (pwd[i] != pwd[i + 1]) continue;
        if (n_pair == 1) return true;
        return has_non_overlapping_pairs(pwd[i + 2 ..], n_pair - 1);
    }
    return false;
}

pub fn isvalid(pwd: []const u8) bool {
    if (has_forbidden_letters(pwd)) return false;
    if (!has_increasing_triplet(pwd)) return false;
    return has_non_overlapping_pairs(pwd, 2);
}

pub fn main(init: std.process.Init) !void {
    var buf: [64]u8 = undefined;
    const pwd: []u8 = try std.fmt.bufPrint(&buf, "{s}", .{"vzbxkghb"});
    var outbuf: [64]u8 = undefined;
    var writer = std.Io.File.stdout().writer(init.io, &outbuf);
    var cntr: usize = 2;
    while (get_next(pwd) and cntr > 0) {
        if (isvalid(pwd)) {
            try writer.interface.print("{s}\n", .{pwd});
            cntr -= 1;
        }
    }
    try writer.flush();
}
