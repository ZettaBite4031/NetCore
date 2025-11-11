#define NOB_IMPLEMENTATION
#include "nob.h"


int main(int argc, char** argv) {
    NOB_GO_REBUILD_URSELF(argc, argv);
    
    const char* compiler = "g++";
    Nob_Cmd cmd = {0};
    
    nob_mkdir_if_not_exists("build");

    const char* cxxflags[] = {
        "-std=c++23",
        "-Wall",
        "-Wextra",
        "-Wpedantic",
        "-ggdb",
        NULL
    };

    const char* lib_srcs[] = {
        "src/url.cpp",
        "src/beast_http_transport.cpp",
        NULL
    };

    Nob_File_Paths objs = {0};

    for (int i = 0; lib_srcs[i]; i++) {
        const char* src = lib_srcs[i];
        Nob_String_Builder sb = {0};
        nob_sb_append_cstr(&sb, "build/");
        nob_sb_append_cstr(&sb, nob_path_name(src));
        nob_sb_append_cstr(&sb, ".o");
        Nob_String_View sv = nob_sb_to_sv(sb);
        const char* obj = nob_temp_sv_to_cstr(sv);

        nob_log(NOB_INFO, "Compiling %s -> %s", src, obj);

        nob_cmd_append(&cmd, compiler);
        nob_cmd_append(&cmd, "-c", src);
        for (int j = 0; cxxflags[j]; j++) {
            nob_cmd_append(&cmd, cxxflags[j]);
        }
        nob_cmd_append(&cmd, "-o", obj);

        if (!nob_cmd_run_sync_and_reset(&cmd)) {
            nob_log(NOB_ERROR, "Failed to compile %s", src);
            return 1;
        }

        nob_da_append(&objs, obj);
    }

    const char* example = "examples/main.cpp";
    const char* example_exe = "build/example";

    nob_cmd_append(&cmd, compiler);
    nob_cmd_append(&cmd, "-o", example_exe);
    nob_cmd_append(&cmd, example);
    nob_cmd_append(&cmd, "-Iinclude", "-std=c++23", "-Wall", "-Wextra", "-Wpedantic", "-ggdb");

    for (size_t i = 0; i < objs.count; i++) {
        nob_cmd_append(&cmd, objs.items[i]);
    }

    if (!nob_cmd_run_sync_and_reset(&cmd)) {
        nob_log(NOB_ERROR, "Failed to link %s", example_exe);
        return 1;
    }

    nob_log(NOB_INFO, "Build: %s", example_exe);

    return 0;
}