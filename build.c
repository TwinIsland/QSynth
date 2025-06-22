#define NOB_IMPLEMENTATION
#include "nob.h"

#define BUILD_FOLDER "build/"
#define SRC_FOLDER "src/"
#define EXAMPLES_FOLDER "examples/"
#define LIBS_FOLDER "libs/"
#define DEVICE_FOLDER "device/"

// Function to collect all .c files from src folder recursively
bool collect_src_files(Nob_String_Builder *sb)
{
    // Add device files
    nob_sb_append_cstr(sb, DEVICE_FOLDER "winaudio.c ");

    // Add all source files (you can expand this to be recursive if needed)
    nob_sb_append_cstr(sb, SRC_FOLDER "core/core.c ");
    nob_sb_append_cstr(sb, SRC_FOLDER "voice/voice.c ");
    nob_sb_append_cstr(sb, SRC_FOLDER "synthesizer/synthesizer.c ");
    // Add more source files as needed...

    return true;
}

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

// Function to print usage
void print_usage(const char *program_name)
{
    nob_log(NOB_INFO, "Usage: %s <example_name> [options]", program_name);
    nob_log(NOB_INFO, "");
    nob_log(NOB_INFO, "Examples:");
    nob_log(NOB_INFO, "  %s instruments_test", program_name);
    nob_log(NOB_INFO, "  %s basic_synth", program_name);
    nob_log(NOB_INFO, "  %s sine_wave_test", program_name);
    nob_log(NOB_INFO, "");
    nob_log(NOB_INFO, "Options:");
    nob_log(NOB_INFO, "  --debug     Build with debug symbols");
    nob_log(NOB_INFO, "  --x64       Build for 64-bit (default: 32-bit)");
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

    const char *example_name = argv[1];
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

    // Check if example file exists
    Nob_String_Builder example_path = {0};
    nob_sb_append_cstr(&example_path, EXAMPLES_FOLDER);
    nob_sb_append_cstr(&example_path, example_name);
    nob_sb_append_cstr(&example_path, ".c");
    nob_sb_append_null(&example_path);

    if (!nob_file_exists(example_path.items))
    {
        nob_log(NOB_ERROR, "Example file not found: %s", example_path.items);
        nob_sb_free(example_path);
        return 1;
    }

    // Create build folder
    if (!nob_mkdir_if_not_exists(BUILD_FOLDER))
    {
        nob_sb_free(example_path);
        return 1;
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
    else
    {
        nob_cmd_append(&cmd, "-m32");
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
    nob_cmd_append(&cmd, "-Iinclude", "-Idevice", "-Iassets", "-Ipthread");

    // Library directories
    nob_cmd_append(&cmd, "-Llibs");

    // Source files
    nob_cmd_append(&cmd, DEVICE_FOLDER "winaudio.c");
    nob_cmd_append(&cmd, SRC_FOLDER "**/*.c");

    // Example file
    nob_cmd_append(&cmd, example_path.items);

    // Libraries
    nob_cmd_append(&cmd, "-lwinmm", "-lpthreadVC2");

    // Output file
    Nob_String_Builder output_path = {0};
    nob_sb_append_cstr(&output_path, BUILD_FOLDER);
    nob_sb_append_cstr(&output_path, example_name);
    nob_sb_append_cstr(&output_path, ".exe");
    nob_sb_append_null(&output_path);

    nob_cmd_append(&cmd, "-o", output_path.items);

    // Execute compilation
    nob_log(NOB_INFO, "Building %s...", example_name);
    if (!nob_cmd_run_sync_and_reset(&cmd))
    {
        nob_sb_free(example_path);
        nob_sb_free(output_path);
        return 1;
    }

    // Copy DLLs to build folder
    nob_log(NOB_INFO, "Copying DLLs...");
    if (!copy_dlls_to_build())
    {
        nob_sb_free(example_path);
        nob_sb_free(output_path);
        return 1;
    }

    nob_log(NOB_INFO, "Build successful!");
    nob_log(NOB_INFO, "Executable: %s", output_path.items);
    nob_log(NOB_INFO, "To run: cd %s && ./%s.exe", BUILD_FOLDER, example_name);

    // Cleanup
    nob_sb_free(example_path);
    nob_sb_free(output_path);

    return 0;
}