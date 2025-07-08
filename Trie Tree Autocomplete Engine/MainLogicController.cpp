#include "MainLogicController.h"
#include <windows.h> // Include windows.h here as well for GetStdHandle etc.

// GLOBAL DEFINITIONS (allocate memory for them here, and ONLY here)
HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
CONSOLE_SCREEN_BUFFER_INFO csbi;
COORD g_initial_cursor_pos; // Define the global variable

// StringHandler::getSuffixDifference (assuming it's defined in StringHandler.cpp)
// You might need to include StringHandler.cpp here or link it properly.
// For demonstration, I'll put a dummy definition here if StringHandler.h doesn't define it inline.
/*
// Dummy getSuffixDifference if StringHandler is not fully implemented yet
std::string StringHandler::getSuffixDifference(const std::string& s1, const std::string& s2) {
    if (s2.rfind(s1, 0) == 0) {
        return s2.substr(s1.length());
    }
    return "";
}
*/


void MainLogicController::start_program()
{
    this->trie.load_from_file(DICTIONARY_FILE);
    this->initialize_keywords();
    std::cout << "C++ Autocomplete Console\n";
    // For a cleaner UI, you might remove this line entirely from initial startup
    // std::cout << "Type. Press ' ' for new word, 'TAB' to autocomplete, 'Enter' to confirm, 'ESC' to quit.\n";
}

void MainLogicController::exit_program()
{
    trie.save_to_file(DICTIONARY_FILE);
    std::cout << "Dictionary saved. Goodbye!\n"; // Keep this for graceful exit message
}

