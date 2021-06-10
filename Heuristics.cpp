#ifndef HEURISTICS_CPP
#define HEURISTICS_CPP
#include "Planner.h"
#include "Expressions.h"
namespace Heuristics{
	Expressions::Atoms positiveGoal;
	
	void setGoal(Expressions::Expression* goalExpression){
		positiveGoal.clear();
		Expressions::Atoms ignoreList;
		goalExpression->applyPositive(positiveGoal,ignoreList);
	}	
	
	double atomDistanceHeuristics(const DoradoPlanner::WorldState& state){
		Expressions::Atoms::iterator itPos = positiveGoal.begin();
		unsigned int count = 0;
		for(Expressions::Atoms::iterator it = state.world->atoms.begin(); itPos != positiveGoal.end() && it != state.world->atoms.end(); ++it){
			while(*itPos<=*it && itPos != positiveGoal.end()){
				if(*itPos == *it){
					count++;
				}
				++itPos;
			}
		}
		return positiveGoal.size() - count;
	}
	
};
#endif
