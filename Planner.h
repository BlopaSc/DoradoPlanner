#ifndef PLANNER_H
#define PLANNER_H
#include "Astar.cpp"
#include "Expressions.cpp"
#include "PDDL.cpp"
#include <iostream>
#include <map>
#include <string>
#include <utility>
#include <vector>

class DoradoPlanner{
	public:
		class Action{
			public:
				static std::map<AStar::idaction_t,std::string> mapActions;
				std::string name;
				Expressions::Expression* precondition;
				Expressions::Expression* effect;
				AStar::idaction_t actionid;
				Action(const std::string &n,Expressions::Expression* pc,Expressions::Expression* ef);
		};
		class WorldState{
			public:
				static std::vector<Action> actions;
				static Expressions::Expression* goal;
				Expressions::World* world;
				WorldState();
				WorldState(Expressions::World* w);
				~WorldState();
				AStar::idstate_t getKey();
				AStar::NodeNeighbors<WorldState> getNeighbors();
				static bool goalFunction(const WorldState& state);
		};
	protected:
		std::vector<std::vector<std::pair<std::string,std::string>>> possibleParameters(PDDL::Problem* problem, const std::vector<std::pair<std::string,std::string>> &params, int curr=0);
		PDDL::Domain* domain;
	public:
		DoradoPlanner(const std::string filename);
		std::vector<std::string> plan(const std::string filename,AStar::AStarMetrics *mets=0);
};

#endif
