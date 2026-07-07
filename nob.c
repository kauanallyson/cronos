#define NOB_IMPLEMENTATION
#include "nob.h"

#define BUILD_FOLDER "build/"
#define SRC_FOLDER "src/"
#define RAYPATH "raylib-6.0_linux_amd64/"

int main(int argc, char **argv)
{
    NOB_GO_REBUILD_URSELF(argc, argv);

    if (!nob_mkdir_if_not_exists(BUILD_FOLDER))
    {
        nob_log(NOB_ERROR, "Could not create build folder '%s'", BUILD_FOLDER);
        return 1;
    }

    Nob_Cmd cmd = {0};
    nob_cmd_append(&cmd, "gcc", "-Wall", "-Wextra",
                   "-o",
                   BUILD_FOLDER "cronos", SRC_FOLDER "main.c");

    nob_cmd_append(&cmd, "-I" RAYPATH "include");
    nob_cmd_append(&cmd, "-L" RAYPATH "lib");
    nob_cmd_append(&cmd, "-l:libraylib.a", "-lm", "-lX11");

    if (!nob_cmd_run(&cmd))
        return 1;

    return 0;
}