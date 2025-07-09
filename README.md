# CodeSense Copilot [![Made with C++](https://img.shields.io/badge/Made%20with-C%2B%2B-blue.svg)](https://isocpp.org/)

## Table of Contents
* [Project Overview](#project-overview)
* [Features](#features)
* [How It Works: Algorithmic Depth](#how-it-works-algorithmic-depth)
    * [1. The Trie Data Structure](#1-the-trie-data-structure-foundation-of-prefix-search)
    * [2. Adaptive Learning and Frequency-Based Personalization](#2-adaptive-learning-and-frequency-based-personalization)
    * [3. Advanced Fuzzy Matching Algorithm](#3-advanced-fuzzy-matching-algorithm-resilient-suggestions)
    * [4. Comprehensive Scoring and Ranking Mechanisms](#4-comprehensive-scoring-and-ranking-mechanisms)
    * [5. Persistent Dictionary Storage](#5-persistent-dictionary-storage)
* [Getting Started](#getting-started)
* [Future Enhancements](#future-enhancements)
* [Contributing](#contributing)
* [License](#license)

---

## Project Overview

**CodeSense Copilot** is an **AI-powered**, console-based autocomplete engine meticulously crafted to revolutionize coding efficiency. Leveraging a robust Trie data structure, sophisticated fuzzy matching algorithms, and adaptive, dynamic frequency-based ranking, it delivers **predictive, intuitively context-aware, and exceptionally relevant** code completion, empowering developers with intelligent, real-time assistance directly within the command line.

This project goes beyond basic autocompletion, offering a rich interactive experience with syntax highlighting and smart input handling, aiming to provide a powerful development environment even in a console setting.

---

## Features

* **Intelligent Autocomplete:**
    * **Fuzzy Matching and Typo Tolerance:** Employs a sophisticated recursive **edit-distance algorithm** (e.g., Levenshtein distance) to identify the closest dictionary matches, even with typo errors or parial input. 
    * **Adaptive Learning & Personalization:** Prioritizes frequently used words and learned selections, making suggestions more relevant and adapting to user's coding patterns.
    * **Prefix-Awareness:** Quickly narrows down suggestions based on the current word's prefix.
    * **Balanced Scoring for Relevance:** A custom scoring algorithm intelligently combines the word's learned frequency with its edit distance from the input ensuring that suggestions are optimally ranked, prioritizing highly relevant and frequently used terms while still offering accurate matches even with minor discrepancies.
    * **Dictionary Security**: Persistent dictionary that saves learned word frequencies.
      
* **Systems Programming:**
    * **Direct Console Interaction:** Leverages **Windows API functions** (`<windows.h>`, `_getch()`) for direct manipulation of the console buffer, enabling immediate and highly responsive user feedback.
    * **Low-Level Console Control:** Implements precise cursor positioning and manipulation, providing fine-grained control over the console display for an enhanced text editing experience.
    * **Real-time Character-by-Character Input:** Processes input in real-time, handling each character individually for instantaneous response and dynamic display updates without buffering delays.
    * **Efficient Memory Management:** Manages memory for persistent data structure.
      
* **Compiler/Language Theory:**
    * **Syntax Highlighting & Token Classification:** Implements a built-in lexer/tokenizer to intelligently categorize input into various token types (e.g., keywords, comments, strings) and applies distinct colors for enhanced readability within compatible terminals.
    * **Smart Indentation:** Provides automatic indentation adjustment based on C++ scope delimiters (e.g., {}) and employs heuristic-based indentation for common control flow structures (for, while, if, switch) to ensure consistent code formatting.
    * **Basic Auto-Closing:** Intuitively auto-closes common character pairs like double quotes (") and single quotes ('), streamlining string and character literal entry.
   
---

## How It Works: Algorithmic Depth

The CodeSense Copilot's core functionality is powered by an interplay of an optimized Trie (Prefix) tree data structure, a recursive fuzzy matching algorithm and a dynamic, adaptive ranking system, all underpinned by careful memory management and persistent storage.

### 1. The Trie Data Structure: Foundation of Prefix Search

The backbone of the autocomplete engine is an **optimized Trie (prefix tree)**, designed for efficient storage and retrieval of words based on their prefixes.

* **Node Architecture:** Each `TrieNode` is a lightweight structure comprising:
    * `std::array<std::unique_ptr<TrieNode>, ALPHABET_SIZE>`: An array of `std::unique_ptr`s representing child nodes for each character in the alphabet (typically 'a'-'z'). The use of `std::unique_ptr` ensures automatic memory management and avoids manual deallocation, mitigating memory leaks.
    * `bool is_end_of_word`: A flag indicating if the path leading to this node represents a complete, valid word in the dictionary.
    * `int frequency`: An integer counter storing the usage frequency of the word ending at this node, crucial for adaptive ranking.
* **Word Insertion (`Trie::insert`)**: Words are inserted character by character. For each character, the Trie is traversed, and new `TrieNode`s are dynamically created via `std::make_unique` if a path does not exist. Upon reaching the end of a word, `is_end_of_word` is set to `true`, and the `frequency` is initialized or updated. Only lowercase characters are processed.
* **Efficient Prefix Retrieval (`Trie::get_words_with_prefix` & `Trie::dfs`)**: Locating all words sharing a given prefix involves traversing the Trie down to the node corresponding to that prefix. From this prefix node, a **Depth-First Search (DFS)** (`Trie::dfs`) is initiated to traverse all descendant paths. Each path concluding at an `is_end_of_word` node is collected along with its associated frequency, enabling rapid retrieval of all relevant prefix matches.

### 2. Adaptive Learning and Frequency-Based Personalization

The system exhibits a direct form of learning and adaptation, continuously enhancing the relevance of its suggestions based on user interaction.

* **Usage Logging (`Trie::log_selection`)**: When a user selects or explicitly confirms an autocompleted word, the `log_selection` method is invoked. This method efficiently traverses the Trie to the corresponding word's node and **increments its `frequency` count by a predefined value (e.g., 5)**. This direct update mechanism ensures that words frequently used by the developer are given higher prominence in future suggestions, personalizing the autocomplete experience over time.
* **Dynamic Relevance:** The accumulated frequency acts as a robust indicator of a word's practical relevance within a specific user's coding patterns.

### 3. Advanced Fuzzy Matching Algorithm: Resilient Suggestions

CodeSense Copilot's ability to suggest words despite typos or partial input is powered by a sophisticated **recursive fuzzy matching algorithm** (`Trie::search_fuzzy`), a variant of the Levenshtein (edit-distance) algorithm.

* **Recursive Exploration**: The `search_fuzzy` function performs a recursive traversal of the Trie, simultaneously comparing paths in the Trie against the `target` input string. It meticulously tracks `edits_remaining` (the maximum allowed edit operations) and `edits_used` (actual edits consumed) to ensure matches fall within the specified `max_edits` threshold.
* **Edit Operations Simulation**: At each step, the algorithm explores four fundamental edit possibilities:
    * **Match**: If a Trie character perfectly matches the current `target` character, no edit is consumed, and both Trie and target indices advance.
    * **Substitution**: If a Trie character does *not* match the current `target` character, one edit is consumed, and both Trie and target indices advance, simulating a character substitution.
    * **Insertion (into target)**: Simulates the insertion of a character into the `target` string. The Trie index advances to a child, but the `target` index remains the same, consuming one edit.
    * **Deletion (from target)**: Simulates the deletion of a character from the `target` string. The Trie index remains the same, but the `target` index advances, consuming one edit.
* **Duplicate Management**: A `std::unordered_map<std::string, FuzzyMatch>` (`results`) is used to store discovered fuzzy matches. This map efficiently handles cases where the same word might be reached via different edit paths, ensuring that only the path with the *minimum* `edit_distance` is retained for that word.
* **Termination Conditions**: Recursion terminates if a null node is encountered (invalid Trie path) or if `edits_remaining` falls below zero (exceeding the allowed edit threshold). Special handling is implemented for when the `target` string has been fully processed (`index == target.size()`), allowing for additional "insertions" (trailing characters in a Trie word) to be considered as valid edits.

### 4. Comprehensive Scoring and Ranking Mechanisms

The engine employs a multi-faceted ranking system to deliver the most relevant suggestions.

* **Dynamic Scoring (`Trie::get_ranked_fuzzy_matches`)**: After identifying all potential fuzzy matches, each `FuzzyMatch` object is assigned a calculated `score`. The primary scoring formula is `score = frequency - alpha * edit_distance`, where:
    * `frequency` directly reflects the word's learned usage.
    * `edit_distance` quantifies the "fuzziness" or deviation from the input.
    * `alpha` is a tunable weighting parameter that determines the relative importance of edit distance versus frequency. A higher `alpha` penalizes less accurate matches more heavily.
    * The `FuzzyMatch` struct defines a custom `operator<` that prioritizes exact matches (edit distance 0), then sorts by the calculated `score` in descending order, and finally by word lexicographically (ascending) for tie-breaking.
* **Prefix-Boosted Fuzzy Filtering (`Trie::get_top_k_fuzzy_matches`)**: This method refines the fuzzy match results. It first identifies top `k` words that are *exact prefix matches* using `get_top_k_with_prefix`. Then, it iterates through all fuzzy matches: if a fuzzy match is *also* found within the set of prefix matches, its score is significantly boosted (by `+10`), and non-prefix fuzzy matches are discarded. This strategy ensures that high-relevance fuzzy suggestions are those that still maintain a strong prefix connection to the user's input. The final list is then truncated to `k` elements.
* **Frequency-Driven Prefix Ranking (`Trie::get_top_k_with_prefix`)**: For pure prefix-based suggestions, this method efficiently extracts all words with a given prefix and uses a `std::priority_queue` (implemented as a max-heap based on frequency) to retrieve the top `k` most frequent words, ensuring highly relevant and commonly used terms are prioritized.

### 5. Persistent Dictionary Storage

To ensure that learned frequencies and the entire autocomplete dictionary persist across application sessions, CodeSense Copilot implements robust file I/O operations.

* **Saving (`Trie::save_to_file`)**: All words and their associated frequencies are collected from the Trie via a DFS traversal (`Trie::collect_all_words`) and written to a specified file, typically in a `word frequency` per line format.
* **Loading (`Trie::load_from_file`)**: Upon application startup, the system can load a previously saved dictionary from a file. Each word-frequency pair is read and re-inserted into the Trie, restoring the learned state and ensuring continuity of the personalized autocomplete experience.
---

## Getting Started

### Prerequisites
* **Operating System:** Windows 10 or later (due to direct Windows API usage).
* **Compiler:** MSVC C++ compiler (comes with Visual Studio).
* **IDE:** Visual Studio 2019 or newer (recommended for seamless build).

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

## Future Enhancements

* **Advanced Contextual Awareness:** Implement a more robust parser to understand C++ syntax and provide truly semantic suggestions (e.g., class members after `.` or `->`).
* **Multi-line Editing:** Extend the console editor to support navigation and editing across multiple lines.
* **Cross-Platform Compatibility:** Adapt the console rendering logic to work on Linux/macOS using libraries like NCurses or similar.
* **Configurable Settings:** Allow users to customize `max_edits`, `alpha` values, and highlighting colors.
* **Dynamic Keyword Loading:** Load keywords from external files or C++ standard libraries.

---

## Contributing

Contributions to CodeSense Copilot are wellcomed! If you'd like to contribute, please follow these steps:

1.  Fork the repository.
2.  Create a new branch (`git checkout -b feature/your-feature-name`).
3.  Make your changes.
4.  Commit your changes (`git commit -m 'Add new feature X'`).
5.  Push to the branch (`git push origin feature/your-feature-name`).
6.  Open a Pull Request.

Please ensure your code adheres to good practices and includes relevant tests where applicable.

---

## License

This project is licensed under the MIT License - see the [LICENSE.md](LICENSE.md) file for details.

---
