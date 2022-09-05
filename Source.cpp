#include <limits> 
#include <iostream>
#include <string>
#include <iterator> 
#include <vector> 
#include <utility> 
#include <math.h> 
#include <stdexcept>


#define INI_HASH 3 // Default hashtable length

using std::iterator;
using std::vector;
using std::pair;
using std::cin;
using std::cout;
using std::string;
using std::getline;
using std::isdigit;


template <typename T>
class HashTable {
private:
	typedef vector<pair<int, T> > bucket;
	vector<bucket> table;
	vector<int> keys; // All the keys in the hashtable

	/* Function declaration */
	void rehash(const int old_size);
	typename bucket::iterator search_key(bucket& b_idx, const int& key, int start, int end);
	pair<int, T>& insert_pair(bucket& b_idx, const pair<int, T>& k_v, int start, int end);
public:
	HashTable() : table(INI_HASH) {}; // Default hashtable

	/* Function declaration */
	int size() const { return keys.size(); } // Return the number of elements in the table
	bool empty() const { return keys.empty(); }	 // Return true if the hashtable is empty, otherwise false
	void print_table() const; 
	bool insert(const int key, const T element); 
	void erase(const int& key);
	void clear();

	T* find(const int& key);

	T& operator[](int key); // Overload [] operator to access elements in the table
	
};


/*Clear the table and return to the default table*/
template<typename T>
void HashTable<T>::clear()
{
	table.clear();
	table.resize(INI_HASH); // Resize to default hashtable
	keys.clear();
}




/**
* Insert the key-value pair `k_v` in the range [start, end) of the bucket at some index `b_idx` in the table
* The key-value pairs are always inserted in ascending ordering according to their keys
* @param b_idx Bucket at some index to be inserted with key-value pair
* @param k_v Key-value-pair to be inserted
* @param start The starting point of the insertion, included; Condition: start < end
* @param end The ending point of the insertion, excluded; Condition: start < end
* 
* NOTE: It won't update keys
*/
template<typename T>
pair<int, T>& HashTable<T>::insert_pair(bucket& b_idx, const pair<int, T>& k_v, int start, int end) {
	if (end == start) {
		throw std::invalid_argument("Insert_pair Argument Exception: end == start\n");
	}
	if (end - start == 1) {
		if (k_v.first < b_idx[start].first) {
			b_idx.insert(b_idx.begin() + start, k_v);
			return b_idx[start];
		}
		else {
			b_idx.insert(b_idx.begin() + end, k_v);
			return b_idx[end];
		}
	}
	int mid = floor((start + end) / 2); // Middle index
	if (k_v.first < b_idx[mid].first) return this->insert_pair(b_idx, k_v, start, mid);
	else return this->insert_pair(b_idx, k_v, mid, end);
}

/**
* Search the key in the range [start, end) of bucket[idx] and return the iterator that points to it, otherwise b_idx.end()
* @param b_idx Bucket at index idx
* @param key Key to be search for
* @param start The starting point of the search, included. 
* @param end The ending point of the search, excluded. 
* 
* 
* @return the iterator that points to the pair whose key  = `key` if found , otherwise b_idx.end()
* 
*/
template<typename T>
typename HashTable<T>::bucket::iterator HashTable<T>::search_key(bucket& b_idx, const int& key, int start, int end) 
{
	if ((end - start) == 1) {
		if (b_idx[start].first == key) return b_idx.begin() + start;
		else return b_idx.end();
	}
	if (start == end) return b_idx.end();
	int mid = floor((start + end) / 2);
	if (key < b_idx[mid].first) return this->search_key(b_idx, key, start, mid);
	return this->search_key(b_idx, key, mid, end);
}

