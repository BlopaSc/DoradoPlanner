#ifndef EXPRESSIONS_CPP
#define EXPRESSIONS_CPP
#define EXPRESSION_TYPE_OFFSET 32
#include "Expressions.h"

namespace Expressions{
	const char* andStr = "and";
	const char* orStr = "or";
	const char* notStr = "not";
	const char* equalsStr = "=";
	const char* implyStr = "imply";
	const char* whenStr = "when";
	const char* existsStr = "exists";
	const char* forallStr = "forall";
	
	namespace ExpressionType {
		const idtype_t NONE = 0x0;
		const idtype_t CONSTANT = 0x1;
		const idtype_t VARIABLE = 0x2;
		const idtype_t ATOM = 0x4;
		const idtype_t AND = 0x8;
		const idtype_t OR = 0x10;
		const idtype_t NOT = 0x20;
		const idtype_t EQUALS = 0x40;
		const idtype_t IMPLY = 0x80;
		const idtype_t WHEN = 0x100;
		const idtype_t EXISTS = 0x200;
		const idtype_t FORALL = 0x400;
		const idtype_t WORLD = 0x8000;
	};
	
	// Expression class
	WordMap Expression::words;
	ExpressionMap Expression::exprs;
	ReverseWordMap Expression::iwords;
	ReverseExpressionMap Expression::iexprs;
	Expression::Expression() : type(ExpressionType::NONE) {};
	Expression::Expression(idexpr_t k, idtype_t t) : key(k),type(t) {};
	Expression::~Expression(){};
	bool Expression::isModeledBy(World* world){ return false; }
	bool Expression::isLaxModeledBy(World* maxWorld,World* minWorld){ return false; }
	void Expression::apply(World* world,Atoms &addList,Atoms &removeList){}
	void Expression::applyPositive(Atoms &addList,Atoms &removeList){}
	Expression* Expression::substitute(idexpr_t o,idexpr_t n){ return this; }
	std::ostream& Expression::print(std::ostream& out) const { return out<<"Undefined"; }
	std::ostream& operator<<(std::ostream &out, Expression &e){ return e.print(out); }
	inline idexpr_t Expression::registerWord(const std::string &str){
		idexpr_t key = iwords[str];
		words[key] = str;
		return key;
	}
	inline Expression* Expression::registerConstant(const std::string &cnt){
		idexpr_t idcnt = registerWord(cnt);
		idexpr_t key = idcnt;
		Expression** exprPtr = &exprs[key];
		if(!*exprPtr){
			*exprPtr = new Constant(key,idcnt);
		}
		return *exprPtr;
	}
	inline Expression* Expression::registerVariable(const std::string &var,const std::string &grp){
		idexpr_t idvar = registerWord(var), idgrp, key;
		if(grp.empty()){
			idgrp = 0;
			key = idvar;
		}else{
			idgrp = registerWord(grp);
			Arguments args{idvar,idgrp};
			key = iexprs[args] | (ExpressionType::VARIABLE<<EXPRESSION_TYPE_OFFSET);
		}
		Expression** exprPtr = &exprs[key];
		if(!*exprPtr){
			*exprPtr = new Variable(key,idvar,idgrp);
		}
		return *exprPtr;
	}
	inline Expression* Expression::registerExpression(idtype_t type, Arguments &args){
		idexpr_t key = iexprs[args] | (type<<EXPRESSION_TYPE_OFFSET);
		Expression** exprPtr = &exprs[key];
		if(!*exprPtr){
			switch(type){
				case ExpressionType::ATOM:
					*exprPtr = new Atom(key,args);
					break;
				case ExpressionType::AND:
					*exprPtr = new And(key,args);
					break;
				case ExpressionType::OR:
					*exprPtr = new Or(key,args);
					break;
				case ExpressionType::NOT:
					*exprPtr = new Not(key,args);
					break;
				case ExpressionType::EQUALS:
					*exprPtr = new Equals(key,args);
					break;
				case ExpressionType::IMPLY:
					*exprPtr = new Imply(key,args);
					break;
				case ExpressionType::WHEN:
					*exprPtr = new When(key,args);
					break;
				case ExpressionType::EXISTS:
					*exprPtr = new Exists(key,args);
					break;
				case ExpressionType::FORALL:
					*exprPtr = new Forall(key,args);
					break;
			}
		}
		return *exprPtr;
	}
	
