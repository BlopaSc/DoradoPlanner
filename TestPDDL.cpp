#include <stdio.h>
#include "PDDL.cpp"

using namespace PDDL;
using std::cout;
using std::endl;

bool leakTest = true;

int main(){
	
	for(int i=0;i<(leakTest?100000:1);i++){
	
	Domain* domain = parsePDDLDomain("test_domains/elevator_adl_domain.pddl");
	if(!leakTest){ cout<<*domain<<std::endl; } 
	domain = parsePDDLDomain("test_domains/elevator_adl_domain.pddl");
	if(!leakTest){ cout<<*domain<<std::endl; } 
	Problem* problem = parsePDDLProblem("test_domains/elevator_adl_problem1.pddl");
	if(!leakTest){ cout<<*problem<<std::endl; } 
	problem = parsePDDLProblem("test_domains/elevator_adl_problem1.pddl");
	if(!leakTest){ cout<<*problem<<std::endl; } 
	
	}
	
	printf("Press ENTER...");
	fgetc(stdin);
	return 0;
}
