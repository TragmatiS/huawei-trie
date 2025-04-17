#include <iostream>
#include <iomanip>

#include "compact_prefix_tree.h"

using namespace std;

int main() {
	ios_base::sync_with_stdio(false);
	cin.tie(nullptr); cout.tie(nullptr);

	vector<string> dictionary;

	int m; cin >> m >> ws;
	for (int i = 0; i < m; i++) {
		string s; getline(cin, s);
		dictionary.push_back(s);
	}

	compact_prefix_tree tree(dictionary);
	int q; cin >> q >> ws;
	vector<bool> answer;
	for (int i = 0; i < q; i++) {
		string query; getline(cin, query);
		answer.push_back(tree.exists(query));
	}

	copy(answer.begin(), answer.end(), ostream_iterator<bool>(cout, " "));
}