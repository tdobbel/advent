const std = @import("std");

const AtomCounter = struct {
    atoms: usize,
    Rn: usize,
    Y: usize,
    Ar: usize,

    pub fn init() AtomCounter {
        return AtomCounter{ .atoms = 0, .Rn = 0, .Y = 0, .Ar = 0 };
    }
};

const ReactionMap = std.StringHashMap(std.ArrayList([]const u8));
const StringSet = std.StringHashMap(void);

pub fn count_replacements(gpa: std.mem.Allocator, reactions: *const ReactionMap, molecule: []const u8) !usize {
    var i: usize = 0;
    var possibles = StringSet.init(gpa);
    while (i < molecule.len) {
        var size: usize = 1;
        while (i + size < molecule.len and molecule[i + size] >= 'a') {
            size += 1;
        }
        const atom = molecule[i .. i + size];
        if (reactions.get(atom)) |products| {
            for (products.items) |mol| {
                const new_mol = try std.fmt.allocPrint(gpa, "{s}{s}{s}", .{ molecule[0..i], mol, molecule[i + size ..] });
                try possibles.put(new_mol, {});
            }
        }
        i += size;
    }
    return possibles.count();
}

pub fn main(init: std.process.Init) !void {
    const argv = try init.minimal.args.toSlice(init.arena.allocator());
    if (argv.len < 2) {
        return error.MissingInputFile;
    }

    var arena = std.heap.ArenaAllocator.init(std.heap.page_allocator);
    defer arena.deinit();
    const gpa = arena.allocator();

    const file = try std.Io.Dir.cwd().readFileAlloc(init.io, argv[1], gpa, .unlimited);
    var lines: std.ArrayList([]const u8) = .empty;

    var line_iter = std.mem.splitScalar(u8, file, '\n');
    while (line_iter.next()) |line| {
        if (line.len == 0) continue;
        try lines.append(gpa, line);
    }

    const n = lines.items.len;

    var reactions = ReactionMap.init(gpa);
    defer reactions.deinit();

    var splitted: [2][]const u8 = undefined;
    for (lines.items[0 .. n - 1]) |line| {
        var iter = std.mem.splitSequence(u8, line, " => ");
        var i: usize = 0;
        while (iter.next()) |part| : (i += 1) {
            splitted[i] = part;
        }
        const item = try reactions.getOrPut(splitted[0]);
        if (!item.found_existing) {
            item.value_ptr.* = .empty;
        }
        var values = item.value_ptr;
        try values.append(gpa, splitted[1]);
    }

    const molecule = lines.items[n - 1];
    const part1 = try count_replacements(gpa, &reactions, molecule);


    var buf: [256]u8 = undefined;
    var writer = std.Io.File.stdout().writer(init.io, &buf);

    try writer.interface.print("Part 1: {}\n", .{part1});

    var cntr = AtomCounter.init();

    var i: usize = 0;
    while (i < molecule.len) {
        var size: usize = 1;
        while (i + size < molecule.len and molecule[i + size] >= 'a') {
            size += 1;
        }
        const atom = molecule[i .. i + size];
        cntr.atoms += 1;
        if (std.mem.eql(u8, atom, "Rn")) {
            cntr.Rn += 1;
        } else if (std.mem.eql(u8, atom, "Ar")) {
            cntr.Ar += 1;
        } else if (std.mem.eql(u8, atom, "Y")) {
            cntr.Y += 1;
        }
        i += size;
    }

    // I cheated on reddit
    // Logic:
    // -----
    // The rules are the following:
    //
    // Let X be any atom except Rn, Ar and Y (as these 3 atoms do not react and
    // are thus never replaced)
    // For the initial creation of atom from an election
    //    e -> XX
    // Then we have the following possibilities for the replacement of X:
    //  - X -> XX
    //  - X -> X Rn X Ar
    //  - X -> X Rn X Y X Ar
    //  - X -> X Rn X Y X Y X Ar
    //
    // Rn and Ar do not react and 1 replacement generates either 0 or exactly
    // one Rn-Ar pair. They must not be counted in the number of
    // replacements/steps
    //
    // In each if the replacements involving Y, the number of Ys must be
    // removed twice from the length of the generated molecule to obtain 1 (=
    // the one atom that created them)
    const part2 = cntr.atoms - cntr.Rn - cntr.Ar - 2 * cntr.Y - 1;
    try writer.interface.print("Part 2: {}\n", .{part2});
    try writer.flush();
}
