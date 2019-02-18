#ifndef TOOLS_HH
#define TOOLS_HH

#include <string>
#include <vector>

namespace tools
{
	// Converts the specified value to a string to the specified number of
	// decimal places (precision).
	std::string toString(double value, int precision = 4);
	// Removes leading and trailing whitespace from the supplied string.
	std::string trim(const std::string& s);
	// Removes leading and trailing whitespace from the supplied string.
	// NOTE: This method edits the reference rather than returning a
	// value.
	void trim_ref(std::string& s);
	// Splits a string into substrings using the chosen delimiter.
	std::vector<std::string> split(const std::string& s, char delim);
	// Splits a string into substrings using an array of delimiters.
	std::vector<std::string> split(
		const std::string& s,
		const std::vector<char>& delims
	);
}

#endif
