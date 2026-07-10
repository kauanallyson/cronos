#define NOB_IMPLEMENTATION
#define NOB_WARN_DEPRECATED
#include "nob.h"
#define FLAG_IMPLEMENTATION
#include "flag.h"

#define BUILD_DIR "build/"
#define SRC_DIR "src/"

#ifdef _WIN32
#define RAYPATH "./raylib-6.0_win64_mingw-w64/"
#define RAYGUI_OBJ BUILD_DIR "raygui.o"
#define OUT_BIN BUILD_DIR "cronos.exe"
#else
#define RAYPATH "./raylib-6.0_linux_amd64/"
#define RAYGUI_OBJ BUILD_DIR "raygui.o"
#define OUT_BIN BUILD_DIR "cronos"
#endif

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

static bool compile_raygui(bool release)
{
    const char *inputs[] = {SRC_DIR "raygui.c", SRC_DIR "raygui.h"};
    int rebuild = nob_needs_rebuild(RAYGUI_OBJ, inputs, NOB_ARRAY_LEN(inputs));
    if (rebuild < 0)
        return false;
    if (rebuild == 0)
        return true;

    nob_log(NOB_INFO, "Compiling raygui...");
    Nob_Cmd cmd = {0};
    build_defaults(&cmd, release);
    nob_cmd_append(&cmd, "-I.");
    nob_cmd_append(&cmd, "-I" RAYPATH "include");
    nob_cmd_append(&cmd, "-c", "-o", RAYGUI_OBJ, SRC_DIR "raygui.c");
    return nob_cmd_run(&cmd);
}

static bool build(Nob_Cmd *cmd)
{
    nob_cmd_append(cmd, "-I.");
    nob_cmd_append(cmd, "-I" RAYPATH "include");
    nob_cmd_append(cmd, "-o", OUT_BIN, SRC_DIR "main.c", RAYGUI_OBJ);
    nob_cmd_append(cmd, "-L" RAYPATH "lib");
#ifdef _WIN32
    nob_cmd_append(cmd, "-l:libraylib.a", "-lgdi32", "-luser32", "-lwinmm", "-lshell32", "-mwindows");
#else
    nob_cmd_append(cmd, "-l:libraylib.a", "-lm", "-lX11");
#endif

    return nob_cmd_run(cmd);
}

int main(int argc, char **argv)
{
    NOB_GO_REBUILD_URSELF(argc, argv);

    bool *help = flag_bool("help", false, "Print this help message and exit");
    bool *release = flag_bool("release", false, "Build in release mode with optimizations");

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

    if (!nob_mkdir_if_not_exists(BUILD_DIR))
    {
        nob_log(NOB_ERROR, "Could not create build folder '%s'", BUILD_DIR);
        return 1;
    }

    if (!compile_raygui(*release))
        return 1;

    Nob_Cmd cmd = {0};
    build_defaults(&cmd, *release);
    if (!build(&cmd))
        return 1;

    nob_log(NOB_INFO, "Build successful!");
    return 0;
}
