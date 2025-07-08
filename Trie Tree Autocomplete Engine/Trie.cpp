#include "Trie.h"
#include <queue>
#include <fstream>
#include <unordered_set>

void Trie::insert(const std::string& word, int freq)
{
	TrieNode* node = root.get();

	for (char ch : word) {

		if (!std::islower(ch)) continue; // allow only small letters

		size_t index = ch - 'a';

		if (node->children[index] == nullptr) {
			node->children[index] = std::make_unique<TrieNode>();
		}
		node = node->children[index].get();
	}

	node->is_end_of_word = true;
	node->frequency = freq;

}

void Trie::log_selection(const std::string& word)
{
	TrieNode* node = root.get();

	for (char ch : word) {
		size_t index = ch - 'a';
		if (!node->children[index]) {
			return;
		}
		node = node->children[index].get();
	}

	if (node->is_end_of_word) {
		node->frequency = node->frequency + 5;
	}

}

void Trie::debug_print() const
{
	std::string current;
	debug_print_recursive(root.get(), current, 0);
}

std::vector<std::pair<std::string, int>> Trie::get_words_with_prefix(const std::string& prefix) const 
{
	std::vector<std::pair<std::string, int>> results;
	TrieNode* node = this->root.get();

	for (char ch : prefix) {
	
		size_t index = ch - 'a';
		if (!node->children[index]) return results;
		node = node->children[index].get();

	}

	std::string current = prefix;
	dfs(node, current, results);

	return results;
}

std::vector<FuzzyMatch> Trie::get_top_k_fuzzy_matches(const std::string& input, int max_edits, int k) const
{
	std::vector<FuzzyMatch> all_matches = get_ranked_fuzzy_matches(input, max_edits);

	/*
	std::cout << "\n--- All Fuzzy Matches for input '" << input << "' (before sorting) ---\n";
	for (const auto& m : all_matches) {
		std::cout << "Word: " << m.word
			<< ", Freq: " << m.frequency
			<< ", Dist: " << m.edit_distance
			<< ", Score: " << m.score << "\n";
	}
	std::cout << "--------------------------------------------------------\n";
	// --- END DEBUG PRINTS ---
	*/
	std::vector<std::string> words_with_prefix = this->get_top_k_with_prefix(input, k);
	std::unordered_set<std::string> prefix_words_set(words_with_prefix.begin(), words_with_prefix.end());
	//this->prefix_checking(matches, prefix_words_set);
	
	for (int i = 0; i < all_matches.size(); /* no i++ here */) { // Note: no i++ in the loop header
		bool found_in_prefix = false;
		if (prefix_words_set.count(all_matches[i].word)) { // count returns 1 if found, 0 if not
			found_in_prefix = true;
			all_matches[i].score = all_matches[i].score + 10;
		}
		if (!found_in_prefix) {
			all_matches.erase(all_matches.begin() + i);
		}
		else {
			i++;
		}
	}

	if ((int)all_matches.size() > k) {
		all_matches.resize(k); // keep only top k
	}

	return all_matches;
}

std::vector<std::string> Trie::get_top_k_with_prefix(const std::string& prefix, int k) const
{
	
	std::vector<std::pair<std::string, int>> all_words = get_words_with_prefix(prefix);

	auto cmp = [](const auto& a, const auto& b) {
		// max heap based on frequeyncy
		return a.second < b.second || (a.second == b.second && a.first > b.first);
		};

	std::priority_queue <
		std::pair<std::string, int>,
		std::vector<std::pair<std::string, int>>,
		decltype(cmp)
	> pq(cmp, all_words);

	std::vector<std::string> result;
	for (int i = 0; i < k && !pq.empty(); i++) {
		result.push_back(pq.top().first);
		pq.pop();
	}
	return result;
}

void Trie::dfs(TrieNode* node, std::string& current, std::vector<std::pair<std::string, int>>& results) const
{
	if (node->is_end_of_word) {
		results.emplace_back(current, node->frequency);
	}

	for (char ch = 'a'; ch <= 'z'; ++ch) {
		size_t index = ch - 'a';
		if (node->children[index]) {
			current.push_back(ch);
			dfs(node->children[index].get(), current, results);
			current.pop_back();
		}
	}
}

