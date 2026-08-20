const std = @import("std");

pub fn build(b: *std.Build) void {
    const target = b.standardTargetOptions(.{});
    const optimize = b.standardOptimizeOption(.{});

    const exe = b.addExecutable(.{
        .name = "day06",
        .root_module = b.createModule(.{
            .root_source_file = b.path("src/main.zig"),
            .link_libc = true,
            .target = target,
            .optimize = optimize,
        }),
    });
    const lib_module = b.createModule(.{
        .target = target,
        .optimize = .Debug,
        .link_libc = true,
    });
    lib_module.addCSourceFiles(.{
        .files = &.{"lib/simple_regex.c"},
        .flags = &.{"-std=c99"},
    });
    const lib = b.addLibrary(.{
        .name = "simple_regex",
        .root_module = lib_module,
        .linkage = .static,
    });

    exe.root_module.linkLibrary(lib);
    exe.root_module.addIncludePath(b.path("lib"));
    exe.root_module.link_libc = true;

    const translate_c = b.addTranslateC(.{
        .root_source_file = b.path("lib/simple_regex.h"),
        .target = target,
        .optimize = .Debug,
    });
    exe.root_module.addImport("c", translate_c.createModule());

    b.installArtifact(exe);

    const run_step = b.step("run", "Run the app");
    const run_cmd = b.addRunArtifact(exe);
    run_step.dependOn(&run_cmd.step);

    run_cmd.step.dependOn(b.getInstallStep());

    if (b.args) |args| {
        run_cmd.addArgs(args);
    }

}