	// World class
	Groups World::groups;
	World::World(idexpr_t k, Atoms &a) : Expression(k, ExpressionType::WORLD) {
		atoms = std::move(a);
	}
	World* World::apply(Expression* action){
		Atoms addList;
		Atoms removeList;
		action->apply(this,addList,removeList);
		Atoms::iterator itRemove = removeList.begin();
		for(Atoms::iterator itAtoms = atoms.begin(); itAtoms!=atoms.end(); ++itAtoms){
			idexpr_t atom = *itAtoms;
			while(itRemove!=removeList.end() && *itRemove < atom){
				++itRemove;
			}
			if(itRemove==removeList.end() || atom < *itRemove){
				addList.insert(atom);
			}
		}
		idexpr_t key = iexprs[addList] | (ExpressionType::WORLD<<EXPRESSION_TYPE_OFFSET);
		Expression** exprPtr = &exprs[key];
		if(!*exprPtr){
			*exprPtr = new World(key,addList);
		}
		return (World*)(*exprPtr);
	}
	bool World::operator==(const World &other) const{
		return atoms==other.atoms;
	}
	std::ostream& World::print(std::ostream& out) const {
		out << "World: ";
		for(idexpr_t a : atoms){ out << *exprs.at(a) << " "; }
		return out << std::endl;
	}

	// Constant class
	Constant::Constant(idexpr_t k,idexpr_t c) : Expression(k ,ExpressionType::CONSTANT),constant(c) {};
	std::ostream& Constant::print(std::ostream& out) const {
		out << words.at(constant);
		return out;
	}
	
	// Variable class
	Variable::Variable(idexpr_t k,idexpr_t v,idexpr_t g) : Expression(k, ExpressionType::VARIABLE),variable(v),group(g) {};
	std::ostream& Variable::print(std::ostream& out) const {
		out << words.at(variable);
		if(group){
			out << " - " << words.at(group);
		}
		return out;
	}
	
	// Logical Expression class
	LogicalExpression::LogicalExpression(idexpr_t k,idtype_t t,Arguments &a) : Expression(k,t) { args = std::move(a); }
	Expression* LogicalExpression::substitute(idexpr_t o,idexpr_t n){
		Expression* result = this;
		idexpr_t newidExprs[0x100];
		int pos[0x100];
		int counter = 0;
		int newCount = 0;
		for(idexpr_t a : args){
			Expression* expr = exprs.at(a)->substitute(o,n);
			if(expr->key  != a){
				newidExprs[newCount] = expr->key;
				pos[newCount++] = counter;
			}
			counter++;
		}
		if(newCount){
			Arguments newArgs;
			pos[newCount] = 0x7FFFFFFF;
			counter = 0;
			newCount = 0;
			int next_pos = pos[0];
			for(idexpr_t a : args){
				if(counter==next_pos){
					newArgs.push_back(newidExprs[newCount]);
					next_pos = pos[++newCount];
				}else{
					newArgs.push_back(a);
				}
				counter++;
			}
			result = registerExpression(type,newArgs);
		}
		return result;
	}
	std::ostream& LogicalExpression::print(std::ostream& out) const {
		unsigned int i=1;
		out<<"(";
		switch(type){
			case ExpressionType::AND: out<<andStr; break;
			case ExpressionType::OR: out<<orStr; break;
			case ExpressionType::NOT: out<<notStr; break;
			case ExpressionType::EQUALS: out<<equalsStr; break;
			case ExpressionType::IMPLY: out<<implyStr; break;
			case ExpressionType::WHEN: out<<whenStr; break;
			case ExpressionType::EXISTS: out<<existsStr; break;
			case ExpressionType::FORALL: out<<forallStr; break;
			default: i=0;
		}
		bool addParenthesis = (type==ExpressionType::EXISTS || type == ExpressionType::FORALL);
		for(idexpr_t arg : args){
			if(i++){out << " ";}
			if(addParenthesis){ out << '('; }
			out << *exprs.at(arg);
			if(addParenthesis){ out << ')'; addParenthesis = false; }
		}
		return out<<")";
	}
	
