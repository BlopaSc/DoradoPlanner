#ifndef EXPRESSIONS_DICTIONARY_CPP
#define EXPRESSIONS_DICTIONARY_CPP
#include "ExpressionsDictionary.h"
namespace ExpressionsDictionary{
	unsigned long long int UniqueIdentifier::uniqueID = 0;
	// Node Trie
	template <typename K,typename D> Trie<K,D>::NodeTrie::NodeTrie() : key(0), data(0){ }
	template <typename K,typename D> Trie<K,D>::NodeTrie::NodeTrie(K key) : key(key),data(0){ }
	template <typename K,typename D> Trie<K,D>::NodeTrie::~NodeTrie(){
		for(auto it = children.begin(); it != children.end(); ++it){
			delete it->second;
		}
	}
	// Trie
	template <typename K,typename D> Trie<K,D>::Trie() : count(0){ root = new NodeTrie(); }
	template <typename K,typename D> Trie<K,D>::~Trie(){ delete root; }
	template <typename K,typename D> unsigned long long int Trie<K,D>::size(){ return count; }
	template <typename K,typename D> template<class C> D& Trie<K,D>::operator[](const C &container){
		NodeTrie* ptr = root;
		for(auto elem : container){
			NodeTrie** tmp = &ptr->children[elem];
			if(!*tmp){ *tmp = new NodeTrie(elem); }
			ptr = *tmp;
		}
		if(!ptr->data){
			++count;
			ptr->data = ++uniqueID;
		}
		return ptr->data;
	}
	template <typename K,typename D> void Trie<K,D>::clear(){
		for(auto it = root->children.begin(); it != root->children.end(); ++it){
			delete it->second;
		}
		root->children.clear();
		count = 0;
	}
}
#endif
