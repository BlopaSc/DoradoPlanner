#ifndef PLANNER_CPP
#define PLANNER_CPP
#include "PDDL.cpp"
#include "Expressions.cpp"
#include "Astar.cpp"
#include <iostream>
#include <map>
#include <string>
#include <utility>
#include <vector>

class DoradoPlanner{
	protected:
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
			// TODO: Consider inheriting from World and just moving contents
			public:
				static std::vector<Action> actions;
				static Expressions::Expression* goal;
				Expressions::World* world;
				WorldState(Expressions::World* w);
				~WorldState();
				AStar::idstate_t getKey();
				AStar::NodeNeighbors<WorldState> getNeighbors();
				static bool goalFunction(WorldState* state);
		};
		std::vector<std::vector<std::pair<std::string,std::string>>> possibleParameters(PDDL::Problem* problem, const std::vector<std::pair<std::string,std::string>> &params, int curr=1);
	public:
		PDDL::Domain* domain;
	public:
		DoradoPlanner(const std::string filename);
		std::vector<std::string> plan(const std::string filename,AStar::AStarMetrics *mets=0);
};

// Static variables
std::map<AStar::idaction_t,std::string> DoradoPlanner::Action::mapActions;
std::vector<DoradoPlanner::Action> DoradoPlanner::WorldState::actions;
Expressions::Expression* DoradoPlanner::WorldState::goal = 0;

// Action subclass
DoradoPlanner::Action::Action(const std::string &n,Expressions::Expression* pc,Expressions::Expression* ef) : name(n), precondition(pc), effect(ef) {
	actionid = mapActions.size()+1;
	mapActions[actionid] = n;
}

// World subclass
DoradoPlanner::WorldState::WorldState(Expressions::World* w) : world(w) {};
DoradoPlanner::WorldState::~WorldState() {};
AStar::idstate_t DoradoPlanner::WorldState::getKey(){ return world->key; }
AStar::NodeNeighbors<DoradoPlanner::WorldState> DoradoPlanner::WorldState::getNeighbors(){
	AStar::NodeNeighbors<DoradoPlanner::WorldState> neighbors;
	for(const Action &act : actions){
		if(act.precondition->isModeledBy(world)){
			WorldState* newState = new WorldState(world->apply(act.effect));
			neighbors.push_back({newState,1.0,act.actionid});
		}
	}
	return neighbors;
}
bool DoradoPlanner::WorldState::goalFunction(WorldState* state){
	return goal->isModeledBy(state->world);
}

// DoradoPlanner class
DoradoPlanner::DoradoPlanner(const std::string filename){
	domain = PDDL::parsePDDLDomain(filename);
}

std::vector<std::string> DoradoPlanner::plan(const std::string filename,AStar::AStarMetrics *mets){
	std::vector<std::string> solution;
	PDDL::Problem* problem = PDDL::parsePDDLProblem(filename);
	Action::mapActions.clear();
	WorldState::actions.clear();
	std::vector<Action> actions;
	for(const PDDL::Domain::Action &act : domain->actions){
		std::vector<std::vector<std::pair<std::string,std::string>>> params = possibleParameters(problem, act.parameters);
		Expressions::Expression* precondition = Expressions::make_expression(act.precondition);
		Expressions::Expression* effect = Expressions::make_expression(act.effect);
		for(const std::vector<std::pair<std::string,std::string>> &paramPerm : params){
			std::string name = act.name;
			Expressions::Expression* preconditionGrounded = precondition;
			Expressions::Expression* effectGrounded = effect;
			for(const std::pair<std::string,std::string> &paramSubstitution : paramPerm){
				name += " " + paramSubstitution.second;
				Expressions::idexpr_t var = Expressions::get_idword(paramSubstitution.first);
				Expressions::idexpr_t grounded = Expressions::get_idword(paramSubstitution.second);
				preconditionGrounded = preconditionGrounded->substitute(var,grounded);
				effectGrounded = effectGrounded->substitute(var,grounded);
			}
			actions.push_back({name,preconditionGrounded,effectGrounded});
		}
	}
	// TODO: To world init add constants
	WorldState* initialState = new WorldState(Expressions::make_world(problem->init,problem->sets));
	WorldState::goal = Expressions::make_expression(problem->goal);
	// Remove impossible actions
	Expressions::Atoms maximumList = initialState->world->atoms;
	for(const Action &act : actions){
		Expressions::Atoms addList;
		Expressions::Atoms ignoreList;
		act.effect->applyPositive(addList,ignoreList);
		for(Expressions::idexpr_t expr : addList){
			maximumList.insert(expr);
		}
	}
	Expressions::World* maximumWorld = new Expressions::World(0,maximumList);
	for(const Action &act : actions){
		if(act.precondition->isModeledBy(maximumWorld)){ WorldState::actions.push_back(act); }
	}
	delete maximumWorld;
	// Perform planning
	AStar::Path<WorldState> path = AStar::AStar(initialState,WorldState::goalFunction,AStar::defaultHeuristic,mets);
	for(const std::pair<AStar::idaction_t,WorldState*> &act : path){
		if(!act.first){ continue; }
		solution.push_back(Action::mapActions.at(act.first));
	}
	AStar::releasePath(path);
	delete initialState;
	return solution;
}

std::vector<std::vector<std::pair<std::string,std::string>>> DoradoPlanner::possibleParameters(PDDL::Problem* problem, const std::vector<std::pair<std::string,std::string>> &params, int curr){
	std::vector<std::vector<std::pair<std::string,std::string>>> permutations;
	int index = params.size() - curr;
	if(index){
		std::vector<std::vector<std::pair<std::string,std::string>>> nextPermutations = possibleParameters(problem,params,curr+1);
		for(const std::string &obj : problem->sets.at(params.at(index).second)){
			for(std::vector<std::pair<std::string,std::string>> perm : nextPermutations){
				perm.push_back({params.at(index).first,obj});
				permutations.push_back(perm);
			}
		}
	}else{
		for(const std::string &obj : problem->sets.at(params.at(index).second)){
			permutations.push_back({{params.at(index).first,obj}});
		}
	}
	return permutations;
}

#endif
