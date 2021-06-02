#include "ExpressionsDictionary.cpp"
#include <string>
#include <vector>
#include <iostream>
using namespace std;
#include <stdio.h>

using namespace ExpressionsDictionary;

int main(){
	
	Trie<char,unsigned int> t;
	Trie<int,unsigned int> t2;
	
	string str = "hello";
	string str2 = "blopa";
	
	cout<<"Insert "<<str<<": "<<t[str]<<endl;
	cout<<"Insert "<<str2<<": "<<t[str2]<<endl;
	cout<<"Insert "<<str<<": "<<t[str]<<endl;
	cout<<"Insert "<<str<<": "<<t[str]<<endl;
	cout<<"Insert "<<str2<<": "<<t[str2]<<endl;
	cout<<"Insert "<<str<<": "<<t[str]<<endl;
	
	vector<int> x{-1};
	vector<int> y{3,2};
	vector<int> z{5};
	
	cout<<"Insert "<<x[0]<<": "<<t2[x]<<endl;
	cout<<"Insert "<<x[0]<<": "<<t2[x]<<endl;
	cout<<"Insert "<<y[0]<<": "<<t2[y]<<endl;
	cout<<"Insert "<<x[0]<<": "<<t2[x]<<endl;
	cout<<"Insert "<<z[0]<<": "<<t2[z]<<endl;
	
	for(int i=0;i<1000;i++){
		for(int j=0;j<1000;j++){
			x = {i,j};
			t2[x];
		}
	}
	
	std::cout<<"Size: "<<t2.size()<<std::endl;
	printf("Press ENTER...");
	fgetc(stdin);
	t2.clear();
	std::cout<<"Size: "<<t2.size()<<std::endl;
	
	printf("Press ENTER...");
	fgetc(stdin);
	return 0;
}
