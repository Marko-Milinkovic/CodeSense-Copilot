#pragma once
#include <iostream>
#include <string>
#include <vector>
#include <algorithm> // For std::count
#include "Trie.h"
#include <unordered_set>
#include <conio.h> // for unbuffered input reading
#include "StringHandler.h"
#include <windows.h> // Crucial for HANDLE, COORD, CONSOLE_SCREEN_BUFFER_INFO, etc.
#include <regex>     // For std::regex_search

// ASCII values for special keys
#define KEY_SPACE 32
#define KEY_TAB   9
#define KEY_ENTER 13 // ASCII for Carriage Return (Enter key for _getch())
#define KEY_ESC   27 // Escape key to exit the loop
#define KEY_BACKSPACE 8 // ASCII for Backspace - NOW DEFINED

// Global handle and info for console output (initialized once)
extern HANDLE hConsole;
extern CONSOLE_SCREEN_BUFFER_INFO csbi;
extern COORD g_initial_cursor_pos; // Global for initial cursor position of the prompt

const std::string DICTIONARY_FILE = "dictionary.txt";
const int MAX_EDITS = 5;
const int TOP_K = 1; // previously 30
const double ALPHA = 1.0;

enum class TokenTYPE {
	DEFAULT,
	KEYWORD,
	STRING_LITERAL,
	NUMBER_LITERAL,
	COMMENT,
	OPERATOR,
	PUNCTUATION, // For braces, parentheses, semicolons etc.
	PREPROCESSOR // For #include, #define etc.
};

const WORD COLOR_DEFAULT = FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE; // White
const WORD COLOR_KEYWORD = FOREGROUND_BLUE | FOREGROUND_INTENSITY;              // Bright Blue
const WORD COLOR_STRING = FOREGROUND_GREEN | FOREGROUND_RED;                    // Yellow (for strings, often green in IDEs)
const WORD COLOR_NUMBER = FOREGROUND_RED | FOREGROUND_INTENSITY;                // Bright Red
const WORD COLOR_COMMENT = FOREGROUND_GREEN | FOREGROUND_INTENSITY;             // Bright Green
const WORD COLOR_OPERATOR = FOREGROUND_RED | FOREGROUND_BLUE | FOREGROUND_INTENSITY; // Bright Magenta/Purple
const WORD COLOR_PUNCTUATION = FOREGROUND_BLUE | FOREGROUND_RED;                // Magenta/Purple (less intense)
const WORD COLOR_PREPROCESSOR = FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_INTENSITY; // Bright Cyan

struct Token {
	std::string text;
	TokenTYPE type;
	size_t start_pos; // Starting position of the token in the original string
	size_t length;    // Length of the token
};

class MainLogicController {

public:
	Trie trie;
	void start_program();
	void exit_program();
	void interactive_loop();

private:
	StringHandler string_handler;
	int current_indent_level = 0; // New member variable for indentation
	const int INDENT_SPACES = 4; // Define indentation size (e.g., 4 spaces)

	// Method to initialize the set of C++ keywords
	void initialize_keywords();
	// The tokenizer function: takes a line of code and breaks it into tokens
	std::vector<Token> tokenize(const std::string& line);
	// Set of C++ keywords for quick lookup during tokenization
	std::unordered_set<std::string> keywords;


	void clear_input_area_and_reset_cursor() {
		COORD current_cursor_pos;
		GetConsoleScreenBufferInfo(hConsole, &csbi);
		current_cursor_pos = csbi.dwCursorPosition;

		DWORD charsWritten;
		DWORD conSize = csbi.dwSize.X - current_cursor_pos.X;
		FillConsoleOutputCharacter(hConsole, ' ', conSize, current_cursor_pos, &charsWritten);
		FillConsoleOutputAttribute(hConsole, csbi.wAttributes, conSize, current_cursor_pos, &charsWritten);

		SetConsoleCursorPosition(hConsole, g_initial_cursor_pos);
	};

	void redraw_input_line(const std::string& current_display_buffer); // Declaration will be implemented with indent

	std::string show_suggestions(const std::string& input);
	void prefix_checking(std::vector<FuzzyMatch>& matches, std::unordered_set<std::string>& prefix_words_set);
};