	// Atom class
	Atom::Atom(idexpr_t k,Arguments &a) : LogicalExpression(k,ExpressionType::ATOM,a) {};
	bool Atom::isModeledBy(World* world){
		return world->atoms.find(key) != world->atoms.end();
	}
	bool Atom::isLaxModeledBy(World* maxWorld,World* minWorld){ return isModeledBy(maxWorld); }
	void Atom::apply(World* world,Atoms &addList,Atoms &removeList){
		addList.insert(key);
	}
	void Atom::applyPositive(Atoms &addList,Atoms &removeList){ addList.insert(key); }
	Expression* Atom::substitute(idexpr_t o,idexpr_t n){
		Expression* expr = this;
		bool subs = false;
		for(idexpr_t a : args){
			if(a==o){
				subs = true;
				break;
			}
		}
		if(subs){
			Arguments newArgs;
			for(idexpr_t a : args){
				newArgs.push_back(a==o?n:a);
			}
			expr = registerExpression(type,newArgs);
		}
		return expr;
	}

	// And class
	And::And(idexpr_t k,Arguments &a) : LogicalExpression(k,ExpressionType::AND,a) {};
	bool And::isModeledBy(World* world){
		for(idexpr_t a : args){
			if(!exprs.at(a)->isModeledBy(world)){ return false; }
		}
		return true;
	}
	bool And::isLaxModeledBy(World* maxWorld,World* minWorld){
		for(idexpr_t a : args){
			if(!exprs.at(a)->isLaxModeledBy(maxWorld,minWorld)){ return false; }
		}
		return true;
	}
	void And::apply(World* world,Atoms &addList,Atoms &removeList){
		for(idexpr_t a : args){
			exprs.at(a)->apply(world,addList,removeList);
		}
	}
	void And::applyPositive(Atoms &addList,Atoms &removeList){ for(idexpr_t a : args){ exprs.at(a)->applyPositive(addList,removeList); } }
	
	// Or class
	// Can't be applied, should throw error
	Or::Or(idexpr_t k,Arguments &a) : LogicalExpression(k,ExpressionType::OR,a) {};
	bool Or::isModeledBy(World* world){
		for(idexpr_t a : args){
			if(exprs.at(a)->isModeledBy(world)){ return true; }
		}
		return false;
	}
	bool Or::isLaxModeledBy(World* maxWorld,World* minWorld){
		for(idexpr_t a : args){
			if(exprs.at(a)->isLaxModeledBy(maxWorld,minWorld)){ return true; }
		}
		return false;
	}
	
	// Not class
	Not::Not(idexpr_t k,Arguments &a) : LogicalExpression(k,ExpressionType::NOT,a) {};
	bool Not::isModeledBy(World* world){
		return !exprs.at(args.front())->isModeledBy(world);
	}
	bool Not::isLaxModeledBy(World* maxWorld,World* minWorld){
		return !exprs.at(args.front())->isLaxModeledBy(minWorld,maxWorld);
	}
	void Not::apply(World* world,Atoms &addList,Atoms &removeList){
		exprs.at(args.front())->apply(world,removeList,addList);
	}
	void Not::applyPositive(Atoms &addList,Atoms &removeList){ exprs.at(args.front())->applyPositive(removeList,addList); }

	// Equals class
	// Can't be applied, should throw error
	Equals::Equals(idexpr_t k,Arguments &a) : LogicalExpression(k,ExpressionType::EQUALS,a) {};
	bool Equals::isModeledBy(World* world){ return args.front()==args.back(); }
	bool Equals::isLaxModeledBy(World* maxWorld,World* minWorld){ return args.front()==args.back(); }
	Expression* Equals::substitute(idexpr_t o,idexpr_t n){
		Expression* expr = this;
		bool subs = false;
		for(idexpr_t a : args){
			if(a==o){
				subs = true;
				break;
			}
		}
		if(subs){
			Arguments newArgs;
			for(idexpr_t a : args){
				newArgs.push_back(a==o?n:a);
			}
			expr = registerExpression(type,newArgs);
		}
		return expr;
	}
	
