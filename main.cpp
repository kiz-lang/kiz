enum class CmdOption {
    None,
    Run,
    Version,
    //Lsp,
    Help,
    Interact
};

auto main(int argc, char* argv[]) -> int {
    auto opt = CmdOption::None;
    Str path;

    for (int i = 1; i < argc; ++i) {
        const char* arg = argv[i];

        if (strcmp(arg, "--run") == 0 || strcmp(arg, "-r") == 0) {
            opt = CmdOption::Run;
            if (i + 1 < argc)
                path = Str(argv[++i]);
            else
                return 1;
        }
        else if (strcmp(arg, "--version") == 0 || strcmp(arg, "-v") == 0)
            opt = CmdOption::Version;
        else if (strcmp(arg, "--lsp") == 0 || strcmp(arg, "-l") == 0)
            opt = CmdOption::Lsp;
        else if (strcmp(arg, "--help") == 0 || strcmp(arg, "-h") == 0)
            opt = CmdOption::Help;
        else if (strcmp(arg, "--interact") == 0 || strcmp(arg, "-i") == 0)
            opt = CmdOption::Interact;
        else
            return 1;
    }

    switch (opt) {
    case CmdOption::Run:
            break;

    case CmdOption::Version:
        break;

    case CmdOption::Lsp:
        break;

    case CmdOption::Help:
        break;

    case CmdOption::Interact:
        break;

    default:
        break;
    }

    return 0;
}