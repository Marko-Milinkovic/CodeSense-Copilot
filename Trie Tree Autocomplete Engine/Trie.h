#pragma once
#include <iostream>
#include <memory>
#include <array>
#include <vector>
#include <string>
#include <unordered_map>



struct FuzzyMatch {
	std::string word;
	int frequency;
	int edit_distance;
	double score;

	bool operator<(const FuzzyMatch& other) const {
		// --- NEW LOGIC FOR EDIT_DISTANCE == 0 PRIORITIZATION ---

		// Case 1: 'this' has edit_distance 0, 'other' does not.
		// 'this' should come before 'other' (return true).
		if (this->edit_distance == 0 && other.edit_distance != 0) {
			return true;
		}

		// Case 2: 'other' has edit_distance 0, 'this' does not.
		// 'this' should come after 'other' (return false).
		if (this->edit_distance != 0 && other.edit_distance == 0) {
			return false;
		}

		// --- EXISTING LOGIC (when edit_distance status is the same for both) ---
		// This part is reached if:
		// A) Both 'this' and 'other' have edit_distance == 0
		// B) Both 'this' and 'other' have edit_distance > 0

		// Primary sort: descending by score
		// If scores are different, the one with the higher score comes first.
		// (score > other.score means 'this' is "less than" 'other' if we want descending order)
		if (score != other.score) {
			return score > other.score;
		}

		// Secondary sort (for ties in score): ascending by word (lexicographical)
		// If scores are equal, the one with the lexicographically smaller word comes first.
		return word < other.word;
	}
};


class Trie {

private:

	class TrieNode {

	public:

		bool is_end_of_word;
		std::array<std::unique_ptr<TrieNode>, 26> children;

		int frequency = 0;

		// node does not contain symbol character but they are known based on children index

		TrieNode() : is_end_of_word(false) {}

	};

	std::unique_ptr<TrieNode> root;

public:

	Trie() : root(std::make_unique<TrieNode>()) {}

	void insert(const std::string& word, int freq = 1);
	void log_selection(const std::string& word);

	std::vector<std::string> get_top_k_with_prefix(const std::string& prefix, int k) const;
	
	void dfs(TrieNode* node, std::string& current, std::vector<std::pair<std::string, int>>& results) const;

	std::vector<FuzzyMatch> get_top_k_fuzzy_matches(const std::string& input, int max_edits, int k) const;
	
	void debug_print() const;

	void save_to_file(const std::string& filename) const;
	void load_from_file(const std::string& filename);

private:

	std::vector<std::pair<std::string, int>> get_words_with_prefix(const std::string& prefix) const;
	
	std::vector<FuzzyMatch> get_ranked_fuzzy_matches(const std::string& input, int max_edits, double alpha = 1) const;
	void search_fuzzy(TrieNode* node, const std::string& target, std::string& current, int index, int edits_remaining, std::unordered_map<std::string, FuzzyMatch>& results, int edits_used) const;

	void collect_all_words(TrieNode* node, std::string& current, std::vector<std::pair<std::string, int>>& out) const;

	void debug_print_recursive(TrieNode* node, std::string& current, int depth) const;
};
