#pragma once

#include <iterator>
#include <vector>
#include <algorithm>
#include <string>
#include <string_view>

using namespace std;

struct edge {
	int destination;

	//[left, right) in the edge string stash
	int left;
	int right;
};

struct node {
	vector<edge> children;
};

class compact_prefix_tree {
private:
	vector<node> node_stash; //node_stash[0] is the root
	string edge_string_stash;

	char at(const edge& e, int index) const;

	//remember to revalidate references b/c this reallocates node_stash
	int new_node();

	void add_record(string_view what);

public:
	compact_prefix_tree(const vector<string>& dictionary);
	bool exists(string_view query) const;
};

char compact_prefix_tree::at(const edge& e, int index) const {
	return edge_string_stash[e.left + index];
}

int compact_prefix_tree::new_node() {
	int index = node_stash.size();
	node_stash.push_back(node{});
	return index;
}

void compact_prefix_tree::add_record(string_view what) {
	int current_node_index = 0;
	int current_edge_index = -1;
	int i_when_edge_entered = -1;
	bool at_node = true;

	auto get_current_node = [&]() -> node& {
		return node_stash[current_node_index];
		};

	for (int i = 0; i < what.size(); i++) { //we'll be inspecting what[i]
		node& current_node = get_current_node();
		if (at_node) {
			auto it = find_if(
				current_node.children.begin(),
				current_node.children.end(),
				[&](const edge& e) -> bool {
					return at(e, 0) == what[i];
				}
			);
			if (it == current_node.children.end()) {
				//no such edge -> out of the tree...

				int destination = new_node(); //make the leaf node...

				int left = edge_string_stash.size();
				edge_string_stash.append(what.begin() + i, what.end()); //store in the stash...
				int right = edge_string_stash.size();

				get_current_node().children.emplace_back(destination, left, right); //store the new edge
				return;
			}
			//found it!
			at_node = false;
			current_edge_index = it - current_node.children.begin();
			i_when_edge_entered = i;
			i--; //so we are processing the same symbol again but in the edge we entered
			continue;
		}

		//in edge
		edge& current_edge = current_node.children[current_edge_index];
		int i_along_edge = i - i_when_edge_entered;
		if (what[i] == at(current_edge, i_along_edge)) {
			if (i_along_edge + 1 < current_edge.right - current_edge.left) {
				continue; //everything is fine
			}
			//the edge ends here!
			at_node = true;
			current_edge_index = -1;
			current_node_index = current_edge.destination;
			i_when_edge_entered = -1; //not necessary but let's keep it
			continue;
		}
		//busting out of the current edge
		//the split should be like [left, left+i_along_edge), [left+i_along_edge, right)
		edge old_edge = current_edge;
		current_edge.right = current_edge.left + i_along_edge;
		current_edge.destination = new_node();
		node& the_node = node_stash[current_edge.destination];

		the_node.children.push_back({
			old_edge.destination,
			current_edge.right,
			old_edge.right }); //the split edge

		at_node = true;
		current_edge_index = -1;
		current_node_index = current_edge.destination;
		i_when_edge_entered = -1;
		i--; //so we are processing this symbol again but at the node we entered
	}
}

compact_prefix_tree::compact_prefix_tree(
	const vector<string>& dictionary
) : node_stash(1), edge_string_stash{} {
	for (const string& record : dictionary) {
		add_record(record);
	}
}

//check if query terminates at some node
bool compact_prefix_tree::exists(string_view query) const {
	int current_node_index = 0;
	int current_edge_index = -1;
	int i_when_edge_entered = -1;
	bool at_node = true;
	for (int i = 0; i < query.size(); i++) {
		const node& current_node = node_stash[current_node_index];
		if (at_node) {
			auto it = find_if(
				current_node.children.cbegin(),
				current_node.children.cend(),
				[&](const edge& e) -> bool {
					return at(e, 0) == query[i];
				}
			);
			if (it == current_node.children.cend()) {
				return false;
			}
			at_node = false;
			current_edge_index = it - current_node.children.cbegin();
			i_when_edge_entered = i;
			i--;
			continue;
		}

		const edge& current_edge = current_node.children[current_edge_index];
		int i_along_edge = i - i_when_edge_entered;
		if (query[i] == at(current_edge, i_along_edge)) {
			if (i_along_edge + 1 < current_edge.right - current_edge.left) {
				continue; //everything is fine
			}
			//the edge ends here!
			at_node = true;
			current_edge_index = -1;
			current_node_index = current_edge.destination;
			i_when_edge_entered = -1; //not necessary but let's keep it
			continue;
		}
		//busting out of the current edge
		return false;
	}
	return at_node; //can only terminate at nodes
}