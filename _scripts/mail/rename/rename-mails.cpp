/*
    rename-mails.cpp

    Simple program to read email files from a directory tree
    and print some header fields.

    Uses the mimetic library for parsing MIME messages.

    No copyright, just do what you want with it.
*/

#include <iostream>
#include <fstream>
#include <filesystem>
#include <mimetic/mimetic.h>
#include <regex>

namespace fs = std::filesystem;
using namespace std;

auto version = "20251115-1";
std::regex emailRegex(R"([a-zA-Z0-9._%+-]+@[a-zA-Z0-9.-]+\.[a-zA-Z]{2,})");
std::smatch match;

/******************************************************************************************************
 * @brief Converts a string to lowercase.
 * @param s Input string
 * @return Lowercase version of the input string
 */
// #include <algorithm>
// #include <string>
// #include <cctype>
std::string to_lowercase(std::string s)
{
    std::transform(s.begin(), s.end(), s.begin(),
                   [](unsigned char c)
                   { return std::tolower(c); });
    return s;
}

/******************************************************************************************************
 * @brief Converts a MIME date header to a UTC string in the format "YYYYMMDD_HHMMSS".
 * @param dateHeader The MIME date header string.
 * @return A string representing the date in "YYYYMMDD_HHMMSS" format, or an empty string if parsing fails.
 *
 */
std::string mimeDateToUTCString(const std::string &dateHeader)
{
    std::string s = dateHeader;

    // // Verwijder prefix "Date:" indien aanwezig
    // if (s.rfind("Date:", 0) == 0)
    //     s = s.substr(5);

    // // Trim spaties
    // while (!s.empty() && isspace(s.front()))
    //     s.erase(0, 1);
    // while (!s.empty() && isspace(s.back()))
    //     s.pop_back();

    // // Tijdzone staat altijd achteraan: ±HHMM (5 chars)
    // if (s.size() < 5)
    //     return "";

    // std::string tz = s.substr(s.size() - 5);
    // s.erase(s.size() - 6); // verwijder spatie + offset

    // Parse hoofddeel van datum
    std::tm tm = {};
    std::istringstream iss(s);

    // Try with weekday
    iss >> std::get_time(&tm, "%a, %d %b %Y %H:%M:%S");
    if (iss.fail())
    {
        // Try without weekday
        iss.clear();
        iss.str(s);
        iss >> std::get_time(&tm, "%d %b %Y %H:%M:%S");
        if (iss.fail())
        {
            return "";
        }
    }

    char buffer[32];
    std::strftime(buffer, sizeof(buffer), "%Y%m%d_%H%M%S", &tm);
    return buffer;
}

/******************************************************************************************************
 * @brief Checks if mime entity has essential headers.
 * @param me Mime entity to check
 * @return true if mime entity has essential headers, false otherwise
 *
 */
bool is_valid_mime(const mimetic::MimeEntity &me)
{
    const mimetic::Header &h = me.header();
    return h.hasField("From") ||
           h.hasField("Date") ||
           h.hasField("Subject") ||
           h.hasField("Message-ID") ||
           h.hasField("To");
}

/******************************************************************************************************
 * @brief Prints 'From' en 'Date' headers.
 * @param path File to check
 */
int process_file(const fs::path &p)
{
    int return_value = 0; // assume invalid file

    ifstream f(p);
    if (!f)
        return return_value; // cannot open file

    std::string from_field = "";
    std::string date_field = "";
    try
    {
        mimetic::MimeEntity me(f); // read only headers

        if (!is_valid_mime(me))
            return return_value; // skip invalid mime files

        cout << "\n FILE: " << p.string() << "\n";

        const mimetic::Header &h = me.header();

        if (h.hasField("From"))
        {
            auto tmp = h.field("From").value();
            if (std::regex_search(tmp, match, emailRegex))
            {
                from_field = to_lowercase(match[0]);
            }
        }

        if (h.hasField("Date"))
        {
            auto tmp = mimeDateToUTCString(h.field("Date").value());
            if (!tmp.empty())
            {
                date_field = tmp;
            }
        }

        if (from_field.empty() || date_field.empty())
        {
            cout << "    [SKIP]  Missing From or Date field]\n";
            return return_value;
        }

        try
        {
            fs::path new_filename = p.parent_path() / (date_field + "---[FROM]_" + from_field + ".eml");
            cout << "    Renaming to: " << new_filename.string() << "\n";
            fs::rename(p, new_filename);
            if (fs::exists(new_filename))
            {
                std::cout << "    [SUCCES]  File renamed succesfully\n";
                return_value = 1; // successful processed file
            }
            else
                std::cout << "    [FAIL]  File renaming failed\n";
        }
        catch (const fs::filesystem_error &e)
        {
            std::cerr << "    [EXCEPTION]   " << e.what() << '\n';
        }
    }
    catch (const std::exception &e)
    {
        cerr << "[Fatal]  Parse error in " << p << ": " << e.what() << "\n";
    }

    return return_value;
}


/******************************************************************************************************
 * @brief Main program entry point.
 */
int main(int argc, char *argv[])
{
    int files_total = 0;
    int files_valid = 0;

    // --- check arguments ---
    if (argc != 2)
    {
        cerr << "[Fatal]  Use: " << argv[0] << " <start directory>\n";
        return 1;
    }

    fs::path startdir(argv[1]);

    if (!fs::exists(startdir))
    {
        cerr << "[Fatal]  Path does not exist: " << startdir << "\n";
        return 1;
    }

    if (!fs::is_directory(startdir))
    {
        cerr << "[Fatal]  Path is not a directory: " << startdir << "\n";
        return 1;
    }

    cout << "\n    _________________________ Rename Mails, v" << version << " _________________________\n\n\n";

    // --- Directory doorlopen ---
    for (const auto &entry : fs::recursive_directory_iterator(startdir))
    {
        if (entry.is_regular_file())
        {
            files_total++;
            files_valid += process_file(entry.path());
        }
    }

    return 0;
}

// g++ -std=c++17 -Wall -O2 rename-mails.cpp -o rename-mails-cpp  -lmimetic