void MainLogicController::interactive_loop()
{
    std::string complete_input_buffer; // Stores the entire line typed by the user
    std::string current_word_buffer;  // Stores the current word being typed (clears on space)
    int ch;

    // Get initial cursor position where "Input: " prompt starts
    GetConsoleScreenBufferInfo(hConsole, &csbi);
    g_initial_cursor_pos = csbi.dwCursorPosition;

    redraw_input_line(""); // Initial prompt display

    while (true) {
        ch = _getch(); // Read character without echoing and without waiting for Enter

        // Handle special keys
        if (ch == KEY_ESC) {
            // Clear the line before exiting message
            clear_input_area_and_reset_cursor(); // Use the more precise clear
            std::cout << "Program exited\n";
            break; // Exit the loop
        }
        else if (ch == KEY_ENTER) {
            // First, process the current line's content for potential indent changes
            if (!complete_input_buffer.empty()) {
                std::string trimmed_buffer = complete_input_buffer;
                // Remove trailing whitespace for better matching
                trimmed_buffer.erase(trimmed_buffer.find_last_not_of(" \t\n\r") + 1);

                // Simple brace-based indent increment
                if (trimmed_buffer.back() == '{') {
                    current_indent_level++;
                }
                // Heuristic for loop/conditional start (e.g., "for (int i = 0;" or "if (condition)")
                else if (std::regex_search(trimmed_buffer, std::regex(R"((for|while|if|switch)\s*\(.*)", std::regex_constants::icase))) {
                    current_indent_level++;
                }
            }

            std::cout << "\n"; // Move to next line

            // Clear buffers for the new line
            complete_input_buffer.clear();
            current_word_buffer.clear();

            // Update g_initial_cursor_pos for the *new* line
            GetConsoleScreenBufferInfo(hConsole, &csbi);
            g_initial_cursor_pos = csbi.dwCursorPosition;

            redraw_input_line(""); // Redraw the new empty prompt with new indentation
        }
        else if (ch == KEY_BACKSPACE) {
            if (!complete_input_buffer.empty()) {
                char last_char = complete_input_buffer.back();
                complete_input_buffer.pop_back();

                // If we backspace a brace, adjust indent level
                if (last_char == '{' && current_indent_level > 0) {
                    current_indent_level--;
                }
                else if (last_char == '}' && current_indent_level > 0) { // If backspacing a '}' from its dedented position
                    // This logic is for visual consistency if '}' caused a dedent but was then backspaced
                    // It can be complex; a simpler heuristic: if the char before '}' was a space/tab
                    // that was part of the original indent, then it might be valid to re-increment.
                    // For now, if we backspace a closing brace, and the current line would naturally be at a higher indent
                    // due to previous actions (e.g., typing `{` on prior line, pressing enter, then typing `}` immediately)
                    // then we might need to re-increment. This is context-dependent.
                    // For simplicity, for now, we only decrement on '{' backspace.
                    // A more robust solution would be to look at the line content before backspace.
                }

                // Handle backspacing into an auto-closed character pair
                if (complete_input_buffer.length() >= current_word_buffer.length() + 1) { // Check if there's enough room to pop a char
                    char char_before_last = complete_input_buffer.back();
                    if ((last_char == ')' && char_before_last == '(') ||
                        (last_char == '}' && char_before_last == '{') ||
                        (last_char == ']' && char_before_last == '[') ||
                        (last_char == '"' && char_before_last == '"'))
                    {
                        complete_input_buffer.pop_back(); // Pop the opening character too
                    }
                }

                if (!current_word_buffer.empty()) {
                    current_word_buffer.pop_back();
                }
                redraw_input_line(complete_input_buffer);
            }
            else {
                redraw_input_line(complete_input_buffer); // Ensure cursor is correct even if empty
            }
        }
        else if (ch == KEY_SPACE) {
            complete_input_buffer.push_back(static_cast<char>(ch));
            current_word_buffer.clear(); // A space always starts a new word for autocomplete purposes
            redraw_input_line(complete_input_buffer);
        }
        else if (ch == KEY_TAB) {
            // Attempt autocomplete for current_word_buffer
            std::string suggested_word = this->show_suggestions(current_word_buffer);
            if (!suggested_word.empty()) {
                std::string suffix = string_handler.getSuffixDifference(current_word_buffer, suggested_word);
                complete_input_buffer.append(suffix);
                current_word_buffer = suggested_word; // Update current_word_buffer to full suggestion
                redraw_input_line(complete_input_buffer);
            }
            else {
                // If no suggestion, insert actual tab spaces (as defined by INDENT_SPACES)
                for (int i = 0; i < INDENT_SPACES; ++i) {
                    complete_input_buffer.push_back(' ');
                }
                current_word_buffer.clear(); // Tab can also signify end of current word for autocomplete context
                redraw_input_line(complete_input_buffer);
            }
        }
        else if (ch == 0 || ch == 224) {
            // Extended keys (e.g., arrow keys). Consume the second byte.
            _getch();
            redraw_input_line(complete_input_buffer); // Redraw just in case, no change to buffer
        }
        else {
            // Regular character input
            char char_typed = static_cast<char>(ch);
            complete_input_buffer.push_back(char_typed); // Push the character first

            // Adjust indent if a '}' is typed, and it's the first non-whitespace character on the line
            if (char_typed == '}') {
                std::string current_line_content_no_prompt_or_indent = complete_input_buffer;
                // Remove leading whitespace (which would be from the indent)
                size_t first_non_ws = current_line_content_no_prompt_or_indent.find_first_not_of(" \t");
                if (first_non_ws != std::string::npos) {
                    current_line_content_no_prompt_or_indent = current_line_content_no_prompt_or_indent.substr(first_non_ws);
                }
                else {
                    current_line_content_no_prompt_or_indent = ""; // Line is all whitespace
                }

                // If after removing initial whitespace, '}' is the first character, then dedent
                if (!current_line_content_no_prompt_or_indent.empty() && current_line_content_no_prompt_or_indent[0] == '}') {
                    if (current_indent_level > 0) {
                        current_indent_level--; // Dedent for closing brace
                    }
                }
            }
            bool auto_closed = false;
            /*
            // Auto-closing Braces/Quotes
            
            if (char_typed == '(') {
                complete_input_buffer.push_back(')');
                auto_closed = true;
            }
            else if (char_typed == '{') {
                complete_input_buffer.push_back('}');
                auto_closed = true;
            }
            else if (char_typed == '[') {
                complete_input_buffer.push_back(']');
                auto_closed = true;
            }
            */
            if (char_typed == '"') {
                // Only auto-close if it's not closing an existing quote.
                // This is a simple heuristic: count quotes in the current line.
                // If it's odd, it means we just typed an opening quote.
                if (std::count(complete_input_buffer.begin(), complete_input_buffer.end(), '"') % 2 != 0) {
                    complete_input_buffer.push_back('"');
                    auto_closed = true;
                }
            }
            else if (char_typed == '\'') {
                // Single quotes: similar logic to double quotes
                if (std::count(complete_input_buffer.begin(), complete_input_buffer.end(), '\'') % 2 != 0) {
                    complete_input_buffer.push_back('\'');
                    auto_closed = true;
                }
            }

            current_word_buffer.push_back(char_typed); // Push the actual character typed into current_word_buffer

            redraw_input_line(complete_input_buffer); // Redraw to show the new characters

            // If auto-closed, move cursor back one position
            if (auto_closed) {
                COORD current_cursor_pos;
                GetConsoleScreenBufferInfo(hConsole, &csbi);
                current_cursor_pos = csbi.dwCursorPosition;
                current_cursor_pos.X--; // Move cursor back one character
                SetConsoleCursorPosition(hConsole, current_cursor_pos);
            }
        }
    } // End of while(true)
} // End of interactive_loop()