	// Imply class
	// Can't be applied, should throw error (an applied Imply is a When)
	Imply::Imply(idexpr_t k,Arguments &a) : LogicalExpression(k,ExpressionType::IMPLY,a) {};
	bool Imply::isModeledBy(World* world){ return !exprs.at(args.front())->isModeledBy(world) || exprs.at(args.back())->isModeledBy(world); }
	bool Imply::isLaxModeledBy(World* maxWorld,World* minWorld){ return !exprs.at(args.front())->isLaxModeledBy(minWorld,maxWorld) || exprs.at(args.back())->isLaxModeledBy(maxWorld,minWorld); }

	// When class
	// Can't be modeled, should throw error (a modeled When is an Imply)
	When::When(idexpr_t k,Arguments &a) : LogicalExpression(k,ExpressionType::WHEN,a) {};
	void When::apply(World* world,Atoms &addList,Atoms &removeList){
		if(exprs.at(args.front())->isModeledBy(world)){
			exprs.at(args.back())->apply(world, addList, removeList);
		}
	}
	void When::applyPositive(Atoms &addList,Atoms &removeList){ exprs.at(args.back())->applyPositive(addList,removeList); }

	// Exists class
	// Can't be applied, should throw error
	Exists::Exists(idexpr_t k,Arguments &a) : LogicalExpression(k,ExpressionType::EXISTS,a) {};
	bool Exists::isModeledBy(World* world){
		Variable* v = (Variable*)exprs.at(args.front());
		idexpr_t gid = v->group;
		for(idexpr_t member : world->groups.at(gid)){
			if(exprs.at(args.back())->substitute(v->variable,member)->isModeledBy(world)){ return true; }
		}
		return false;
	}
	bool Exists::isLaxModeledBy(World* maxWorld,World* minWorld){
		Variable* v = (Variable*)exprs.at(args.front());
		idexpr_t gid = v->group;
		for(idexpr_t member : maxWorld->groups.at(gid)){
			if(exprs.at(args.back())->substitute(v->variable,member)->isLaxModeledBy(maxWorld,minWorld)){ return true; }
		}
		return false;
	}
	
	// Forall class
	Forall::Forall(idexpr_t k,Arguments &a) : LogicalExpression(k,ExpressionType::FORALL,a) {};
	bool Forall::isModeledBy(World* world){
		Variable* v = (Variable*)exprs.at(args.front());
		idexpr_t gid = v->group;
		for(idexpr_t member : world->groups.at(gid)){
			if(!exprs.at(args.back())->substitute(v->variable,member)->isModeledBy(world)){ return false; }
		}
		return true;
	}
	bool Forall::isLaxModeledBy(World* maxWorld,World* minWorld){
		Variable* v = (Variable*)exprs.at(args.front());
		idexpr_t gid = v->group;
		for(idexpr_t member : maxWorld->groups.at(gid)){
			if(!exprs.at(args.back())->substitute(v->variable,member)->isLaxModeledBy(maxWorld,minWorld)){ return false; }
		}
		return true;
	}
	void Forall::apply(World* world,Atoms &addList,Atoms &removeList){
		Variable* v = (Variable*)exprs.at(args.front());
		idexpr_t gid = v->group;
		for(idexpr_t member : world->groups.at(gid)){
			exprs.at(args.back())->substitute(v->variable,member)->apply(world,addList,removeList);
		}
	}
	void Forall::applyPositive(Atoms &addList,Atoms &removeList){
		Variable* v = (Variable*)exprs.at(args.front());
		idexpr_t gid = v->group;
		for(idexpr_t member : World::groups.at(gid)){
			exprs.at(args.back())->substitute(v->variable,member)->applyPositive(addList,removeList);
		}
	}
	
