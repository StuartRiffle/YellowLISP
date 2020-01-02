// YellowLISP (c) 2020 Stuart Riffle (MIT license)

#include "Yellow.h"
#include "Interpreter.h"
#include "Testing.h"
#include "Console.h"
#include "CommandLine.h"

void PrintTitleASCII(Console* console)
{
    const char* titleASCII = R"TITLE(
            __  __     ____              __    _________ ____
    _______ \ \/ /__  / / /___ _      __/ /   /  _/ ___// __ \ _________
   _________ \  / _ \/ / / __ \ | /| / / /    / / \__ \/ /_/ / ________ 
  _________  / / ___/ / / /_/ / |/ |/ / /____/ / ___/ / ____/ ________  
            /_/\___/_/_/\____/|__/|__/_____/___//____/_/


)TITLE";

    console->PrintColor(COLOR_YELLOW, 0, titleASCII);
}

void PrintTitleANSI(Console* console)
{
    const char* titleANSI = R"TITLE(
  #|  #|                                     #|     #####|  ###|  ####|  
  #|  #| #####| #|     #|      ###|  #|   #| #|       #|   #|     #|  #| 
   ###|  #|     #|     #|     #|  #| #|   #| #|       #|    ###|  #|  #| 
    #|   ###|   #|     #|     #|  #| #| | #| #|       #|       #| ####|  
    #|   #|     #|     #|     #|  #| ######| #|       #|       #| #|     
    #|   #####| #####| #####|  ###|   #| #|  #####| #####|  ###|  #|     

)TITLE";
    int thickLine = 220;

    for (const char* str = titleANSI; *str; str++)
    {
        if(*str == '#')
            console->PrintColor(COLOR_YELLOW,  0, "%c", thickLine);
        else if(*str == '|')
            console->PrintColor(COLOR_BROWN,   0, "%c", thickLine);
        else
            console->Print("%c", *str);
    }
}
void PrintBanner(Console* console)
{
    if (console->IsExtendedCharSet())
        PrintTitleANSI(console);
    else
        PrintTitleASCII(console);

    console->PrintColor(COLOR_WHITE, 0, "    An open source interpreter for something not entirely unlike LISP\n");
    console->PrintColor(COLOR_BLUE,  0, "    https://github.com/StuartRiffle/YellowLISP ");
    console->PrintColor(COLOR_WHITE, 0, "(c) 2020 Stuart Riffle\n\n");
}

void PrintOptions(Console* console)
{
    console->Print("Usage: YellowLISP [options] [lisp files]\n\n");

    console->Print("This is an interpreter for a limited but tastefully chosen subset of LISP. It is\n");
    console->Print("a work in progress, and should not be used for production.\n\n");

    console->Print("LISP files have the extension \".lisp\". If any are specified, they will\n"); 
    console->Print("be loaded and evaluated in the order given. YellowLISP will exit after that.\n\n");
    console->Print("Type \"(help)\" at the interactive prompt to see the runtime options.\n");
    console->Print("Type \"(exit)\" (or hit CTRL-C) to end your session.\n\n");

    console->Print("Available command line options are:\n");
    console->Print("  --repl       Drop to an interactive prompt after running any LISP files\n");
    console->Print("  --debug      Run in debug mode (verbose diagnostic output)\n");
    console->Print("  --no-color   Disable colored console output\n");
    console->Print("  --log <file> Mirror output to <file>\n");
    console->Print("  --version    Print just the version and exit\n");
    console->Print("  --help       Display this message and exit\n");
}

int main(int argc, char** argv)
{
    std::unique_ptr<Console> console(new Console());

    CommandLine commandLine(argc, argv);
    InterpreterSettings settings;

    char versionStr[80];
    sprintf(versionStr, "YellowLISP %d.%d.%d", VERSION_MAJOR, VERSION_MINOR, VERSION_PATCH);

    // Handle flags

    if (commandLine.HasFlag("--no-color"))
        console->EnableColor(false);

    if (commandLine.HasFlag("--version"))
    {
        console->Print("%s\n", versionStr);
        return RETURN_SUCCESS;
    }

    if (commandLine.HasFlag("--help"))
    {
        PrintBanner(console.get());
        PrintOptions(console.get());
        return RETURN_SUCCESS;
    }

#if DEBUG_BUILD
    console->EnableDebugOutput(true);
    SanityCheck(console.get());
#endif

    if (commandLine.HasFlag("--repl"))
        settings._repl = true;

    if (commandLine.HasFlag("--debug"))
    {
        console->EnableDebugOutput(true);
        settings._debugMode = true;
    }

    string logFileName;
    if (commandLine.HasParam("--log", logFileName))
        console->LogToFile(logFileName.c_str());

    // Now let's try and LISP something

    Interpreter lisp(console.get(), &settings);

    vector<string> sourceFiles = commandLine.ArgsEndingWith(".lisp");
    if (sourceFiles.size() > 0)
    {
        // Source code was specified, so run it all then exit

        for (string& filename : sourceFiles)
        {
            std::ifstream file(filename);
            if (!file)
            {
                console->PrintErrorPrefix();
                console->Print("file not found: %s\n", filename.c_str());

                return RETURN_PARSING_ERROR;
            }

            std::stringstream ss;
            ss << file.rdbuf();
            string source = ss.str();

            string output = lisp.Evaluate(source);
            console->Print("%s\n", output.c_str());
        }

        if (!settings._repl)
            return RETURN_SUCCESS;
    }

    // Otherwise, drop into the REPL

    PrintBanner(console.get());

    const char* pad = console->IsColor()? " " : "";
    console->PrintColor(COLOR_BLACK, COLOR_YELLOW, "%s%s%s\n", pad, versionStr, pad);

    lisp.RunREPL();

    return RETURN_SUCCESS;
}
