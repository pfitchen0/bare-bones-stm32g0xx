_ARCH_FLAGS = ["-mcpu=cortex-m0plus", "-mthumb"]

_C_FLAGS = ["-Wall", "-Wextra", "-g", "-O0", "-I."]

_LD_FLAGS = [
    "-nostartfiles",
    "-nostdlib",
    "--specs=nano.specs",
    "--specs=nosys.specs",
    "-lc",
    "-lgcc",
    "-Wl,--gc-sections",
    "-L.",
]

Stm32g0xxLibraryInfo = provider(
    fields=[
        "hdrs",
        "archives",
    ]
)


def _stm32g0xx_impl(ctx):
    # Gather transitive files from dependencies.
    transitive_hdrs = []
    transitive_archives = []
    for dep in ctx.attr.deps:
        transitive_hdrs.append(dep[Stm32g0xxLibraryInfo].hdrs)
        transitive_archives.append(dep[Stm32g0xxLibraryInfo].archives)

    # Create new hdrs depset.
    hdrs_depset = depset(direct=ctx.files.hdrs, transitive=transitive_hdrs)

    # TODO: Consider adding attributes for objs and archives in case
    # precompiled libraries need to be compiled in.
    objs = []
    archives = []

    # Now compile the srcs to objs.
    for src in ctx.files.srcs:
        obj = ctx.actions.declare_file("{}.o".format(src.basename))
        objs.append(obj)
        if src.extension == "s":
            cmd = "arm-none-eabi-gcc {flags} -c {src} -o {obj}".format(
                flags=" ".join(_ARCH_FLAGS),
                src=src.path,
                obj=obj.path,
            )
            ctx.actions.run_shell(
                command=cmd,
                inputs=depset(direct=[src], transitive=[hdrs_depset]),
                outputs=[obj],
                use_default_shell_env=True,
            )
        elif src.extension == "c":
            cmd = "arm-none-eabi-gcc {flags} -c {src} -o {obj}".format(
                flags=" ".join(_ARCH_FLAGS + _C_FLAGS),
                src=src.path,
                obj=obj.path,
            )
            ctx.actions.run_shell(
                command=cmd,
                inputs=depset(direct=[src], transitive=[hdrs_depset]),
                outputs=[obj],
                use_default_shell_env=True,
            )

    # Combine objs into an archive.
    archive = ctx.actions.declare_file("{}.a".format(ctx.label.name))
    cmd = "arm-none-eabi-ar -rc {archive} {objs}".format(
        archive=archive.path, objs=" ".join([obj.path for obj in objs])
    )
    ctx.actions.run_shell(
        command=cmd,
        inputs=objs,
        outputs=[archive],
        use_default_shell_env=True,
    )

    # Create new archives depset. Use topological order so the direct archive is always first when
    # converting this depset to a list. The ordering of archives matters when linking!
    archives_depset = depset(
        direct=[archive], order="topological", transitive=transitive_archives
    )

    # If no ldscript is provided, return default and library info structs.
    if not ctx.file.ldscript:
        return [
            DefaultInfo(files=depset(direct=[archive])),
            Stm32g0xxLibraryInfo(
                hdrs=hdrs_depset,
                archives=archives_depset,
            ),
        ]

    # Assuming a ldscript is provided at this point. Link archives together.
    elf = ctx.actions.declare_file("{}.elf".format(ctx.label.name))
    cmd = "arm-none-eabi-gcc -T {ldscript} {flags} {archives} -o {elf}".format(
        flags=" ".join(_LD_FLAGS),
        ldscript=ctx.file.ldscript.path,
        archives=" ".join([archive.path for archive in archives_depset.to_list()]),
        elf=elf.path,
    )
    ctx.actions.run_shell(
        command=cmd,
        inputs=depset(direct=[ctx.file.ldscript], transitive=[archives_depset]),
        outputs=[elf],
        use_default_shell_env=True,
    )

    # Finally, objcopy the elf to a bin and return it.
    bin = ctx.actions.declare_file("{}.bin".format(ctx.label.name))
    cmd = "arm-none-eabi-objcopy -O binary {elf} {bin}".format(
        elf=elf.path, bin=bin.path
    )
    ctx.actions.run_shell(
        command=cmd,
        inputs=[elf],
        outputs=[bin],
        use_default_shell_env=True,
    )
    return [DefaultInfo(files=depset(direct=[bin]))]


_stm32g0xx_rule = rule(
    implementation=_stm32g0xx_impl,
    attrs={
        "srcs": attr.label_list(allow_files=[".c", ".s"]),
        "hdrs": attr.label_list(allow_files=[".h"]),
        "deps": attr.label_list(providers=[Stm32g0xxLibraryInfo]),
        "ldscript": attr.label(allow_single_file=[".ld"]),
    },
)


def stm32g0xx_library(
    name,
    srcs=[],
    hdrs=[],
    deps=[],
):
    _stm32g0xx_rule(
        name=name,
        srcs=srcs,
        hdrs=hdrs,
        deps=deps,
    )


def stm32g0xx_binary(
    name,
    ldscript,
    srcs=[],
    hdrs=[],
    deps=[],
):
    _stm32g0xx_rule(
        name=name,
        srcs=srcs,
        hdrs=hdrs,
        deps=deps,
        ldscript=ldscript,
    )