/*
std::vector<FuzzyMatch> Trie::get_fuzzy_matches(const std::string& input, int max_edits) const
{	
	// entry point method for kicking in recursive fuzzy search
	// max_edits is LevenshteinDistance = minimum number of single-character edits (insertions, deletions, or substitutions) required to change one word into another 
	// example -> LevenshteinDistance("kitten", "sitting") = 3

	// input = user's input we search fuzzy matches
	// max_edits = LevenshteinDistance, by default allows one edit per word [ applw = apple ]

	std::vector<FuzzyMatch> results; // container for all the fuzzy matching words found 
	std::string current; // temprary string buffer to build up the word currently being explored during fuzzy search
	search_fuzzy(root.get(), input, current, 0, max_edits, results, 0); // start the recursive fuzzy search

	// post processing
	std::sort(results.begin(), results.end()); // optional, but good for consistent output
	results.erase(std::unique(results.begin(), results.end()), results.end()); // remove dupes
	return results;
}
*/

void Trie::search_fuzzy(TrieNode* node, const std::string& target, std::string& current, int index, int edits_remaining, std::unordered_map<std::string, FuzzyMatch>& results, int edits_used) const
{
	// recursive helper for fuzzy search
	// recursivly explore all valid paths in the Trie thad could potentially form a fuzzy match to target with the given edits_remaining limit
	// explores all possible paths in the Trie, corresponding to different combinations of edits, while keeping edits_remaining count updated
	// each recursive call needs to know:
	// 1) where are we in Trie currently during DFS = node
	// 2) what target string we are trying to match (original user input string) = target
	// 3) what word is being built up along the current path in the Trie = current
	// 4) what character we are currently trying to match in the target string = index
	// 5) how many edits are we still allowed to make to transform target into word found in the Trie (when zero search stops) = edits_remaining
	// 6) variable to store found matches = results;

	// [1] Base cases / Pruning
	// If we hit a null node (meaning this path doesn't exist in the Trie)
	// Or if we've used up all our allowed edits (edits_remaining becomes negative)
	// then this path cannot lead to a valid match. Stop exploring.
	if (!node || edits_remaining < 0) return;

	// [2] End of Target String (Base Case for Target Index) ---
	// This block is executed when we have processed all characters of the 'target' string.
	// At this point, 'index' has reached the length of the 'target' string
	if (index == target.size()) {
		if (node->is_end_of_word && edits_remaining >= 0) {
			auto it = results.find(current);
			if (it == results.end() || it->second.edit_distance > edits_used) {
				results[current] = FuzzyMatch{ current, node->frequency, edits_used };
			}
		}
		// Allow extra insertions at the end
		// This part handles cases where the target string might be a prefix of a word in the Trie,
		// and the remaining characters in the Trie word are "insertions" relative to the target.
		// Example: target="cat", max_edits=1. Trie has "cats".
		// When index == target.size() (after 't' in target 'cat'), we are at Trie node for 't'.
		// We can then "insert" 's' (if edits_remaining > 0) to match "cats".
		for (char ch = 'a'; ch <= 'z'; ++ch) {
			int i = ch - 'a';
			if (node->children[i]) { // If there's a child for this character in the Trie
				current.push_back(ch); // Add the Trie char to our current_word_path
				// Recurse: Move to the child node, but 'index' (target position) remains the same
				// as we are simulating an *insertion* into the target (i.e., we are consuming an edit
				// without advancing in the target string)
				search_fuzzy(node->children[i].get(), target, current, index, edits_remaining - 1, results, edits_used + 1);
				current.pop_back(); // backtrack
			}
		}
		return;
	}

	// [3] General Recursive Step
	// This is the core recursive logic that explores the four edit possibilities: Match, Substitution, Insertion, Deletion

	char target_ch = target[index]; // Get the current character from the target string

	// Explore all 26 possible next characters in Trie;
	// This loop explores 'Match' and 'Substitution' possibilities by comparing
	// It als handles 'Insertion' from the perspective of the Trie (skipping the target character)
	for (char ch = 'a'; ch <= 'z'; ++ch) {
		int i = ch - 'a';
		if (!node->children[i]) continue;

		current.push_back(ch); // Tentatively add this Trie character to our current word path

		if (ch == target_ch) {
			// A) Perfect match (no edits)
			// If the Trie character matches the target character, no edit is consumed
			// We advance in both the Trie (to child) and the target string (to next index)
			search_fuzzy(node->children[i].get(), target, current, index + 1, edits_remaining, results, edits_used);
		}
		else {
			// B) Substitution (1 edit)
			// If the Trie character DOES NOT match the target character, one edit is consumed
			// We advance in both Trie and target string
			search_fuzzy(node->children[i].get(), target, current, index + 1, edits_remaining - 1, results, edits_used + 1);
		}

		// C) Insertion (into target string)
		// This simulates inserting the current Trie character into the target string
		// We advance in the Trie (to child) but DO NOT advance in the target string
		// This implies that the 'ch' character from the Trie is "inserted" into the target before
		// One edit is consumed
		search_fuzzy(node->children[i].get(), target, current, index, edits_remaining - 1, results, edits_used + 1);

		current.pop_back();
	}

	// D) Deletion (skip char in word)
	// This simulates deleting the current target character ('target_ch') from the target string.
	// We DO NOT advance in the Trie (stay at the same node) but advance in the target string (index + 1).
	// This implies that target_ch is deleted and we move on to try matching the next target character
	// with the same Trie node. One edit is consumed.
	search_fuzzy(node, target, current, index + 1, edits_remaining - 1, results, edits_used + 1);

}

