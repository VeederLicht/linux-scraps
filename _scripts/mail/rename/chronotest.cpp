#include <iostream>
#include <string>
#include <sstream>
#include <stdexcept>
#include <iomanip>  // For std::get_time
#include <ctime>    // For struct tm, timegm
#include <clocale>  // For std::setlocale
#include <cstdlib>  // For std::strtol

/**
 * @brief Converts a MIME-style date string to a Linux timestamp
 * using C-style functions (no <chrono>).
 *
 * @note This solution is LESS PORTABLE than the C++20 <chrono> version.
 * It relies on:
 * 1. `std::get_time` (C++11) for parsing.
 * 2. `timegm` (a POSIX/GNU extension, not standard C++) to convert
 * a UTC struct tm to a time_t. This is standard on Linux.
 *
 * @param date_string The date string, e.g., "Wed, 21 Oct 2015 07:28:00 -0700"
 * @return The Linux timestamp (seconds since Unix epoch) as a 64-bit integer.
 * @throws std::runtime_error if parsing fails.
 */
long long mimeDateToTimestamp(const std::string& date_string) {
    // Set locale to "C" to ensure "Oct" and "Wed" are parsed correctly.
    std::setlocale(LC_TIME, "C");

    struct tm t = {}; // Zero-initialize the struct
    std::stringstream ss(date_string);

    // Format for the date/time part AND the offset
    const char* format = "%a, %d %b %Y %H:%M:%S %z";

    // Use std::get_time (C++11) to parse the string into the struct tm
    ss >> std::get_time(&t, format);

    if (ss.fail()) {
        throw std::runtime_error("Failed to parse date string: " + date_string);
    }
    
    //
    // --- The Problem: Applying the Offset ---
    //
    // `std::get_time` parses the time (07:28:00) into t.tm_hour, etc.
    // It *parses* the offset ("-0700") but does NOT apply it to the
    // other fields. We must handle the offset manually.
    //
    // The C++11 standard does not provide a standard way to get the
    // parsed offset value.
    //
    // We will re-parse the offset part manually.
    //

    // 1. Manually find and parse the offset
    size_t offset_pos = date_string.find_last_of(' ');
    if (offset_pos == std::string::npos) {
         throw std::runtime_error("Could not find offset in date string.");
    }
    std::string offset_part = date_string.substr(offset_pos + 1);
    
    // Convert string offset (e.g., "-0700") to an integer
    long offset_val = std::strtol(offset_part.c_str(), nullptr, 10);

    // Convert integer offset (e.g., -700) to seconds
    int offset_hours = static_cast<int>(offset_val / 100);
    int offset_mins = static_cast<int>(offset_val % 100);
    long offset_seconds = (offset_hours * 3600) + (offset_mins * 60);

    // 2. Convert the parsed time (struct tm) to a timestamp
    // `timegm` is the key function. It's a Linux/GNU extension
    // that converts a `struct tm` *assuming it is already in UTC*
    // to a time_t (Unix timestamp).
    //
    // `t` currently holds 07:28:00. `timegm(&t)` will create a timestamp
    // for "07:28:00 UTC".
    time_t timestamp_as_utc = timegm(&t);

    // 3. Apply the offset.
    // The actual time is "07:28:00 at -0700", which is "14:28:00 UTC".
    // Our timestamp is for "07:28:00 UTC".
    // To get "14:28:00 UTC", we must *subtract* the offset.
    //
    // (Timestamp for 07:28:00 UTC) - (-25200 seconds)
    //  = (Timestamp for 07:28:00 UTC) + 25200 seconds
    //  = (Timestamp for 14:28:00 UTC)
    //
    return timestamp_as_utc - offset_seconds;
}

int main() {
    // Example from RFC 5322
    std::string mime_date_1 = "Wed, 21 Oct 2015 07:28:00 -0700";
    // Example with a positive offset
    std::string mime_date_2 = "Sun, 16 Nov 2025 14:30:00 +0100";
    // Example with UTC (GMT)
    std::string mime_date_3 = "Fri, 25 Jul 2003 12:00:00 +0000";

    try {
        long long ts1 = mimeDateToTimestamp(mime_date_1);
        std::cout << "'" << mime_date_1 << "' -> " << ts1 << std::endl;
        // Expected: 1445437680

        long long ts2 = mimeDateToTimestamp(mime_date_2);
        std::cout << "'" << mime_date_2 << "' -> " << ts2 << std::endl;
        // Expected: 1763367000

        long long ts3 = mimeDateToTimestamp(mime_date_3);
        std::cout << "'" << mime_date_3 << "' -> " << ts3 << std::endl;
        // Expected: 1059134400

    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}

//   g++ -std=c++11 -D_GNU_SOURCE -o chronotest chronotest.cpp