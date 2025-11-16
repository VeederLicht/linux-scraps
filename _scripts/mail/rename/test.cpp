#include <iostream>
#include <filesystem>
namespace fs = std::filesystem;

int main(int argc, char *argv[])
{

// --- check arguments ---
    if (argc != 2)
    {
        std::cerr << "[Fatal]  Use: " << argv[0] << " <start directory>\n";
        return 1;
    }

    fs::path entry(argv[1]);

    if (!fs::exists(entry))
    {
        std::cerr << "[Fatal]  Path does not exist: " << entry << "\n";
        return 1;
    }

    // for (const auto& entry : fs::recursive_directory_iterator(startdir)) {
        // relatieve pad t.o.v. startdir
        // fs::path relative = fs::relative(entry.path(), startdir);


        std::cout << entry.parent_path() << "\n";
    // }
}