/**
* Rehash the table after resizing
* @param old_size The old table size before resizing
*/
template<typename T>
void HashTable<T>::rehash(const int old_size) {
	bucket k_vs; // A vector of key-value pairs that should be rehashed
	vector<typename bucket::iterator> erase_its; // A vector of iterators to the key-value pairs that should be rehashed
	int key;
	int new_idx; // New index for the `key`
	int new_size = table.size(); //Current new table size 
	vector<int> keys_backup = keys; // Save the current keys in keys_backup because `erase` function will remove keys from this->keys during rehashing
	int b_size;

	
	for (int idx = 0; idx < old_size; idx++) {
		for (auto it = table[idx].begin(); it < table[idx].end(); it++) {
			key = (*it).first;
			new_idx = key % new_size; // New index for key  = `key` after resizing
			if (new_idx != idx) {
				k_vs.push_back((*it));
				erase_its.push_back(it);
			}
		}
		/* Erase all the pairs that should be rehashed at table[idx]  */
		if (!erase_its.empty()) {
			for (auto eit = erase_its.begin(); eit < erase_its.end(); eit++) {
				table[idx].erase(*eit);
			}
			erase_its.clear();
		}
	}
	/* Insert back to the table the key-value pairs that was removed for rehashing */
	for (auto k_v : k_vs) {
		key = k_v.first;
		new_idx = key % new_size;
		b_size = table[new_idx].size();
		if (b_size == 0) table[new_idx].push_back(k_v);
		else this->insert_pair(table[new_idx], k_v, 0, b_size);
		if (table[new_idx].size() != b_size + 1) {
			throw "Insersion failed when rehshing!!\n";
		}
	}

	keys = keys_backup; //Restore all the keys in the table for `keys`
}


/**
* Insert a new key-value pair to the hashtable; Insertion fails if key already in the hashtable
* @param key Key to be inserted with `element`
* @param element Value to be inserted with `key`
* 
* @return bool True if insertion occurred, otherwise false
*/
template<typename T>
bool HashTable<T>::insert(const int key, const T element)
{	
	int idx = key % (table.size());
	typename vector<pair<int, T>>::iterator find_key = table[idx].end();
	if (!keys.empty()) {
		find_key = this->search_key(table[idx], key, 0, table[idx].size());
	} //No insertion if key aready in the table
	if (find_key != table[idx].end()) {
		return false;
	} // Start insertion
	else {
		int old_size = table.size();
		/*Increased the size of the table  to twice the current size plus one whenever the load factor is >= 0.8 */
		if ((keys.size() / old_size) >= 0.8) {
			table.resize(old_size * 2 + 1);
			this->rehash(old_size);
			cout << "Increasing table size to " << table.size() << ".... DONE!\n"; // For debugging
			idx = key % (table.size()); // Update idx to new idx after resizing
		}
		bucket& b_idx = table[idx]; // bucket at idx of the table
		int b_size = b_idx.size(); // bucket size
		int newsize = b_size + 1; // bucket size after successful insertion
		pair <int, T> new_kv(key, element);
		if (b_size == 0) b_idx.push_back(new_kv);
		else this->insert_pair(b_idx, new_kv, 0, b_size);
		if (b_idx.size() == newsize) {
			keys.push_back(key);
			return true;
		}
	}
	return false;
}

/**
* Searches the table for an element with a key = `key` and returns a pointer to the element if the search is successful, or nullptr if the search is unsuccessful
* @param key Key to be searched for
* 
* @return A pointer to the element if the search is successful, or nullptr if the search is unsuccessful
*/
template<typename T>
T* HashTable<T>::find(const int& key)
{
	int idx = key % (table.size());
	bucket& b_idx = table[idx];
	if (b_idx.empty()) return nullptr; // Empty bucket
	auto it = this->search_key(b_idx, key, 0, b_idx.size());
	if (b_idx.end() == it) return nullptr;
	else return &((*it).second);
}


/**
* Erase the key-value pair which key = `key` from the table
* @param key Key to be removed from the table
*/
template<typename T>
void HashTable<T>::erase(const int& key)
{
	int idx = key % (table.size());
	bucket& b_idx = table[idx];
	auto it = this->search_key(b_idx, key, 0, b_idx.size());
	if (it != b_idx.end()) {
		b_idx.erase(it);
		/* Update `keys`*/
		for (auto it = keys.begin(); it < keys.end(); it++) {
			if ((*it) == key) {
				keys.erase(it);
				break;
			}
		}
		
	}
}


template<typename T>
T& HashTable<T>::operator[](int key) {
	int idx = key % (table.size());
	T element; // Default item for key `key`
	bucket& b_idx = table[idx];
	pair <int, T> new_kv(key, element);

	if (b_idx.size() == 0) {
		b_idx.push_back(new_kv);
		keys.push_back(key);
		return (*b_idx.begin()).second;
	}
	auto it = this->search_key(b_idx, key, 0, b_idx.size());
	if (it != b_idx.end()) return (*it).second;
	else {
		return (this->insert_pair(b_idx, new_kv, 0, b_idx.size())).second;
	}
}