void MainLogicController::initialize_keywords()
{
    keywords = {
    "alignas", "alignof", "and", "and_eq", "asm", "auto",
    "bool", "break",
    "case", "catch", "char", "char8_t", "char16_t", "char32_t", "class", "concept", "const", "consteval", "constexpr", "const_cast", "continue", "co_await", "co_returns", "co_yield",
    "decltype", "default", "delete", "do", "double", "dynamic_cast",
    "else", "enum", "explicit", "export", "extern",
    "false", "float", "for", "friend",
    "goto",
    "if", "inline", "int",
    "long",
    "mutable",
    "namespace", "new", "noexcept", "nullptr",
    "operator", "or", "or_eq",
    "private", "protected", "public",
    "reflexpr", "register", "reinterpret_cast", "requires", "return",
    "short", "signed", "sizeof", "static", "static_assert", "static_cast", "struct",
    "switch", "synchronized",
    "template", "this", "thread_local", "throw", "true", "try", "typedef", "typeid", "typename",
    "union", "unsigned", "using",
    "virtual", "void", "volatile",
    "wchar_t", "while",
    "xor", "xor_eq"
    };
    // Add common preprocessor directives, though they are usually handled by a preprocessor phase
    // You could also parse them as a special type if needed.
    // For now, we'll include common ones in the keywords set if we want to highlight them.
    keywords.insert("#include");
    keywords.insert("#define");
    keywords.insert("#ifdef");
    keywords.insert("#ifndef");
    keywords.insert("#endif");
    keywords.insert("#pragma");
}

