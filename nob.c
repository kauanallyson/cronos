#define NOB_IMPLEMENTATION
#define NOB_WARN_DEPRECATED
#include "nob.h"

#define BUILD_DIR "build/"
#define SRC_DIR "src/"
#define RAYPATH "./raylib-6.0_linux_amd64/"

int main(int argc, char **argv)
{
    NOB_GO_REBUILD_URSELF(argc, argv);

    if (!nob_mkdir_if_not_exists(BUILD_DIR))
    {
        nob_log(NOB_ERROR, "Could not create build folder '%s'", BUILD_DIR);
        return 1;
    }

    Nob_Cmd cmd = {0};
    nob_cmd_append(&cmd, "gcc", "-Wall", "-Wextra", "-ggdb");

    nob_cmd_append(&cmd, "-I.");
    nob_cmd_append(&cmd, "-I" RAYPATH "include");
    nob_cmd_append(&cmd, "-o", BUILD_DIR "cronos", SRC_DIR "main.c");
    nob_cmd_append(&cmd, "-L" RAYPATH "lib");
    nob_cmd_append(&cmd, "-l:libraylib.a", "-lm", "-lX11");

    if (!nob_cmd_run(&cmd))
        return 1;

    return 0;
}