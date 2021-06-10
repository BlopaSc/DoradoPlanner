#ifndef PLANNER_CPP
#define PLANNER_CPP
#include "Planner.h"
#include "Heuristics.cpp"

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
DoradoPlanner::WorldState::WorldState() : world(0) {};
DoradoPlanner::WorldState::WorldState(Expressions::World* w) : world(w) {};
DoradoPlanner::WorldState::~WorldState() {};
AStar::idstate_t DoradoPlanner::WorldState::getKey(){ return world->key; }
AStar::NodeNeighbors<DoradoPlanner::WorldState> DoradoPlanner::WorldState::getNeighbors(){
	AStar::NodeNeighbors<DoradoPlanner::WorldState> neighbors;
	for(const Action &act : actions){
		if(act.precondition->isModeledBy(world)){
			Expressions::World* w = world->apply(act.effect);
			neighbors.push_back({WorldState(w),1.0,act.actionid});
		}
	}
	return neighbors;
}
bool DoradoPlanner::WorldState::goalFunction(const WorldState& state){
	return goal->isModeledBy(state.world);
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
		Expressions::Expression* precondition = Expressions::make_expression(act.precondition);
		Expressions::Expression* effect = Expressions::make_expression(act.effect);
		if(!act.parameters.size()){
			actions.push_back({act.name,precondition,effect});
			continue;
		}
		std::vector<std::vector<std::pair<std::string,std::string>>> params = possibleParameters(problem, act.parameters);
		for(const std::vector<std::pair<std::string,std::string>> &paramPerm : params){
			std::string name;
			Expressions::Expression* preconditionGrounded = precondition;
			Expressions::Expression* effectGrounded = effect;
			for(const std::pair<std::string,std::string> &paramSubstitution : paramPerm){
				name =  " " +paramSubstitution.second + name;
				Expressions::idexpr_t var = Expressions::get_idword(paramSubstitution.first);
				Expressions::idexpr_t grounded = Expressions::get_idword(paramSubstitution.second);
				preconditionGrounded = preconditionGrounded->substitute(var,grounded);
				effectGrounded = effectGrounded->substitute(var,grounded);
			}
			name = act.name + name;
			actions.push_back({name,preconditionGrounded,effectGrounded});
		}
	}
	WorldState initialState(Expressions::make_world(problem->init,problem->sets));
	WorldState::goal = Expressions::make_expression(problem->goal);
	// Remove impossible actions
	Expressions::Atoms maximumList = initialState.world->atoms;
	Expressions::Atoms minimumList = initialState.world->atoms;
	for(const Action &act : actions){
		Expressions::Atoms addList;
		Expressions::Atoms removeList;
		act.effect->applyPositive(addList,removeList);
		for(Expressions::idexpr_t expr : addList){
			maximumList.insert(expr);
		}
		for(Expressions::idexpr_t expr : removeList){
			minimumList.erase(expr);
		}
	}
	Expressions::World* maximumWorld = new Expressions::World(0,maximumList);
	Expressions::World* minimumWorld = new Expressions::World(0,minimumList);
	for(const Action &act : actions){
		if(act.precondition->isLaxModeledBy(maximumWorld,minimumWorld)){ WorldState::actions.push_back(act); }
	}
	delete maximumWorld;
	delete minimumWorld;
	// TODO: Smart choose heuristic
	Heuristics::setGoal(WorldState::goal);
	//double (*heuristic)(const WorldState& state) = &AStar::defaultHeuristic;
	double (*heuristic)(const WorldState& state) = &Heuristics::atomDistanceHeuristics;
	// Perform planning
	AStar::Path<WorldState> path = AStar::AStar(initialState,WorldState::goalFunction,heuristic,mets);
	for(const std::pair<AStar::idaction_t,WorldState> &act : path){
		if(!act.first){ continue; }
		solution.push_back(Action::mapActions.at(act.first));
	}
	return solution;
}

std::vector<std::vector<std::pair<std::string,std::string>>> DoradoPlanner::possibleParameters(PDDL::Problem* problem, const std::vector<std::pair<std::string,std::string>> &params, int curr){
	std::vector<std::vector<std::pair<std::string,std::string>>> permutations;
	int index = curr;
	if(index+1<params.size()){
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
