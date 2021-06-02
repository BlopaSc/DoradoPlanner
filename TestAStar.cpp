#include "AStar.cpp"
#include <iostream>

#define HUMAN "H "
#define WOLF "W "
#define SHEEP "S "
#define VEG "V "
#define RIVER "||| "
#define E "_ "

using std::cout;

class RiverState{
	public:
		char human;
		char wolf;
		char sheep;
		char vegetable;
		RiverState() : human(false),wolf(false),sheep(false),vegetable(false){};
		RiverState(char h,char w,char s,char v) : human(h),wolf(w),sheep(s),vegetable(v){}; 
		friend std::ostream& operator<<(std::ostream& os, const RiverState& st);
		AStar::NodeNeighbors<RiverState> getNeighbors(){
			AStar::NodeNeighbors<RiverState> neighbors;
			if(human==vegetable && !(wolf==sheep)){
				RiverState* newState = new RiverState(!human,wolf,sheep,!vegetable);
				neighbors.push_back({newState,1.0,4});
			}
			if(human==sheep){
				RiverState* newState = new RiverState(!human,wolf,!sheep,vegetable);
				neighbors.push_back({newState,1.0,3});
			}
			if(human==wolf && !(sheep==vegetable)){
				RiverState* newState = new RiverState(!human,!wolf,sheep,vegetable);
				neighbors.push_back({newState,1.0,2});
			}
			if(!(wolf==sheep) && !(sheep==vegetable)){
				RiverState* newState = new RiverState(!human,wolf,sheep,vegetable);
				neighbors.push_back({newState,1.0,1});
			}
			return neighbors;
		}
		AStar::idstate_t getKey(){
			return (human | wolf<<1 | sheep<<2 | vegetable<<3);
		}
};

std::ostream& operator<< (std::ostream &out, const RiverState &st){
	out << 
		(!st.human?HUMAN:E) << (!st.wolf?WOLF:E) << (!st.sheep?SHEEP:E) << (!st.vegetable?VEG:E) << 
		RIVER <<
		(st.human?HUMAN:E) << (st.wolf?WOLF:E) << (st.sheep?SHEEP:E) << (st.vegetable?VEG:E);
	return out;
}

bool solved(RiverState* s){
	return s->human && s->wolf && s->sheep && s->vegetable;
}

int main(){
	
	RiverState* initState = new RiverState();
	
	cout<<"Initial state:"<<std::endl<<*initState<<std::endl;
	bool leakTest = true;
	for(int i=0;i<(leakTest?1000000:1);i++){
		AStar::Path<RiverState> solution = AStar::AStar(initState,solved);
		if(!leakTest){
			cout<<"Solution size: "<<solution.size()<<std::endl;
			int steps = 0;
			for(std::pair<AStar::idaction_t,RiverState*> step : solution){
				cout<<"Step "<<(steps++)<<": "<<(*(step.second))<<"\t\tAction: "<<(step.first)<<std::endl;
			}
		}
		AStar::releasePath(solution);
	}
	delete initState;
	
	cout << "Type and press ENTER...";
	char c;
	std::cin>>c;
	return 0;
}
