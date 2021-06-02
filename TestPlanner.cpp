#include "Planner.cpp"
#include <stdio.h>
#include <algorithm>
#include <iostream>
using std::cout;
using std::endl;

bool leakTest=false;

int main(){
	AStar::AStarMetrics metrics;
	std::vector<std::string> res;
	
	for(int i=0;i<(leakTest?100:1);i++){
	
	DoradoPlanner dpl("test_domains/elevator_adl_domain.pddl");
	
	res = dpl.plan("test_domains/elevator_adl_problem6.pddl",&metrics);
	
	}
	
	cout<<"Solution size: "<<res.size()<<endl;
	int act = 0;
	for(std::string s : res){
		cout<<"Action "<<++act<<": "<<s<<endl;
	}
	cout <<endl<< metrics << endl;
	
	Expressions::releaseMemory();
	
	printf("Press ENTER...");
	fgetc(stdin);
	return 0;
}
