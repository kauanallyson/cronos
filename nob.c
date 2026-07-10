#define NOB_IMPLEMENTATION
#define NOB_WARN_DEPRECATED
#include "nob.h"
#define FLAG_IMPLEMENTATION
#include "flag.h"

#define BUILD_DIR "build/"
#define SRC_DIR "src/"
#define RAYPATH_LINUX "./raylib-6.0_linux_amd64/"
#define RAYPATH_WINDOWS "./raylib-6.0_win64_mingw-w64/"

static bool build_defaults(Nob_Cmd *cmd, bool release)
{
    nob_cmd_append(cmd, "cc", "-Wall", "-Wextra");
    if (release)
        nob_cmd_append(cmd, "-O3");
    else
    {
        nob_cmd_append(cmd, "-ggdb");
    }
}

static bool linux_flags(Nob_Cmd *cmd)
{
    nob_cmd_append(cmd, "-I.");
    nob_cmd_append(cmd, "-I" RAYPATH_LINUX "include");
    nob_cmd_append(cmd, "-o", BUILD_DIR "cronos", SRC_DIR "main.c");
    nob_cmd_append(cmd, "-L" RAYPATH_LINUX "lib");
    nob_cmd_append(cmd, "-l:libraylib.a", "-lm", "-lX11");

    return nob_cmd_run(cmd);
}

static bool win_flags(Nob_Cmd *cmd)
{
    nob_cmd_append(cmd, "-I.");
    nob_cmd_append(cmd, "-I" RAYPATH_WINDOWS "include");
    nob_cmd_append(cmd, "-o", BUILD_DIR "cronos.exe", SRC_DIR "main.c");
    nob_cmd_append(cmd, "-L" RAYPATH_WINDOWS "lib");
    nob_cmd_append(cmd, "-l:libraylib.a", "-lgdi32", "-luser32", "-lwinmm", "-lshell32", "-mwindows");

    return nob_cmd_run(cmd);
}

int main(int argc, char **argv)
{
    NOB_GO_REBUILD_URSELF(argc, argv);

    // Core Flags
    bool *list = flag_bool("list", false, "List all available build targets and exit");
    bool *help = flag_bool("help", false, "Print this help message and exit");
    bool *release = flag_bool("release", false, "Build in release mode with optimizations");

    // Target Flags
    bool *linux_flag = flag_bool("linux", false, "Build for Linux target");
    bool *win_flag = flag_bool("win", false, "Build for Windows target");

    if (!flag_parse(argc, argv))
    {
        flag_print_error(stderr);
        flag_print_options(stderr);
        return 1;
    }

    if (*help)
    {
        printf("Usage: ./nob [OPTIONS]\n");
        flag_print_options(stdout);
        return 0;
    }

    if (*list)
    {
        printf("Available targets:\n");
        printf("    linux\n");
        printf("    win\n");
        return 0;
    }

    if (!nob_mkdir_if_not_exists(BUILD_DIR))
    {
        nob_log(NOB_ERROR, "Could not create build folder '%s'", BUILD_DIR);
        return 1;
    }

    bool target_linux = false;
    bool target_win = false;

    if (*linux_flag)
    {
        target_linux = true;
    }
    else if (*win_flag)
    {
        target_win = true;
    }
    else
    {
#ifdef _WIN32
        target_win = true;
#else
        target_linux = true;
#endif
    }

    Nob_Cmd cmd = {0};
    build_defaults(&cmd, *release);

    if (target_linux)
    {
        nob_log(NOB_INFO, "Building for Linux...");
        if (!linux_flags(&cmd))
            return 1;
    }
    else if (target_win)
    {
        nob_log(NOB_INFO, "Building for Windows...");
        if (!win_flags(&cmd))
            return 1;
    }

    nob_log(NOB_INFO, "Build successful!");
    return 0;
}