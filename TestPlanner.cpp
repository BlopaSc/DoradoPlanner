#include "Planner.cpp"
#include <stdio.h>
#include <string.h>
#include <algorithm>
#include <chrono>
#include <iomanip>
#include <iostream>
using std::cout;
using std::endl;

double totTime = 0.0;

int passes = 0;
int tests = 0;

bool leakTest=false;

void performTest(const char* testName, const char* domain, const char* problem){
	AStar::AStarMetrics metrics;
	auto tStart = std::chrono::steady_clock::now();
	DoradoPlanner dpl(domain);
	std::vector<std::string> res = dpl.plan(problem,&metrics);
	double timeMs = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - tStart).count();
	if(!leakTest){ 
		std::cout<<"Test "<<testName<<":\t";
		for(int i=0; i<(25-strlen(testName))/8 ; i++){ cout << "\t"; }
		std::cout<<(res.size()?"PASSED":"FAILED")<<"\tactions: "<<res.size()
			<<"\tt-time: "<<std::setprecision(3)<<(timeMs/1000.0)
			<<"\tFnodes: "<<metrics.frontierNodes
			<<"\tEnodes: "<<metrics.expandedNodes<<std::endl;
	}
	totTime += timeMs;
	Expressions::releaseMemory();
	PDDL::releaseMemory();
}

int main(){
	
	for(int i=0;i<(leakTest?100:1);i++){
	
	performTest("airport-p04","competition/airport/p04-domain.pddl","competition/airport/p04-airport2-p1.pddl");
	performTest("airport-p05","competition/airport/p05-domain.pddl","competition/airport/p05-airport2-p1.pddl");
	performTest("airport-p06","competition/airport/p06-domain.pddl","competition/airport/p06-airport2-p2.pddl");
	
	performTest("airport-adl-p01","competition/airport-adl/domain.pddl","competition/airport-adl/p01-airport1-p1.pddl");
	performTest("airport-adl-p02","competition/airport-adl/domain.pddl","competition/airport-adl/p02-airport1-p1.pddl");
	
	performTest("elevators-s2-2","competition/elevators-00-strips/domain.pddl","competition/elevators-00-strips/s2-2.pddl");
	performTest("elevators-s3-4","competition/elevators-00-strips/domain.pddl","competition/elevators-00-strips/s3-4.pddl");
	performTest("elevators-s4-3","competition/elevators-00-strips/domain.pddl","competition/elevators-00-strips/s4-3.pddl");
	performTest("elevators-s5-2","competition/elevators-00-strips/domain.pddl","competition/elevators-00-strips/s5-2.pddl");
	performTest("elevators-s6-1","competition/elevators-00-strips/domain.pddl","competition/elevators-00-strips/s6-1.pddl");
	
	performTest("elevators-adl-s2-4","competition/elevators-00-adl/domain.pddl","competition/elevators-00-adl/s2-4.pddl");
	performTest("elevators-adl-s3-2","competition/elevators-00-adl/domain.pddl","competition/elevators-00-adl/s3-2.pddl");
	performTest("elevators-adl-s4-1","competition/elevators-00-adl/domain.pddl","competition/elevators-00-adl/s4-1.pddl");
	performTest("elevators-adl-s4-4","competition/elevators-00-adl/domain.pddl","competition/elevators-00-adl/s4-4.pddl");
	performTest("elevators-adl-s5-3","competition/elevators-00-adl/domain.pddl","competition/elevators-00-adl/s5-3.pddl");
	
	performTest("logistics-p01","competition/logistics/domain.pddl","competition/logistics/p01.pddl");
	performTest("logistics-p03","competition/logistics/domain.pddl","competition/logistics/p03.pddl");
	performTest("logistics-p04","competition/logistics/domain.pddl","competition/logistics/p04.pddl");
	
	performTest("movie-p03","competition/movie/domain.pddl","competition/movie/prob03.pddl");
	performTest("movie-p10","competition/movie/domain.pddl","competition/movie/prob10.pddl");
	performTest("movie-p13","competition/movie/domain.pddl","competition/movie/prob13.pddl");
	performTest("movie-p17","competition/movie/domain.pddl","competition/movie/prob17.pddl");
	performTest("movie-p24","competition/movie/domain.pddl","competition/movie/prob24.pddl");
	performTest("movie-p29","competition/movie/domain.pddl","competition/movie/prob29.pddl");
	
	performTest("psr-small-p01","competition/psr-small/p01-domain.pddl","competition/psr-small/p01-s2-n1-l2-f50.pddl");
	performTest("psr-small-p03","competition/psr-small/p03-domain.pddl","competition/psr-small/p03-s7-n1-l3-f70.pddl");
	performTest("psr-small-p04","competition/psr-small/p04-domain.pddl","competition/psr-small/p04-s8-n1-l4-f10.pddl");
	performTest("psr-small-p05","competition/psr-small/p05-domain.pddl","competition/psr-small/p05-s9-n1-l4-f30.pddl");
	
	performTest("tpp-p03","competition/tpp/domain.pddl","competition/tpp/p03.pddl");
	performTest("tpp-p04","competition/tpp/domain.pddl","competition/tpp/p04.pddl");

	}
	
	std::cout<<"Total time taken: "<<std::setprecision(3)<<(totTime/1000.0)<<" s"<<std::endl;
	
	printf("Press ENTER...");
	fgetc(stdin);
	return 0;
}
