#include "tools.hh"
#include <regex>
#include <sstream>
#include <string>
#include <vector>
#include <iomanip>
#include <iostream>

namespace tools
{
	std::string toString(double value, int precision)
	{
		// Open a stream to write the value to.
		std::stringstream stream;
		// Set decimal style to fixed.
		stream.flags(std::ios_base::fixed);
		// Set the double precision.
		stream.precision(precision);
		// Add the value into the stream.
		stream << value;
		// Return the string value of the stream.
		return stream.str();
	}

	std::string trim(const std::string& s)
	{
		// Create output string.
		std::string output = s;
		trim_ref(output);
		return output;
	}

	void trim_ref(std::string& s)
	{
		// Create a regular expression to match.
		std::regex r("\\s*(.*?)\\s*");
		// Create an array to hold matches.
		std::smatch matches;
		// Execute the regular expression on the input string.
		if (std::regex_match(s, matches, r))
		{
			s = matches[1];
		}
	}

	std::vector<std::string> split(const std::string& s, const char delim)
	{
		return split(s, std::vector<char>({ delim }));
	}

	std::vector<std::string> split(const std::string& s, const std::vector<char>& delims)
	{
		// Setup vector to hold the split strings.
		std::vector<std::string> output;

		// Combine the delimiters into a single string.
		std::string d;
		for (const char& delim : delims) d += delim;

		// Create regular expression.
		std::regex r("([" + d + "]*)([^" + d + "]+)(?:.*|[\n\r]*)*");
		// Setup vector to hold matches.
		std::smatch matches;
		// Offset the begin iterator by this each time.
		int offset = 0;

		// Continuously find matches (NOTE: technically infinite).
		while (std::regex_match(s.cbegin() + offset, s.cend(), matches, r))
		{
			// Add the non-delimiter match to the output vector.
			output.push_back(matches[2]);
			// Add the length of the matched groups to the offset.
			offset += matches[1].length() + matches[2].length();
			// If offset has overflown, break the loop.
			if (offset >= s.length()) break;
		}

		// Return the output vector.
		return output;
	}
}
