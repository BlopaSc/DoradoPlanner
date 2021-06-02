#include "Expressions.cpp"
#include <stdio.h>

using Expressions::World;
using Expressions::Expression;
using Expressions::make_world;
using Expressions::make_expression;
using Expressions::get_idword;
using Expressions::idexpr_t;

int passed = 0;
int tests = 0;

bool leakTest = true;

void error(std::string s){
	std::cout<<"Error: "<<s<<std::endl;
}

void run_test_simple(const char* testName, std::set<std::string> atoms,std::string expression,bool expectedResult,std::map<std::string,std::set<std::string>> sets = {}){
	World* world = make_world(atoms,sets);
	Expression* expr = make_expression(expression);
	bool result = expr->isModeledBy(world);
	if(!leakTest){ std::cout<<"Test "<<testName<<": "<<(result==expectedResult?"PASSED":"FAILED")<<std::endl; }
	tests++;
	if(result==expectedResult){ passed++; }
}

void run_test_apply(const char* testName, std::set<std::string> atoms,std::string action,std::string expression,bool expectedResult,std::map<std::string,std::set<std::string>> sets = {}){
	World* world = make_world(atoms,sets);
	Expression* act = make_expression(action);
	Expression* expr = make_expression(expression);
	World* newWorld = world->apply(act);
	bool result = expr->isModeledBy(newWorld);
	if(!leakTest){ std::cout<<"Test "<<testName<<": "<<(result==expectedResult?"PASSED":"FAILED")<<std::endl; }
	tests++;
	if(result==expectedResult){ passed++; }
}

class Action{
	public:
		std::string precond;
		std::string effect;
		std::vector<std::pair<std::string,std::string>> params;
		Action(std::string pc,std::string e,std::vector<std::pair<std::string,std::string>> p) : precond(pc),effect(e),params(p) {};
};

void run_test_executeplan(const char* testName, std::set<std::string> atoms, std::vector<Action> plan, std::string expression,bool expectedResult,std::map<std::string,std::set<std::string>> sets = {}){
	World* world = make_world(atoms,sets);
	bool result = true;
	for(Action a : plan){
		Expression* preexp = make_expression(a.precond);
		Expression* effexp = make_expression(a.effect);
		for(std::pair<const std::string&,const std::string&> par : a.params){
			idexpr_t var = get_idword(par.first);
			idexpr_t grounded = get_idword(par.second);
			preexp = preexp->substitute(var,grounded);
			effexp = effexp->substitute(var,grounded);
		}
		if(!preexp->isModeledBy(world)){
			result = false;
			break;
		}
		tests++;
		passed++;
		World* newWorld = world->apply(effexp);
		world = newWorld;
	}
	tests++;
	if(result){
		Expression* exp = make_expression(expression);
		result = exp->isModeledBy(world);
		if(result==expectedResult){ passed++; }
	}
	if(!leakTest){ std::cout<<"Test "<<testName<<": "<<(result==expectedResult?"PASSED":"FAILED")<<std::endl; }
}