std::vector<FuzzyMatch> Trie::get_ranked_fuzzy_matches(const std::string& input, int max_edits, double alpha) const
{	
	// entry point method for kicking in recursive fuzzy search
	// max_edits is LevenshteinDistance = minimum number of single-character edits (insertions, deletions, or substitutions) required to change one word into another 
	// example -> LevenshteinDistance("kitten", "sitting") = 3

	// input = user's input we search fuzzy matches
	// max_edits = LevenshteinDistance, by default allows one edit per word [ applw = apple ]
	std::unordered_map<std::string, FuzzyMatch> result_map;
	std::string current;
	search_fuzzy(root.get(), input, current, 0, max_edits, result_map, 0);

	std::vector<FuzzyMatch> result_vec;
	for (auto& pair : result_map) {
		FuzzyMatch& match = pair.second;
		match.score = match.frequency - alpha * match.edit_distance;
		result_vec.push_back(match);
	}

	std::sort(result_vec.begin(), result_vec.end()); // uses operator<

	return result_vec;
}

void Trie::debug_print_recursive(TrieNode* node, std::string& current, int depth) const
{
	if (!node) return;

	if (node->is_end_of_word) {
		std::cout << std::string(depth * 2, ' ') << "- " << current
			<< " (Freq: " << node->frequency << ")\n";
	}

	for (char ch = 'a'; ch <= 'z'; ++ch) {
		int index = ch - 'a';
		if (node->children[index]) {
			current.push_back(ch);
			debug_print_recursive(node->children[index].get(), current, depth + 1);
			current.pop_back();
		}
	}
}


void Trie::load_from_file(const std::string& filename)
{
	std::ifstream in(filename);
	if (!in) {
		std::cerr << "No saved data found (" << filename << ").\n";
		return;
	}

	std::string word;
	int freq;
	while (in >> word >> freq) {
		insert(word, freq);
	}

}

void Trie::save_to_file(const std::string& filename) const
{	
	std::ofstream out(filename);
	if (!out) {
		std::cerr << "Failed to open file for saving.\n";
		return;
	}

	std::vector<std::pair<std::string, int>> words;
	std::string current;
	collect_all_words(root.get(), current, words);	

	for (const std::pair<std::string, int>& pair : words){
		std::string word = pair.first;
		int frequency = pair.second;
		out << word << ' ' << frequency << '\n';
	}
}

void Trie::collect_all_words(TrieNode* node, std::string& current, std::vector<std::pair<std::string, int>>& out) const
{
	if (node->is_end_of_word) {
		out.emplace_back(current, node->frequency);
	}

	for (char ch = 'a'; ch <= 'z'; ch++) {
		int index = ch - 'a';
		if (node->children[index]) {
			current.push_back(ch);
			collect_all_words(node->children[index].get(), current, out);
			current.pop_back();
		}
	}

}

