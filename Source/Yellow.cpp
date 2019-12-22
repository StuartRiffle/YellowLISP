// YellowLISP (c) 2019 Stuart Riffle

#include "Yellow.h"
#include "Interpreter.h"
#include "Testing.h"
#include "Console.h"
#include "CommandLine.h"

void PrintOptions()
{
    printf("__  __     ____              __    _________ ____ \n");
    printf("\\ \\/ /__  / / /___ _      __/ /   /  _/ ___// __ \\\n");
    printf(" \\  / _ \\/ / / __ \\ | /| / / /    / / \\__ \\/ /_/ /\n");
    printf(" / /  __/ / / /_/ / |/ |/ / /____/ / ___/ / ____/ \n");
    printf("/_/\\___/_/_/\\____/|__/|__/_____/___//____/_/      \n\n");

    printf("Usage: YellowLISP [options] [lisp files]\n\n");

    printf("This is an interpreter for a [very] small subset of LISP. It is a work in\n");
    printf("progress, and should not be used for production work.\n\n");

    printf("LISP sources have the extension \".lisp\". If any are specified, they will\n"); 
    printf("be loaded and evaluated in the order given. YellowLISP will exit after that.\n\n");

    printf("Type \"(help)\" at the interactive prompt to see the runtime options.\n");
    printf("Type \"(exit)\" (or hit CTRL-C) to end your session.\n\n");

    printf("Available command line options are:\n");
    printf("  --repl      Drop to an interactive prompt after running any LISP files\n");
    printf("  --debug     Run in debug mode (verbose diagnostic output)\n");
    printf("  --version   Print just the version and exit\n");
    printf("  --help      Display this message and exit\n");
    printf("\n");

    printf("The latest version of YellowLISP can [theoretically] be found at:\n");
    printf("  https://github.com/StuartRiffle/YellowLISP\n");
}

int main(int argc, char** argv)
{
    SanityCheck();

    CommandLine commandLine(argc, argv);
    InterpreterSettings settings;

    char versionStr[80];
    sprintf(versionStr, "YellowLISP %d.%d.%d", VERSION_MAJOR, VERSION_MINOR, VERSION_PATCH);

    // Handle flags

    if (commandLine.HasFlag("--version"))
    {
        printf("%s\n", versionStr);
        return RETURN_SUCCESS;
    }

    if (commandLine.HasFlag("--help"))
    {
        PrintOptions();
        return RETURN_SUCCESS;
    }

    if (commandLine.HasFlag("--repl"))
        settings._repl = true;

    if (commandLine.HasFlag("--debug"))
        settings._debugMode = true;

    // Now try and LISP something

    Interpreter lisp(&settings);

    try
    {
        vector<string> sourceFiles = commandLine.ArgsEndingWith(".lisp");
        if (sourceFiles.size() > 0)
        {
            // LISP source was specified, so run it all then exit

            for (string& filename : sourceFiles)
            {
                std::ifstream file(filename);
                if (!file)
                {
                    printf("ERROR: File not found: %s\n", filename.c_str());
                    return -1; // FIXME: make this a proper return value
                }

                std::stringstream ss;
                ss << file.rdbuf();
                string source = ss.str();

                string output = lisp.Evaluate(source);
                printf("%s\n", output.c_str());
            }

            if (!settings._repl)
                return RETURN_SUCCESS;
        }

        // Otherwise, drop into the REPL

        SetTextColor(ANSI_BLACK, ANSI_YELLOW);
        printf("\n %s \n", versionStr);
        ResetTextColor();

        lisp.RunREPL();
    }
    catch (std::exception e)
    {
        printf("%s\n", e.what());
        return -1; // FIXME: make this a proper return value
    }

    return RETURN_SUCCESS;
}