int main(){
	std::set<std::string> atoms;
	std::map<std::string,std::set<std::string>> sets;
	std::string expr;
	std::string action;
	std::string pboard,eboard,pserve,eserve,pup,eup,pdown,edown,goal,pstop,estop;
	std::string pload,eload,punload,eunload,pdrive,edrive;

	for(int i=0;i<(leakTest?10000:1);i++){

	atoms = {"(on a b)", "(on b c)", "(on c d)"};

	run_test_simple("simple AND", atoms, "(and (not (on a b)) (on a c))", false);
	run_test_simple("simple OR", atoms, "(or (on a b) (on a d))", true);
	run_test_simple("simple NOT", atoms, "(not (on a b))", false);
	run_test_simple("simple IMPLY", atoms, "(imply (on a b) (on b a))", false);
	run_test_simple("simple EXISTS", atoms, "(exists (?v - ) (on a ?v))", true, { {"", {"a", "b", "c"}} });
	run_test_simple("simple FORALL", atoms, "(forall (?v - restricted) (on ?v b))", true, { {"", {"a", "b", "c"}}, {"restricted", {"a"}} });
	
	atoms = { "(at store mickey)", "(at airport minny)" };
	sets = {{"Locations", {"home", "park", "store", "airport", "theater"}}, {"", {"home", "park", "store", "airport", "theater", "mickey", "minny"}}};
	expr = "(and " 
    "(not (at park mickey)) " 
    "(or "
          "(at home mickey) " 
          "(at store mickey) " 
          "(at theater mickey) " 
          "(at airport mickey)) " 
    "(imply "
              "(friends mickey minny) " 
              "(forall " 
                        "(?l - Locations) "
                        "(imply "
                                "(at ?l mickey) "
                                "(at ?l minny)))))";
    run_test_simple("simple Mickey", atoms, expr, true, sets);

	if(passed!=tests){ error("Failed simple tests"); goto end; }
	
	run_test_apply("apply Mickey not friends", atoms, "(friends mickey minny)", expr, false, sets);
	atoms = { "(at store mickey)", "(at airport minny)", "(friends mickey minny)" };
	run_test_apply("apply Mickey move friends", atoms, "(and (at store minny) (not (at airport minny)))", expr, true, sets);
	
	atoms = {"(at store mickey)", "(at store minny)", "(friends mickey minny)"};
	action = "(and "
	"(at home mickey) "
	"(not (at store mickey)) "
	"(when "
		"(at store minny) "
		"(and "
			"(at home minny) "
			"(not (at store minny)))))";
			
	run_test_apply("apply Mickey collocated", atoms, action, expr, true, sets);
	
	atoms = {"(at store mickey)", "(at airport minny)", "(friends mickey minny)"};
	run_test_apply("apply Mickey not collocated", atoms, action, expr, false, sets);
	
	if(passed!=tests){ error("Failed apply tests"); goto end; }
	
	atoms = {"(above f0 f1)", "(above f0 f2)", "(above f0 f3)", "(above f1 f2)", "(above f1 f3)", 
				"(above f2 f3)", "(origin p0 f3)", "(destin p0 f2)", "(origin p1 f1)", "(destin p1 f3)", "(lift-at f0)"};
	
	pboard = "(and (lift-at ?f) (origin ?p ?f))";  
	eboard = "(boarded ?p)"; 
	
	pserve = "(and (lift-at ?f) (destin ?p ?f) (boarded ?p))";
	eserve = "(and (not (boarded ?p)) (served ?p))";
	
	pup = "(and (lift-at ?f1) (above ?f1 ?f2))";
	eup = "(and (lift-at ?f2) (not (lift-at ?f1)))";
	
	pdown = "(and (lift-at ?f1) (above ?f2 ?f1))";
	edown = "(and (lift-at ?f2) (not (lift-at ?f1)))";
	
	goal = "(served p0)";
	
	run_test_executeplan("execute plan elevator strips 1",atoms, 
					{{pup,eup,{{"?f1", "f0"}, {"?f2", "f3"}}},
                     {pboard,eboard,{{"?f", "f3"}, {"?p", "p0"}}},
                     {pdown,edown,{{"?f1", "f3"}, {"?f2", "f2"}}},
                     {pserve,eserve,{{"?f", "f2"}, {"?p", "p0"}}}}, goal, true);
	
	goal = "(and (served p0) (served p1))";

	run_test_executeplan("execute plan elevator strips 2",atoms, 
					{{pup,eup,{{"?f1", "f0"}, {"?f2", "f1"}}},
                      {pboard,eboard,{{"?f", "f1"}, {"?p", "p1"}}},
                      {pup,eup,{{"?f1", "f1"}, {"?f2", "f3"}}},
                      {pboard,eboard,{{"?f", "f3"}, {"?p", "p0"}}},
                      {pserve,eserve,{{"?f", "f3"}, {"?p", "p1"}}},
                      {pdown,edown,{{"?f1", "f3"}, {"?f2", "f2"}}},
                      {pserve,eserve,{{"?f", "f2"}, {"?p", "p0"}}}}, goal, true);
	
	sets = {{"passenger", {"p0", "p1"}}, {"", {"p0", "p1"}}};
	goal = "(forall (?p - passenger) (served ?p))";
	run_test_executeplan("execute plan elevator strips forall",atoms, 
					{{pup,eup,{{"?f1", "f0"}, {"?f2", "f1"}}},
                      {pboard,eboard,{{"?f", "f1"}, {"?p", "p1"}}},
                      {pup,eup,{{"?f1", "f1"}, {"?f2", "f3"}}},
                      {pboard,eboard,{{"?f", "f3"}, {"?p", "p0"}}},
                      {pserve,eserve,{{"?f", "f3"}, {"?p", "p1"}}},
                      {pdown,edown,{{"?f1", "f3"}, {"?f2", "f2"}}},
                      {pserve,eserve,{{"?f", "f2"}, {"?p", "p0"}}}}, goal, true, sets);
	
	goal = "(not (exists (?p - passenger) (not (served ?p))))";
	run_test_executeplan("execute plan elevator strips exists",atoms, 
					{{pup,eup,{{"?f1", "f0"}, {"?f2", "f1"}}},
                      {pboard,eboard,{{"?f", "f1"}, {"?p", "p1"}}},
                      {pup,eup,{{"?f1", "f1"}, {"?f2", "f3"}}},
                      {pboard,eboard,{{"?f", "f3"}, {"?p", "p0"}}},
                      {pserve,eserve,{{"?f", "f3"}, {"?p", "p1"}}},
                      {pdown,edown,{{"?f1", "f3"}, {"?f2", "f2"}}},
                      {pserve,eserve,{{"?f", "f2"}, {"?p", "p0"}}}}, goal, true, sets);
	
	pstop = "(lift-at ?f)";
	estop = "(and "
               "(forall (?p - passenger) "
                  "(when (and (boarded ?p) "
                             "(destin ?p ?f)) "
                        "(and (not (boarded ?p)) "
                             "(served ?p)))) "
               "(forall (?p - passenger) "
                   "(when (and (origin ?p ?f) (not (served ?p))) "
                              "(boarded ?p))))";
	
	goal = "(forall (?p - passenger) (served ?p))";
	run_test_executeplan("execute plan elevator adl 1",atoms, 
					{{pup,eup,{{"?f1", "f0"}, {"?f2", "f1"}}},
                      {pstop,estop,{{"?f", "f1"}}}}, "(boarded p1)", true, sets);
                      
    run_test_executeplan("execute plan elevator adl 2",atoms, 
					{{pup,eup,{{"?f1", "f0"}, {"?f2", "f1"}}},
                      {pstop,estop,{{"?f", "f1"}}},
                      {pup,eup,{{"?f1", "f1"}, {"?f2", "f3"}}},
                      {pstop,estop,{{"?f", "f3"}}},
                      {pdown,edown,{{"?f1", "f3"}, {"?f2", "f2"}}},
                      {pstop,estop,{{"?f", "f2"}}}}, goal, true, sets);
	
	

	pload = "(and (at ?t ?l) (at ?p ?l) (free ?a1 ?t) "
  		    "(forall (?a2 - truckarea) "
  			    "(imply (closer ?a2 ?a1) (free ?a2 ?t))))";
  	eload = "(and (not (at ?p ?l)) (not (free ?a1 ?t)) (in ?p ?t ?a1))";
  	punload = "(and (at ?t ?l) (in ?p ?t ?a1) "
  		    "(forall (?a2 - truckarea) "
  			    "(imply (closer ?a2 ?a1) (free ?a2 ?t))))";
	eunload = "(and (not (in ?p ?t ?a1)) (free ?a1 ?t) (at ?p ?l))";
	pdrive = "(and (at ?t ?from) (connected ?from ?to))";
	edrive = "(and (not (at ?t ?from)) (at ?t ?to))";
	atoms = {"(at truck1 l3)", "(free a1 truck1)", "(free a2 truck1)","(closer a1 a2)", "(at package1 l2)", "(at package2 l2)", "(at package3 l2)",
         "(connected l1 l2)", "(connected l1 l3)", "(connected l2 l1)", "(connected l2 l3)","(connected l3 l1)", "(connected l3 l2)"};
    sets = {{"trucks", {"truck1", "truck2"}}, {"truckarea", {"a1", "a2"}}, {"", {"a1", "a2", "truck1", "truck2"}}};
	goal = "(and (at package1 l1) (at package2 l3))";
	
	run_test_executeplan("execute plan truck",atoms, 
					{{pdrive,edrive,{{"?t", "truck1"}, {"?from", "l3"}, {"?to", "l2"}}},
                      {pload,eload,{{"?t", "truck1"}, {"?l", "l2"}, {"?p", "package1"}, {"?a1", "a2"}}},
                      {pload,eload,{{"?t", "truck1"}, {"?l", "l2"}, {"?p", "package2"}, {"?a1", "a1"}}},
                      {pdrive,edrive,{{"?t", "truck1"}, {"?from", "l2"}, {"?to", "l3"}}},
                      {punload,eunload,{{"?t", "truck1"}, {"?l", "l3"}, {"?p", "package2"}, {"?a1", "a1"}}},
                      {pdrive,edrive,{{"?t", "truck1"}, {"?from", "l3"}, {"?to", "l1"}}},
                      {punload,eunload,{{"?t", "truck1"}, {"?l", "l1"}, {"?p", "package1"}, {"?a1", "a2"}}}}, goal, true, sets);
	
	if(passed!=tests){ error("Failed plan execution tests"); goto end; }
	
	atoms = {"(has a b)", "(has b a)", "(has a c)", "(has c a)"};
	expr = "(forall (?v - ) (forall (?v1 - ) (has ?v ?v1)))";
	sets = {{"",{"a","b","c"}}};
	run_test_simple("nested forall false", atoms, expr, false, sets);
	atoms = {"(has a b)", "(has a a)", "(has b a)", "(has b b)", "(has a c)", "(has c a)", "(has c c)", "(has b c)", "(has c b)"};
	run_test_simple("nested forall true", atoms, expr, true, sets);

	Expressions::releaseMemory();

	}

end:
	
	atoms = {"(on a b)", "(on b c)", "(on c d)"};
	sets = { {"", {"a", "b", "c"}} };
	
	World* w1 = make_world({"(on a b)", "(on b c)", "(on c d)"},{{"", {"a", "b", "c", "d", "e"}}});
	World* w2 = make_world({"(on a b)", "(on b c)", "(on c d)"},{{"", {"a", "b", "c", "d", "e"}}});
	World* w3 = make_world({"(on a c)", "(on b c)", "(on c d)"},{{"", {"a", "b", "c", "d", "e"}}});
	World* w4 = make_world({"(on a b)", "(on b c)", "(on c d)", "(on d e)"},{{"", {"a", "b", "c", "d", "e"}}});
	
	std::cout << "Comparing worlds:" << std::endl;
	std::cout << "w1 == w2 (True)? " << ((*w1==*w2)?"True":"False") << std::endl;
	std::cout << "w1 == w3 (False)? " << ((*w1==*w3)?"True":"False") << std::endl;
	std::cout << "w1 == w4 (False)? " << ((*w1==*w4)?"True":"False") << std::endl;
	
	std::cout<<"Total passed: "<<passed<<"/"<<tests<<std::endl;
	printf("Press ENTER...");
	fgetc(stdin);
	return 0;
}
