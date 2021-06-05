#ifndef ASTAR_CPP
#define ASTAR_CPP
#include <algorithm>
#include <chrono>
#include <iomanip>
#include <iostream>
#include <limits>
#include <map>
#include <queue>
#include <utility>
#include <vector>

namespace AStar{
	const double INF = std::numeric_limits<double>::max();
	// Declarations
	using idstate_t = unsigned long long int;
	using idaction_t = unsigned long long int;
	class AStarMetrics;
	template <typename T> class Node;
	template <typename T> class Edge;
	template <typename T> class NodeState;
	template <typename T> using NodeNeighbors = std::vector<Edge<T>>;
	template <typename T> using Path = std::vector<std::pair<idaction_t,T*>>;
	template <typename T> double defaultHeuristic(T* state);
	template <typename T> Path<T> AStar(T* initialState,bool (*goalFunction)(T* state),double (*heuristicFunction)(T* state)=&defaultHeuristic,AStarMetrics* metrics=0);
	template <typename T> void releasePath(Path<T> solution);
	
	// Classes
	template <typename T> class Node{
		protected:
			T* state;
			idstate_t id;
		public:
			Node() : state(0), id(-1) {};
			Node(T* s) : state(s){
				id = state->getKey();
			};
			inline void release(T* s){ delete state; state = s; }
			inline T* getState(){ return state; }
			inline idstate_t getIdentifier(){ return id; }
			inline NodeNeighbors<T> getNeighbors(){ return state->getNeighbors(); };
	};
	
	template <typename T> class Edge{
		public:
			Node<T> state;
			double cost;
			idaction_t action;
			Edge(Node<T> s, double c) : state(s),cost(c),action(0) {};
			Edge(Node<T> s, double c, idaction_t a) : state(s),cost(c),action(a) {};
			inline bool operator< (const Edge<T>& other) const{ return cost < other.cost; }
	};	
	
	template <typename T> class NodeState{
		public:
			T* state;
			NodeState<T>* previous;
			double realCost;
			double hCost;
			idaction_t action;
			bool visited;
			bool path;
			NodeState() : state(0), previous(0), realCost(INF), hCost(INF), visited(false),path(false) {};
			NodeState(T* state) : state(state), previous(0), realCost(0), hCost(INF), visited(false),path(true) {};
	};
	
	class AStarMetrics{
		public:
			double timeTaken;
			// Frontier nodes: Number of known different states
			// Expanded nodes: Number of states that were expanded (their neighbors were requested)
			// Visited nodes: Number of states evaluated (might have been repeated/excluded)
			unsigned int frontierNodes;
			unsigned int expandedNodes;
			unsigned int visitedNodes;
			friend std::ostream& operator<<(std::ostream &out, AStarMetrics &mets){
				out << "Time taken: " << std::setprecision(3) << (mets.timeTaken/1000.0) << " s" << std::endl;
				out << "Frontier nodes: " << mets.frontierNodes << std::endl;
				out << "Expanded nodes: " << mets.expandedNodes << std::endl;
				return out << "Visited nodes: " << mets.visitedNodes << std::endl;
			}
	};
	
	template <typename T> double defaultHeuristic(T* state){
		return 0.0;
	}
	
	template <typename T> Path<T> AStar(T* initialState,bool (*goalFunction)(T* state),double (*heuristicFunction)(T* state),AStarMetrics* metrics){
		Path<T> solution;
		std::priority_queue<Edge<T>> frontier;
		std::map<idstate_t, NodeState<T>> knownStates;
		unsigned int expandedNodes = 0;
		unsigned int visitedNodes = 1;
		Node<T> initialNode(initialState);
		auto tStart = std::chrono::steady_clock::now();
		frontier.push({initialNode,0.0});
		knownStates.insert({initialNode.getIdentifier(),NodeState<T>(initialState)});
		Node<T> current;
		NodeState<T>* currentState;
		bool goal = false;
		while(!frontier.empty()){
			current = frontier.top().state;
			frontier.pop();
			currentState = &knownStates[current.getIdentifier()];
			if(goalFunction(current.getState())){
				goal = true;
				break;
			}
			if(currentState->visited){ continue; }
			NodeNeighbors<T> neighbors = current.getNeighbors();
			visitedNodes += neighbors.size();
			expandedNodes++;
			currentState->visited = true;
			for(Edge<T> neighbor : neighbors){
				NodeState<T> *neighborState = &knownStates[neighbor.state.getIdentifier()];
				if(neighborState->state!=0){
					neighbor.state.release(neighborState->state);
				}else{
					neighborState->hCost = heuristicFunction(neighborState->state = neighbor.state.getState());
				}
				if(neighborState->visited){
					continue;
				}
				if(currentState->realCost + neighbor.cost < neighborState->realCost){
					neighborState->action = neighbor.action;
					neighborState->previous = currentState;
					neighborState->realCost = currentState->realCost + neighbor.cost;
					frontier.push({neighbor.state,-(neighborState->realCost + neighborState->hCost)});
				}
			}
		}
		if(goal){
			while(currentState){
				currentState->path = true;
				solution.push_back({currentState->action,currentState->state});
				currentState = currentState->previous;
			}
			std::reverse(solution.begin(),solution.end());
		}
		for(std::pair<unsigned int, NodeState<T>> node : knownStates){
			if(!node.second.path){
				delete node.second.state;
			}
		}
		if(metrics){
			metrics->timeTaken = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - tStart).count();
			metrics->frontierNodes = knownStates.size();
			metrics->expandedNodes = expandedNodes;
			metrics->visitedNodes = visitedNodes;
		}
		return solution;
	}
	
	template <typename T> void releasePath(Path<T> solution){
		int counter = 0;
		for(std::pair<idaction_t,T*> step : solution){
			if(counter++){
				delete step.second;
			}
		}
	}
	
};
#endif
