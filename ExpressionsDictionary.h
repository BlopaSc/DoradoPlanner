#ifndef EXPRESSIONS_DICTIONARY_H
#define EXPRESSIONS_DICTIONARY_H
#include <map>
// TODO: Consider AVL-tree for more balanced tree -> faster lookups
namespace ExpressionsDictionary{
	class UniqueIdentifier{
		protected:
			static unsigned long long int uniqueID;
	};
	template <typename K,typename D> class Trie : public UniqueIdentifier{
		protected:
			class NodeTrie{
				public:
					K key;
					D data;
					std::map<K,NodeTrie*> children;
					NodeTrie();
					NodeTrie(K key);
					~NodeTrie();
			};
			NodeTrie* root;
			unsigned long long int count;
		public:
			Trie();
			~Trie();
			unsigned long long int size();
			template <class C> D& operator[](const C &container);
			void clear();
	};
};
#endif
