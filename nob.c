#define NOB_IMPLEMENTATION
#define NOB_WARN_DEPRECATED
#include "nob.h"

#define FLAG_IMPLEMENTATION
#include "flag.h"

#define BUILD_DIR "build/"
#define SRC_DIR "src/"
#define RAYPATH_LINUX "./raylib-6.0_linux_amd64/"

static bool build_linux(bool release)
{
    Nob_Cmd cmd = {0};
    nob_cmd_append(&cmd, "cc", "-Wall", "-Wextra", "-ggdb");
    if (release)
        nob_cmd_append(&cmd, "-O3");

    nob_cmd_append(&cmd, "-I.");
    nob_cmd_append(&cmd, "-I" RAYPATH_LINUX "include");
    nob_cmd_append(&cmd, "-o", BUILD_DIR "cronos", SRC_DIR "main.c");
    nob_cmd_append(&cmd, "-L" RAYPATH_LINUX "lib");
    nob_cmd_append(&cmd, "-l:libraylib.a", "-lm", "-lX11");

    return nob_cmd_run(&cmd);
}

int main(int argc, char **argv)
{
    NOB_GO_REBUILD_URSELF(argc, argv);

    bool *list = flag_bool("list", false, "List all available build targets and exit");
    bool *help = flag_bool("help", false, "Print this help message and exit");

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
        return 0;
    }

    if (!nob_mkdir_if_not_exists(BUILD_DIR))
    {
        nob_log(NOB_ERROR, "Could not create build folder '%s'", BUILD_DIR);
        return 1;
    }

    if (!build_linux(false))
        return 1;

    return 0;
}