std::vector<Token> MainLogicController::tokenize(const std::string& line)
{
    std::vector<Token> tokens;
    size_t current_pos = 0;
    size_t line_length = line.length();

    while (current_pos < line_length) {
        char c = line[current_pos];

        // 1. Handle Whitespace
        if (isspace(c)) {
            size_t start = current_pos;
            while (current_pos < line_length && isspace(line[current_pos])) {
                current_pos++;
            }
            tokens.push_back({ line.substr(start, current_pos - start), TokenTYPE::DEFAULT, start, current_pos - start });
            continue;
        }

        // 2. Handle Comments
        if (c == '/') {
            if (current_pos + 1 < line_length) {
                // Single-line comment //
                if (line[current_pos + 1] == '/') {
                    size_t start = current_pos;
                    while (current_pos < line_length && line[current_pos] != '\n' && line[current_pos] != '\r') {
                        current_pos++;
                    }
                    tokens.push_back({ line.substr(start, current_pos - start), TokenTYPE::COMMENT, start, current_pos - start });
                    continue; // Continue from current_pos (which is now at or past the newline)
                }
                // Multi-line comment /* */
                else if (line[current_pos + 1] == '*') {
                    size_t start = current_pos;
                    current_pos += 2; // Move past /*
                    while (current_pos + 1 < line_length && !(line[current_pos] == '*' && line[current_pos + 1] == '/')) {
                        current_pos++;
                    }
                    if (current_pos + 1 < line_length) { // Found closing */
                        current_pos += 2; // Move past */
                    }
                    else {
                        // Unclosed multi-line comment, consume till end of line
                        current_pos = line_length;
                    }
                    tokens.push_back({ line.substr(start, current_pos - start), TokenTYPE::COMMENT, start, current_pos - start });
                    continue;
                }
            }
        }

        // 3. Handle String Literals
        if (c == '"' || c == '\'') {
            char quote_char = c;
            size_t start = current_pos;
            current_pos++; // Move past opening quote

            while (current_pos < line_length && line[current_pos] != quote_char) {
                // Handle escaped quotes within string literals
                if (line[current_pos] == '\\' && current_pos + 1 < line_length) {
                    current_pos++; // Skip the escaped character
                }
                current_pos++;
            }

            if (current_pos < line_length) { // Found closing quote
                current_pos++; // Move past closing quote
            }
            else {
                // Unclosed string literal, consume till end of line
                current_pos = line_length;
            }
            tokens.push_back({ line.substr(start, current_pos - start), TokenTYPE::STRING_LITERAL, start, current_pos - start });
            continue;
        }

        // 4. Handle Identifiers and Keywords (unchanged from previous step)
        if (isalpha(c) || c == '_') {
            size_t start = current_pos;
            while (current_pos < line_length && (isalnum(line[current_pos]) || line[current_pos] == '_')) {
                current_pos++;
            }
            std::string word = line.substr(start, current_pos - start);
            if (keywords.count(word)) {
                // Check for preprocessor directives (start with '#')
                if (!word.empty() && word[0] == '#') {
                    tokens.push_back({ word, TokenTYPE::PREPROCESSOR, start, word.length() });
                }
                else {
                    tokens.push_back({ word, TokenTYPE::KEYWORD, start, word.length() });
                }
            }
            else {
                tokens.push_back({ word, TokenTYPE::DEFAULT, start, word.length() }); // Using DEFAULT for general identifiers
            }
            continue;
        }

        // 5. Handle Numbers (unchanged from previous step)
        if (isdigit(c)) {
            size_t start = current_pos;
            while (current_pos < line_length && isdigit(line[current_pos])) {
                current_pos++;
            }
            tokens.push_back({ line.substr(start, current_pos - start), TokenTYPE::NUMBER_LITERAL, start, current_pos - start });
            continue;
        }

        // 6. Handle Multi-character Operators (Add these first, before single-char ops)
        // Add more multi-character operators here as needed
        if (current_pos + 1 < line_length) {
            std::string two_char_op = line.substr(current_pos, 2);
            if (two_char_op == "==" || two_char_op == "!=" || two_char_op == "<=" ||
                two_char_op == ">=" || two_char_op == "&&" || two_char_op == "||" ||
                two_char_op == "++" || two_char_op == "--" || two_char_op == "->" ||
                two_char_op == "::" || two_char_op == "+=" || two_char_op == "-=" ||
                two_char_op == "*=" || two_char_op == "/=" || two_char_op == "%=" ||
                two_char_op == "&=" || two_char_op == "|=" || two_char_op == "^=" ||
                two_char_op == "<<" || two_char_op == ">>") {
                tokens.push_back({ two_char_op, TokenTYPE::OPERATOR, current_pos, 2 });
                current_pos += 2;
                continue;
            }
            // Add three-character operators here if any (e.g., "...")
        }


        // 7. Handle Single-character Operators and Punctuation (expanded list)
        if (std::string("+-*/%&|^~!=<>(){}[];:,.").find(c) != std::string::npos) {
            tokens.push_back({ std::string(1, c), TokenTYPE::PUNCTUATION, current_pos, 1 }); // Using PUNCTUATION for these
            current_pos++;
            continue;
        }

        // If no rule matches, just treat as default and advance (should be rare for C++)
        tokens.push_back({ std::string(1, c), TokenTYPE::DEFAULT, current_pos, 1 });
        current_pos++;
    }
    return tokens;
}

// show_suggestions and prefix_checking remain the same as your last version
// (with the change to show_suggestions to not print "No suggestions found.")

