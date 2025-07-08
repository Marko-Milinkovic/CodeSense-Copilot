# CodeSense Copilot Autocomplete Engine

---

## 🚀 Project Overview

**CodeSense Copilot** is an **AI-powered**, console-based autocomplete engine meticulously crafted to revolutionize coding efficiency. Leveraging a robust Trie data structure, sophisticated fuzzy matching algorithms, and adaptive, dynamic frequency-based ranking, it delivers **predictive, intuitively context-aware, and exceptionally relevant** code completion, empowering developers with intelligent, real-time assistance directly within the command line.

This project goes beyond basic autocompletion, offering a rich interactive experience with syntax highlighting and smart input handling, aiming to provide a powerful development environment even in a console setting.

---

## ✨ Features

* **Intelligent Autocomplete:**
    * **Fuzzy Matching:** Provides suggestions even with typos or partial input using an advanced recursive edit-distance algorithm.
    * **Frequency-Based Ranking:** Prioritizes frequently used words and learned selections, making suggestions more relevant and adapting to your coding patterns.
    * **Prefix-Awareness:** Quickly narrows down suggestions based on the current word's prefix.
* **Real-time Console Editor:**
    * Direct manipulation of the console buffer using the Windows API (`<windows.h>`, `_getch()`) for immediate feedback.
    * Seamless single-line input handling with responsive cursor control and backspace functionality.
* **Syntax Highlighting:**
    * Built-in lexer/tokenizer intelligently categorizes input into various token types (keywords, comments, strings, numbers, operators, punctuation, preprocessor directives).
    * Applies distinct colors to different token types for enhanced readability (requires a compatible terminal).
* **Smart Indentation:**
    * Automatic indentation adjustment based on C++ scope delimiters (e.g., `{`).
    * Heuristic-based indentation for common control flow structures (`for`, `while`, `if`, `switch`).
* **Basic Auto-Closing:**
    * Automatically closes double quotes (`"`) and single quotes (`'`) for common string/char literal entry.
* **Persistent Dictionary:**
    * Saves and loads the autocomplete dictionary, including learned word frequencies, to/from a file, ensuring your preferences persist across sessions.

---

## ⚙️ How It Works

At its core, CodeSense Copilot combines several advanced algorithms and system-level interactions:

### 1. Trie Data Structure
The backbone of the autocomplete engine is a highly optimized **Trie (prefix tree)**. Each node in the Trie stores whether it marks the end of a valid word and, critically, a `frequency` count.
* **`Trie::insert(word, freq)`**: Efficiently adds words to the dictionary, initializing or updating their frequency.
* **`Trie::log_selection(word)`**: Dynamically increases the frequency of a word (by `5` units per selection) whenever it's auto-completed or explicitly confirmed by the user, making the engine adapt and learn.

### 2. Advanced Fuzzy Matching
For suggestions beyond exact prefixes, the engine employs a recursive **fuzzy matching** algorithm (a variant of Levenshtein distance).
* **`Trie::search_fuzzy(...)`**: This method intelligently explores the Trie, simulating various edit operations (insertions, deletions, substitutions) to find words within a specified `max_edits` threshold from the user's input. It meticulously tracks the `edits_used` to determine match quality.

### 3. Intelligent Ranking (`FuzzyMatch` & Scoring)
The system prioritizes the most relevant suggestions through a custom scoring mechanism:
* The `FuzzyMatch` struct holds the `word`, its `frequency`, and the `edit_distance` from the input.
* The `get_ranked_fuzzy_matches` function calculates a `score` for each match (e.g., `score = frequency - alpha * edit_distance`), where `alpha` can be tuned to balance the importance of frequency versus exactness.
* A custom `operator<` for `FuzzyMatch` prioritizes exact matches (edit distance 0), then sorts by the calculated `score` (descending), and finally by word lexicographically (ascending) for ties.

### 4. Lexical Analysis (Tokenizer)
The `MainLogicController::tokenize` function acts as a **basic lexer**, breaking down the input line into distinct **tokens** (e.g., `KEYWORD`, `STRING_LITERAL`, `COMMENT`, `OPERATOR`, `PUNCTUATION`, etc.). This enables the console to render text with appropriate syntax highlighting.

### 5. Real-time Console I/O
The `MainLogicController::interactive_loop` uses direct Windows API calls (`GetStdHandle`, `SetConsoleCursorPosition`, `_getch()`) to provide a responsive, character-by-character editing experience. This allows for:
* Instantaneous character input capture.
* Precise cursor positioning.
* Dynamic redrawing of the input line with syntax coloring.
* Automatic handling of indentation and basic auto-closing characters, enhancing the developer's experience.

---

## 🚀 Getting Started

To build and run this project:

1.  **Clone the Repository:**
    ```bash
    git clone [https://github.com/YourUsername/CodeSense-Copilot-Autocomplete-Engine.git](https://github.com/YourUsername/CodeSense-Copilot-Autocomplete-Engine.git)
    cd CodeSense-Copilot-Autocomplete-Engine
    ```
    (Replace `YourUsername` with your actual GitHub username.)

2.  **Open in Visual Studio:**
    * Open the `.sln` (solution) file in Visual Studio.

3.  **Build the Project:**
    * In Visual Studio, go to `Build` > `Build Solution` (or press `Ctrl + Shift + B`).

4.  **Run the Executable:**
    * Go to `Debug` > `Start Without Debugging` (or press `Ctrl + F5`) to run the console application.

---

## 🔮 Future Enhancements

* **Advanced Contextual Awareness:** Implement a more robust parser to understand C++ syntax and provide truly semantic suggestions (e.g., class members after `.` or `->`).
* **Multi-line Editing:** Extend the console editor to support navigation and editing across multiple lines.
* **Cross-Platform Compatibility:** Adapt the console rendering logic to work on Linux/macOS using libraries like NCurses or similar.
* **Configurable Settings:** Allow users to customize `max_edits`, `alpha` values, and highlighting colors.
* **Dynamic Keyword Loading:** Load keywords from external files or C++ standard libraries.

---
