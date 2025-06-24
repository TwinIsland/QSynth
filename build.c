#define NOB_IMPLEMENTATION
#include "nob.h"

#define BUILD_FOLDER "build/"
#define SRC_FOLDER "src/"
#define tests_FOLDER "tests/"
#define UI_FOLDER "ui/"
#define LIBS_FOLDER "libs/"
#define DEVICE_FOLDER "device/"

// Function to copy all DLLs from libs to build folder
bool copy_dlls_to_build(void)
{
    Nob_File_Paths dll_paths = {0};

    // Find all .dll files in libs folder
    if (!nob_read_entire_dir(LIBS_FOLDER, &dll_paths))
    {
        nob_log(NOB_ERROR, "Could not read libs directory");
        return false;
    }

    bool result = true;
    for (size_t i = 0; i < dll_paths.count; i++)
    {
        const char *file_name = dll_paths.items[i];

        // Check if it's a .dll file
        size_t len = strlen(file_name);
        if (len > 4 && strcmp(file_name + len - 4, ".dll") == 0)
        {
            // Build source and destination paths
            Nob_String_Builder src_path = {0};
            Nob_String_Builder dest_path = {0};

            nob_sb_append_cstr(&src_path, LIBS_FOLDER);
            nob_sb_append_cstr(&src_path, file_name);
            nob_sb_append_null(&src_path);

            nob_sb_append_cstr(&dest_path, BUILD_FOLDER);
            nob_sb_append_cstr(&dest_path, file_name);
            nob_sb_append_null(&dest_path);

            nob_log(NOB_INFO, "Copying %s to %s", src_path.items, dest_path.items);

            if (!nob_copy_file(src_path.items, dest_path.items))
            {
                nob_log(NOB_ERROR, "Failed to copy %s", file_name);
                result = false;
            }

            nob_sb_free(src_path);
            nob_sb_free(dest_path);
        }
    }

    nob_da_free(dll_paths);
    return result;
}

// Function to build miniaudio static library
bool build_miniaudio_lib(bool debug_build, bool x64_build, bool release_build)
{
    // Check if miniaudio implementation file exists
    const char *miniaudio_impl = SRC_FOLDER "audio/miniaudio.c";
    if (!nob_file_exists(miniaudio_impl))
    {
        nob_log(NOB_ERROR, "miniaudio implementation file not found: %s", miniaudio_impl);
        return false;
    }

    Nob_Cmd cmd = {0};

    // Compile to object file
    nob_cmd_append(&cmd, "gcc");
    if (x64_build)
        nob_cmd_append(&cmd, "-m64");
    nob_cmd_append(&cmd, "-Wall", "-Wextra", "-std=c99");

    if (debug_build)
    {
        nob_cmd_append(&cmd, "-g", "-O0", "-DDEBUG");
    }
    else if (release_build)
    {
        nob_cmd_append(&cmd, "-O2", "-DNDEBUG");
    }

    nob_cmd_append(&cmd, "-c", miniaudio_impl, "-o", LIBS_FOLDER "miniaudio.o");

    nob_log(NOB_INFO, "Compiling miniaudio to object file...");
    if (!nob_cmd_run_sync_and_reset(&cmd))
        return false;

    // Create static library
    nob_cmd_append(&cmd, "ar", "rcs", LIBS_FOLDER "libminiaudio.a", LIBS_FOLDER "miniaudio.o");

    nob_log(NOB_INFO, "Creating miniaudio static library...");
    return nob_cmd_run_sync_and_reset(&cmd);
}

