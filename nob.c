#define NOB_IMPLEMENTATION
#define NOB_WARN_DEPRECATED
#include "nob.h"

#define FLAG_IMPLEMENTATION
#include "flag.h"

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
#define RESOURCES_DIR "resources/"
#define RELEASE_DIR "release/"
#define RAYPATH_LINUX "./raylib-6.0_linux_amd64/"
#define RAYPATH_WINDOWS "./raylib-6.0_win64_mingw-w64/"

#define VERSION "0.1"

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

static bool build_windows(bool release)
{
    Nob_Cmd cmd = {0};
    nob_cmd_append(&cmd, "x86_64-w64-mingw32-gcc", "-Wall", "-Wextra", "-ggdb");
    if (release)
        nob_cmd_append(&cmd, "-O3");

    nob_cmd_append(&cmd, "-I.");
    nob_cmd_append(&cmd, "-I" RAYPATH_WINDOWS "include");
    nob_cmd_append(&cmd, "-o", BUILD_DIR "cronos.exe", SRC_DIR "main.c");
    nob_cmd_append(&cmd, "-L" RAYPATH_WINDOWS "lib");
    nob_cmd_append(&cmd, "-static-libgcc");

    nob_cmd_append(&cmd, "-lraylib", "-lopengl32", "-lgdi32", "-lwinmm");
    return nob_cmd_run(&cmd);
}

static bool package_release(const char *pkg_name, const char *binary_path, const char *archive_ext,
                            const char *archiver, const char *archiver_args[], size_t archiver_args_count)
{
    bool result = true;

    const char *stage_dir = nob_temp_sprintf("%s%s", BUILD_DIR, pkg_name);
    if (!nob_mkdir_if_not_exists(stage_dir))
        nob_return_defer(false);

    const char *binary_name = nob_path_name(binary_path);
    const char *staged_binary = nob_temp_sprintf("%s/%s", stage_dir, binary_name);
    if (!nob_copy_file(binary_path, staged_binary))
        nob_return_defer(false);

    const char *staged_resources = nob_temp_sprintf("%s/%s", stage_dir, RESOURCES_DIR);
    if (!nob_copy_directory_recursively(RESOURCES_DIR, staged_resources))
        nob_return_defer(false);

    if (!nob_mkdir_if_not_exists(RELEASE_DIR))
        nob_return_defer(false);

    size_t checkpoint = nob_temp_save();
    const char *release_path = nob_temp_sprintf("../%s%s%s", RELEASE_DIR, pkg_name, archive_ext);

    if (!nob_set_current_dir(BUILD_DIR))
        nob_return_defer(false);

    Nob_Cmd cmd = {0};
    nob_cmd_append(&cmd, archiver);
    for (size_t i = 0; i < archiver_args_count; ++i)
    {
        nob_cmd_append(&cmd, archiver_args[i] == NULL ? release_path : archiver_args[i]);
    }
    nob_cmd_append(&cmd, pkg_name);
    bool archived = nob_cmd_run(&cmd);

    if (!nob_set_current_dir(".."))
        nob_return_defer(false);
    if (!archived)
        nob_return_defer(false);

defer:
    nob_temp_rewind(checkpoint);
    return result;
}

static bool release_linux(void)
{
    const char *pkg_name = nob_temp_sprintf("cronos_v" VERSION "_linux_x86_64");
    const char *args[] = {"-czf", NULL};
    return package_release(pkg_name, BUILD_DIR "cronos", ".tar.gz", "tar", args, NOB_ARRAY_LEN(args));
}

static bool release_windows(void)
{
    const char *pkg_name = nob_temp_sprintf("cronos_v" VERSION "_windows_x86_64");
    const char *args[] = {"-r", NULL};
    return package_release(pkg_name, BUILD_DIR "cronos.exe", ".zip", "zip", args, NOB_ARRAY_LEN(args));
}

int main(int argc, char **argv)
{
    NOB_GO_REBUILD_URSELF(argc, argv);

    char **target_str = flag_str("target", "linux", "Build target: linux, windows, release");
    char **only_str = flag_str("only", "", "When -target=release, restrict the release to a single platform: linux or windows (default: both)");
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
        printf("    windows\n");
        printf("    release\n");
        return 0;
    }

    const char *target = *target_str;
    const char *only = *only_str;

    if (!nob_mkdir_if_not_exists(BUILD_DIR))
    {
        nob_log(NOB_ERROR, "Could not create build folder '%s'", BUILD_DIR);
        return 1;
    }

    if (strcmp(target, "linux") == 0)
    {
        if (!build_linux(false))
            return 1;
    }
    else if (strcmp(target, "windows") == 0)
    {
        if (!build_windows(false))
            return 1;
    }
    else if (strcmp(target, "release") == 0)
    {
        bool do_linux = strcmp(only, "windows") != 0;
        bool do_windows = strcmp(only, "linux") != 0;

        if (*only != '\0' && strcmp(only, "linux") != 0 && strcmp(only, "windows") != 0)
        {
            nob_log(NOB_ERROR, "Unknown -only value '%s' (expected 'linux' or 'windows')", only);
            return 1;
        }

        if (do_linux)
        {
            if (!build_linux(true))
                return 1;
            if (!release_linux())
                return 1;
        }
        if (do_windows)
        {
            if (!build_windows(true))
                return 1;
            if (!release_windows())
                return 1;
        }
    }
    else
    {
        nob_log(NOB_ERROR, "Unknown build target '%s' (expected 'linux', 'windows' or 'release')", target);
        return 1;
    }

    return 0;
}