	World* make_world(std::set<std::string> a,std::map<std::string,std::set<std::string>> g){
		Atoms atoms;
		Expression::words[0] = "";
		World::groups.clear();
		for(std::string s : a){
			Expression* e = make_expression(s);
			atoms.insert(e->key);
		}
		for(std::pair<std::string, std::set<std::string>> group: g){
			idexpr_t gKey = 0;
			if(!group.first.empty()){
				gKey = Expression::registerWord(group.first);
				
			}
			Atoms* members = &World::groups[gKey];
			for(const std::string &s : group.second){
				members->insert(Expression::registerWord(s));
			}
		}
		idexpr_t key = Expression::iexprs[atoms] | (ExpressionType::WORLD<<EXPRESSION_TYPE_OFFSET);
		Expression** exprPtr = &Expression::exprs[key];
		if(!*exprPtr){
			*exprPtr = new World(key,atoms);
		}
		return (World*)(*exprPtr);
	}
	
	// Probably can be optimized, wrote it half drunk
	Expression* make_expression(std::string expression){
		if(expression.front()=='(' && expression.back()==')'){ expression = expression.substr(1,expression.size()-2); }
		std::vector<std::string> fragments;
		Arguments arguments;
		idtype_t type=ExpressionType::ATOM;
		unsigned int anchor = 0;
		unsigned int size = expression.size();
		// Parse string into fragments
		while(anchor < size){
			unsigned int tmp;
			if(expression[anchor]=='('){
				unsigned int parenthesis = 0;
				tmp = anchor;
				do{
					if(expression[tmp]=='('){ parenthesis++; }
					if(expression[tmp]==')'){ parenthesis--; }
					tmp++;
				}while(parenthesis && tmp<size);
			}else{
				tmp = expression.find_first_of(' ',anchor);
				tmp = tmp<=size?tmp:size;
			}
			fragments.push_back(expression.substr(anchor,tmp-anchor));
			anchor = tmp+1;
		}
		// Parse fragments into arguments
		Expression* newExp;
		bool wasVar = false;
		size = fragments.size();
		for(unsigned int i=0;i<size;i++){
			std::string &frag = fragments[i];
			if(!arguments.size() && type==ExpressionType::ATOM){
				if(frag == andStr){
					type = ExpressionType::AND;
				}else if(frag == orStr){
					type = ExpressionType::OR;
				}else if(frag == notStr){
					type = ExpressionType::NOT;
				}else if(frag == equalsStr){
					type = ExpressionType::EQUALS;
				}else if(frag == implyStr){
					type = ExpressionType::IMPLY;
				}else if(frag == whenStr){
					type = ExpressionType::WHEN;
				}else if(frag == existsStr){
					type = ExpressionType::EXISTS;
				}else if(frag == forallStr){
					type = ExpressionType::FORALL;
				}
				if(type!=ExpressionType::ATOM){ continue; }
			}
			if(frag[0]=='('){
				// Expression
				newExp = make_expression(frag);
			}else if(frag[0]=='?'){
				// Variable
				wasVar = true;
				if(i+1<size && fragments[i+1][0]=='-'){
					if(i+2<size){
						newExp = Expression::registerVariable(frag,fragments[i+2]);
						i+=2;
					}else{
						newExp = Expression::registerVariable(frag,"");
						i+=1;
					}
				}else{
					newExp = Expression::registerVariable(frag,"");
				}
			}else{
				// Constant
				newExp = Expression::registerConstant(frag);
			}
			arguments.push_back(newExp->key);
		}
		// Must exclude any single argument structure
		if(arguments.size()==1 && wasVar && type!=ExpressionType::NOT){ 
			return newExp;
		}
		// Create expression from arguments
		return Expression::registerExpression(type,arguments);
	}
	
	inline idexpr_t get_idword(const std::string& s){
		return Expression::registerWord(s);
	}
	
	void releaseMemory(){
		Expression::words.clear();
		for(const std::pair<idexpr_t,Expression*>& exp : Expression::exprs){
			delete exp.second;
		}
		Expression::exprs.clear();
		Expression::iwords.clear();
		Expression::iexprs.clear();
	}
	
};

#endif