/* Print the table content*/ 
template<typename T>
void HashTable<T>::print_table() const
{
	if (this->empty()) {
		cout << "The table is empty!\n";
		return;
	}
	size_t idx;
	for (idx = 0; idx < table.size(); idx++) {
		if (!table[idx].empty()) {
			for (auto i = table[idx].begin(); i < table[idx].end(); i++)
			{
				cout << "key = " << (*i).first << ", value = " << (*i).second << "\n";
			}
		}
	}

}

int getKey() {
	string key_s;
	int key = -1;
	getline(cin, key_s);
	for (char c : key_s) {
		if (!isdigit(c)) {
			cout << "Invalid key. Please enter a valid key:\n";
			key = getKey();
			break;
		}
	}
	if (key_s.size() == 0) {
		cout << "Invalid key. Please enter a valid key:\n";
		key = getKey();
	} 
	else if(key == -1) key = stoi(key_s);

	return key;
}

int getValue() {
	string value_s;
	int value = -1;
	getline(cin, value_s);
	for (char c : value_s) {
		if (!isdigit(c)) {
			cout << "Invalid value. Please enter a valid value:\n";
			value = getValue();
			break;
		}
	}
	if (value_s.size() == 0) {
		cout << "Invalid value. Please enter a valid value:\n";
		value = getValue();
	}
	else if (value == -1) value = stoi(value_s);

	return value;
}


/* Test from user input */
int main(void) {
	HashTable<int> ht;
	int key;
	int value;
	int mode;
	//int mode = 0;
	if (ht.empty()) cout << "The hashtable is currently empty\n";
	//cin >> mode;
	string line;
	cout << "Please enter the mode to operate on the table: (1)Insert (2)Find (3)Erase (4)Clear (5)Print (6)Leave\n";
	// Get the input from std::cin and store into line
	while (getline(cin, line)) {
		if (line.size() > 1) {
			cout << "Invalid mode!!\n";
			cout << "Please enter the mode to operate on the table: (1)Insert (2)Find (3)Erase (4)Clear (5)Print (6)Leave\n";
			continue;
		}
		if (isdigit(line[0])) {
			mode = stoi(line);
		}
		else {
			cout << "Invalid mode!!\n";
			cout << "Please enter the mode to operate on the table: (1)Insert (2)Find (3)Erase (4)Clear (5)Print (6)Leave\n";
			continue;
		}

		if (mode == 6) break;

		switch (mode)
		{
		case 1:
			cout << "INSERTION\n";
			cout << "Please enter the key:\n";
			key = getKey();
			cout << "Please enter the value:\n";
			value = getValue();
			if (ht.insert(key, value)) cout << "Inserting (" << key << ", " << value << ")....Succeed!\n";
			else cout << "Inserting (" << key << ", " << value << ")....Fail!\n";
			break;
		case 2:
			cout << "FIND\n";
			cout << "Please enter the key:\n";
			key = getKey();
			if (ht.find(key)) cout << "Found the key = " << key << "  with value = " << *(ht.find(key)) << "\n";
			else cout << "Cannot find key = " << key << " in the table\n";
			break;
		case 3:
			cout << "ERASE\n";
			cout << "Please enter the key:\n"; 
			key = getKey();
			ht.erase(key);
			cout << "Erasing.... Done!\n";
			break;
		case 4:
			cout << "CLEAR\n";
			ht.clear();
			cout << "Clearing.... Done!\n";
			break;
		case 5:
			ht.print_table();
			break;
		default:
			cout << "Invalid mode!!\n";
			cin.clear();
			break;
		}
		cout << "Please enter the mode to operate on the table: (1)Insert (2)Find (3)Erase (4)Clear (5)Print (6)Leave\n";
	}
	return 0;
}


/*Test for operator[]*/
/*
int main(void) {
	HashTable<char> ht;
	ht.print_table();
	ht[0] = 'a';
	ht[1] = 'b';
	ht[2] = 'c';
	ht.print_table();
	return 0;
}

*/