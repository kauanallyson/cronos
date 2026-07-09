#define NOB_IMPLEMENTATION
#define NOB_WARN_DEPRECATED
#include "nob.h"

#ifdef __MINGW32__
#include <sys/stat.h>
#include <sys/types.h>
int _stat64i32(const char *path, struct _stat64i32 *buffer)
{
    return _stat64(path, (struct __stat64 *)buffer);
}
#endif

#define BUILD_DIR "build/"
#define SRC_DIR "src/"
#define RAYPATH_LINUX "./raylib-6.0_linux_amd64/"
#define RAYPATH_WINDOWS "./raylib-6.0_win64_mingw-w64/"

static bool build_linux(void)
{
    Nob_Cmd cmd = {0};
    nob_cmd_append(&cmd, "cc", "-Wall", "-Wextra", "-ggdb");

    nob_cmd_append(&cmd, "-I.");
    nob_cmd_append(&cmd, "-I" RAYPATH_LINUX "include");
    nob_cmd_append(&cmd, "-o", BUILD_DIR "cronos", SRC_DIR "main.c");
    nob_cmd_append(&cmd, "-L" RAYPATH_LINUX "lib");
    nob_cmd_append(&cmd, "-l:libraylib.a", "-lm", "-lX11");

    return nob_cmd_run(&cmd);
}

static bool build_windows(void)
{
    Nob_Cmd cmd = {0};
    nob_cmd_append(&cmd, "x86_64-w64-mingw32-gcc", "-Wall", "-Wextra", "-ggdb");

    nob_cmd_append(&cmd, "-I.");
    nob_cmd_append(&cmd, "-I" RAYPATH_WINDOWS "include");
    nob_cmd_append(&cmd, "-o", BUILD_DIR "cronos.exe", SRC_DIR "main.c");
    nob_cmd_append(&cmd, "-L" RAYPATH_WINDOWS "lib");
    nob_cmd_append(&cmd, "-static-libgcc");

    nob_cmd_append(&cmd, "-lraylib", "-lopengl32", "-lgdi32", "-lwinmm");
    return nob_cmd_run(&cmd);
}

int main(int argc, char **argv)
{
    NOB_GO_REBUILD_URSELF(argc, argv);

    nob_shift(argv, argc); // skip program name
    const char *target = argc > 0 ? nob_shift(argv, argc) : "linux";

    if (!nob_mkdir_if_not_exists(BUILD_DIR))
    {
        nob_log(NOB_ERROR, "Could not create build folder '%s'", BUILD_DIR);
        return 1;
    }

    if (strcmp(target, "linux") == 0)
    {
        if (!build_linux())
            return 1;
    }
    else if (strcmp(target, "windows") == 0)
    {
        if (!build_windows())
            return 1;
    }
    else
    {
        nob_log(NOB_ERROR, "Unknown build target '%s' (expected 'linux' or 'windows')", target);
        return 1;
    }

    return 0;
}