// Function to build UI application
bool build_ui(bool debug_build, bool x64_build, bool release_build)
{
    // Check if UI main file exists
    const char *ui_main_file = UI_FOLDER "Qsynth.c";
    if (!nob_file_exists(ui_main_file))
    {
        nob_log(NOB_ERROR, "UI main file not found: %s", ui_main_file);
        return false;
    }

    // Build the command
    Nob_Cmd cmd = {0};

    // Compiler
    nob_cmd_append(&cmd, "gcc");

    // Architecture
    if (x64_build)
    {
        nob_cmd_append(&cmd, "-m64");
    }

    // Warning flags
    nob_cmd_append(&cmd, "-Wall", "-Wextra", "-std=c99", "-mwindows");

    // Build type flags
    if (debug_build)
    {
        nob_cmd_append(&cmd, "-g", "-O0", "-DDEBUG");
    }
    else if (release_build)
    {
        nob_cmd_append(&cmd, "-O2", "-DNDEBUG");
    }

    // Include directories
    nob_cmd_append(&cmd, "-Iinclude", "-Iassets", "-Ipthread", "-Iui");

    // Library directories
    nob_cmd_append(&cmd, "-Llibs");

    // Source files
    nob_cmd_append(&cmd, SRC_FOLDER "assets/instruments.c");
    nob_cmd_append(&cmd, SRC_FOLDER "core/core.c");
    nob_cmd_append(&cmd, SRC_FOLDER "core/voice.c");
    nob_cmd_append(&cmd, SRC_FOLDER "envelope/adsr.c");
    nob_cmd_append(&cmd, SRC_FOLDER "filters/biquad.c");
    nob_cmd_append(&cmd, SRC_FOLDER "oscillators/oscillators.c");
    nob_cmd_append(&cmd, SRC_FOLDER "utils/note_table.c");

    // UI main file
    nob_cmd_append(&cmd, ui_main_file);

    // Output file
    Nob_String_Builder output_path = {0};
    nob_sb_append_cstr(&output_path, BUILD_FOLDER);
    nob_sb_append_cstr(&output_path, "QSynth.exe");
    nob_sb_append_null(&output_path);

    nob_cmd_append(&cmd, "-o", output_path.items);

    // Raylib and system libraries
    nob_cmd_append(&cmd, "-lm");

    nob_cmd_append(&cmd, "-lminiaudio");
    nob_cmd_append(&cmd, "-lraylib");
    nob_cmd_append(&cmd, "-lgdi32");
    nob_cmd_append(&cmd, "-lwinmm");      // Windows multimedia
    nob_cmd_append(&cmd, "-lpthreadVC2"); // Pthread

    // Execute compilation
    nob_log(NOB_INFO, "Building QSynth UI with Raylib...");
    bool result = nob_cmd_run_sync_and_reset(&cmd);

    nob_sb_free(output_path);
    return result;
}

// Function to build tests application
bool build_tests(const char *tests_name, bool debug_build, bool x64_build, bool release_build)
{
    // Check if tests file exists
    Nob_String_Builder tests_path = {0};
    nob_sb_append_cstr(&tests_path, tests_FOLDER);
    nob_sb_append_cstr(&tests_path, tests_name);
    nob_sb_append_cstr(&tests_path, ".c");
    nob_sb_append_null(&tests_path);

    if (!nob_file_exists(tests_path.items))
    {
        nob_log(NOB_ERROR, "tests file not found: %s", tests_path.items);
        nob_sb_free(tests_path);
        return false;
    }

    // Build the command
    Nob_Cmd cmd = {0};

    // Compiler
    nob_cmd_append(&cmd, "gcc");

    // Architecture
    if (x64_build)
    {
        nob_cmd_append(&cmd, "-m64");
    }
    // Warning flags
    nob_cmd_append(&cmd, "-Wall", "-Wextra", "-std=c99");

    // Build type flags
    if (debug_build)
    {
        nob_cmd_append(&cmd, "-g", "-O0", "-DDEBUG");
    }
    else if (release_build)
    {
        nob_cmd_append(&cmd, "-O2", "-DNDEBUG");
    }

    // Include directories
    nob_cmd_append(&cmd, "-Iinclude", "-Iassets", "-Ipthread");

    // Library directories
    nob_cmd_append(&cmd, "-Llibs");

    // Source files
    nob_cmd_append(&cmd, SRC_FOLDER "assets/instruments.c");
    nob_cmd_append(&cmd, SRC_FOLDER "core/core.c");
    nob_cmd_append(&cmd, SRC_FOLDER "core/voice.c");
    nob_cmd_append(&cmd, SRC_FOLDER "envelope/adsr.c");
    nob_cmd_append(&cmd, SRC_FOLDER "filters/biquad.c");
    nob_cmd_append(&cmd, SRC_FOLDER "oscillators/oscillators.c");
    nob_cmd_append(&cmd, SRC_FOLDER "utils/note_table.c");

    // tests file
    nob_cmd_append(&cmd, tests_path.items);

    // Libraries
    nob_cmd_append(&cmd, "-lminiaudio", "-lwinmm", "-lpthreadVC2");
    nob_cmd_append(&cmd, "-lwinmm", "-lpthreadVC2");

    // Output file
    Nob_String_Builder output_path = {0};
    nob_sb_append_cstr(&output_path, BUILD_FOLDER);
    nob_sb_append_cstr(&output_path, tests_name);
    nob_sb_append_cstr(&output_path, ".exe");
    nob_sb_append_null(&output_path);

    nob_cmd_append(&cmd, "-o", output_path.items);

    // Execute compilation
    nob_log(NOB_INFO, "Building %s...", tests_name);
    bool result = nob_cmd_run_sync_and_reset(&cmd);

    // Cleanup
    nob_sb_free(tests_path);
    nob_sb_free(output_path);

    return result;
}