void MainLogicController::redraw_input_line(const std::string& current_display_buffer)
{
    // 1. Move cursor to the beginning of the line where the prompt started
    SetConsoleCursorPosition(hConsole, g_initial_cursor_pos);

    // 2. Get current console info to calculate area to clear
    GetConsoleScreenBufferInfo(hConsole, &csbi);
    DWORD charsWritten;

    // 3. Clear the *entire* line from the prompt's start position to the end of the console line
    // Reset color to default before clearing, to ensure the cleared space is default color
    SetConsoleTextAttribute(hConsole, COLOR_DEFAULT);
    DWORD conSize = csbi.dwSize.X - g_initial_cursor_pos.X;
    FillConsoleOutputCharacter(hConsole, ' ', conSize, g_initial_cursor_pos, &charsWritten);
    FillConsoleOutputAttribute(hConsole, COLOR_DEFAULT, conSize, g_initial_cursor_pos, &charsWritten);

    // 4. Move cursor back to the prompt start to print the new line
    SetConsoleCursorPosition(hConsole, g_initial_cursor_pos);

    // 5. Print the "Input: " prompt with default color
    SetConsoleTextAttribute(hConsole, COLOR_DEFAULT);
    //std::cout << "Input: ";

    // 6. Apply current indentation
    for (int i = 0; i < current_indent_level; ++i) {
        for (int j = 0; j < INDENT_SPACES; ++j) {
            std::cout << " "; // Print spaces for indentation (these also use default color)
        }
    }

    // 7. Tokenize the input buffer for highlighting
    std::vector<Token> tokens = tokenize(current_display_buffer);

    // 8. Iterate through tokens and print with appropriate colors
    for (const auto& token : tokens) {
        WORD color_attribute = COLOR_DEFAULT; // Default to white

        switch (token.type) {
        case TokenTYPE::KEYWORD:
            color_attribute = COLOR_KEYWORD;
            break;
        case TokenTYPE::STRING_LITERAL:
            color_attribute = COLOR_STRING;
            break;
        case TokenTYPE::NUMBER_LITERAL:
            color_attribute = COLOR_NUMBER;
            break;
        case TokenTYPE::COMMENT:
            color_attribute = COLOR_COMMENT;
            break;
        case TokenTYPE::OPERATOR:
            color_attribute = COLOR_OPERATOR;
            break;
        case TokenTYPE::PUNCTUATION:
            color_attribute = COLOR_PUNCTUATION;
            break;
        case TokenTYPE::PREPROCESSOR:
            color_attribute = COLOR_PREPROCESSOR;
            break;
        case TokenTYPE::DEFAULT:
            // Fall through to use COLOR_DEFAULT (white)
        default:
            color_attribute = COLOR_DEFAULT;
            break;
        }

        SetConsoleTextAttribute(hConsole, color_attribute); // Set the color for this token
        std::cout << token.text;                            // Print the token's text
    }

    // 9. Reset color to default after printing the entire line
    SetConsoleTextAttribute(hConsole, COLOR_DEFAULT);

    // 10. Ensure cursor is at the end of current_display_buffer + prompt + indentation
    // This is typically handled by std::cout, but explicit setting can prevent issues.
    COORD final_cursor_pos;
    final_cursor_pos.X = g_initial_cursor_pos.X + (current_indent_level * INDENT_SPACES) + current_display_buffer.length();
    final_cursor_pos.Y = g_initial_cursor_pos.Y;
    SetConsoleCursorPosition(hConsole, final_cursor_pos);
}

std::string MainLogicController::show_suggestions(const std::string& input)
{
    std::vector<FuzzyMatch> matches = trie.get_top_k_fuzzy_matches(input, MAX_EDITS, TOP_K);
    std::vector<std::string> words_with_prefix = trie.get_top_k_with_prefix(input, TOP_K);

    std::unordered_set<std::string> prefix_words_set(words_with_prefix.begin(), words_with_prefix.end());

    this->prefix_checking(matches, prefix_words_set);

    if (matches.empty()) {
        return ""; // No suggestions found, return empty string without printing anything.
    }

    return matches[0].word;
}

void MainLogicController::prefix_checking(std::vector<FuzzyMatch>& matches, std::unordered_set<std::string>& prefix_words_set)
{
    for (int i = 0; i < matches.size(); /* no i++ here */) {
        bool found_in_prefix = false;
        if (prefix_words_set.count(matches[i].word)) {
            found_in_prefix = true;
            matches[i].score = matches[i].score + 200; // Boost score for prefix matches
        }
        if (!found_in_prefix) {
            matches.erase(matches.begin() + i);
        }
        else {
            i++;
        }
    }
}