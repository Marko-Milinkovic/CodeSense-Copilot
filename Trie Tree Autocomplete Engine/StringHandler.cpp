#include "StringHandler.h"

std::string StringHandler::getSuffixDifference(const std::string& s1, const std::string& s2)
{
    // 1. Check if s1 is a prefix of s2
    //    s2.rfind(s1, 0) checks if s1 appears at the beginning (index 0) of s2.
    //    It returns 0 if s1 is a prefix, and std::string::npos otherwise.
    if (s2.rfind(s1, 0) == 0) {
        // 2. If s1 is a prefix, extract the substring from s2
        //    starting after the length of s1.
        return s2.substr(s1.length());
    }
    else {
        // 3. Handle cases where s1 is not a prefix of s2
        //    You can return an empty string, throw an exception, or return a special indicator.
        //    Returning an empty string is common for "no meaningful difference" in this context.
        return "";
    }
}