// Function to print usage
void print_usage(const char *program_name)
{
    nob_log(NOB_INFO, "Usage: %s <target> [options]", program_name);
    nob_log(NOB_INFO, "");
    nob_log(NOB_INFO, "Targets:");
    nob_log(NOB_INFO, "  ui                  Build QSynth UI application");
    nob_log(NOB_INFO, "  <tests_name>      Build specific tests");
    nob_log(NOB_INFO, "");
    nob_log(NOB_INFO, "tests:");
    nob_log(NOB_INFO, "  %s ui", program_name);
    nob_log(NOB_INFO, "  %s instruments_test", program_name);
    nob_log(NOB_INFO, "  %s basic_synth", program_name);
    nob_log(NOB_INFO, "  %s sine_wave_test", program_name);
    nob_log(NOB_INFO, "");
    nob_log(NOB_INFO, "Options:");
    nob_log(NOB_INFO, "  --debug     Build with debug symbols");
    nob_log(NOB_INFO, "  --x64       Build for 64-bit");
    nob_log(NOB_INFO, "  --release   Build with optimizations (default)");
}

int main(int argc, char **argv)
{
    NOB_GO_REBUILD_URSELF(argc, argv);

    // Parse command line arguments
    if (argc < 2)
    {
        print_usage(argv[0]);
        return 1;
    }

    const char *target = argv[1];
    bool debug_build = false;
    bool x64_build = false;
    bool release_build = true;

    // Parse options
    for (int i = 2; i < argc; i++)
    {
        if (strcmp(argv[i], "--debug") == 0)
        {
            debug_build = true;
            release_build = false;
        }
        else if (strcmp(argv[i], "--x64") == 0)
        {
            x64_build = true;
        }
        else if (strcmp(argv[i], "--release") == 0)
        {
            release_build = true;
            debug_build = false;
        }
        else
        {
            nob_log(NOB_ERROR, "Unknown option: %s", argv[i]);
            print_usage(argv[0]);
            return 1;
        }
    }

    // Create build folder
    if (!nob_mkdir_if_not_exists(BUILD_FOLDER))
    {
        return 1;
    }

    // Build miniaudio static library first
    if (!nob_file_exists(LIBS_FOLDER "libminiaudio.a"))
    {
        nob_log(NOB_INFO, "Building miniaudio static library...");
        if (!build_miniaudio_lib(debug_build, x64_build, release_build))
        {
            nob_log(NOB_ERROR, "Failed to build miniaudio library!");
            return 1;
        }
        nob_delete_file(LIBS_FOLDER "miniaudio.o");
    } else {
        printf("miniaudio static library already exist. skip compiling.\n");
    }

    bool build_success = false;

    // Check if target is UI
    if (strcmp(target, "ui") == 0)
    {
        build_success = build_ui(debug_build, x64_build, release_build);

        if (build_success)
        {
            nob_log(NOB_INFO, "UI build successful!");
            nob_log(NOB_INFO, "Executable: %QSynth.exe", BUILD_FOLDER);
            nob_log(NOB_INFO, "To run: cd %s && ./QSynth.exe", BUILD_FOLDER);
        }
    }
    else
    {
        // Build tests
        build_success = build_tests(target, debug_build, x64_build, release_build);

        if (build_success)
        {
            nob_log(NOB_INFO, "tests build successful!");
            nob_log(NOB_INFO, "Executable: %s%s.exe", BUILD_FOLDER, target);
            nob_log(NOB_INFO, "To run: cd %s && ./%s.exe", BUILD_FOLDER, target);
        }
    }

    if (build_success)
    {
        // Copy DLLs to build folder
        nob_log(NOB_INFO, "Copying DLLs...");
        if (!copy_dlls_to_build())
        {
            nob_log(NOB_ERROR, "Failed to copy DLLs");
            return 1;
        }
    }
    else
    {
        nob_log(NOB_ERROR, "Build failed!");
        return 1;
    }

    return 0;
}