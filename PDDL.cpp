#ifndef PDDL_CPP
#define PDDL_CPP
#include <fstream>
#include <iostream>
#include <map>
#include <set>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

namespace PDDL{
	
	class Domain;
	class Problem;
	
	std::string loadFile(const std::string &filename);
	std::vector<std::string> splitSections(const std::string &s);
	std::string flattenString(std::string s);
	Domain* parsePDDLDomain(const std::string &filename);
	Problem* parsePDDLProblem(const std::string &filename);
	
	const char* supported[] = {":strips",":typing",":disjunctive-preconditions",
								":equality",":existential-preconditions",
								":universal-preconditions",":quantified-preconditions",
								":conditional-effects",":adl", 0};
	
	class Domain{
		protected:
			static std::map<std::string,Domain*> domains;
		public:
			class Action{
				public:
					std::string name;
					std::string precondition;
					std::string effect;
					std::vector<std::pair<std::string,std::string>> parameters;
					friend std::ostream& operator<<(std::ostream &out, const Action &a);
			};
			std::string domain;
			std::vector<std::string> requirements;
			std::map<std::string,std::set<std::string>> itypes;
			std::map<std::string,std::set<std::string>> types;
			std::vector<Action> actions;
			friend std::ostream& operator<<(std::ostream &out, Domain &d);
			friend Domain* parsePDDLDomain(const std::string &filename);
			friend Problem* parsePDDLProblem(const std::string &filename);
	};
	
	class Problem{
		protected:
			static std::map<std::pair<std::string,std::string>,Problem*> problems;
		public:
			Domain* domain;
			std::string problem;
			std::string goal;
			std::set<std::string> init;
			std::map<std::string,std::set<std::string>> sets;
			friend std::ostream& operator<<(std::ostream &out, Problem &p);
			friend Problem* parsePDDLProblem(const std::string &filename);
	};
	
	// Domain class
	std::map<std::string,Domain*> Domain::domains;
	
	std::ostream& operator<<(std::ostream &out, const Domain::Action &a){
		int i=0;
		out << "\t(:action " << a.name << std::endl << "\t\t:parameters (";
		for(const std::pair<std::string,std::string> &param : a.parameters){ out << (i++?" ":"") << param.first << " - " << param.second; }
		return out << ")" << std::endl << "\t\t:precondition (" << a.precondition << ")" << std::endl << "\t\t:effect (" << a.effect << ")" << std::endl << "\t)";
	}
	
	std::ostream& operator<<(std::ostream &out, Domain &d){
		out << "(define " << std::endl;
		out << "\t(domain " << d.domain << ")" << std::endl;
		if(!d.requirements.empty()){
			out << "\t(:requirements";
			for(const std::string &s : d.requirements){ out << " " << s; }
			out << ")" << std::endl;
		}
		if(!d.types.empty()){
			out << "\t(:types" << std::endl;
			for(const std::pair<const std::string, const std::set<std::string>> &type : d.types){
				out <<"\t\t";
				for(const std::string &typ : type.second){ out << typ << " "; }
				out << "- " << type.first << std::endl;
			}
			out << "\t)" << std::endl;
		}
		for(Domain::Action &a : d.actions){ out << a << std::endl; }
		return out << ")";
	}
	
	// Problem class
	std::map<std::pair<std::string,std::string>,Problem*> Problem::problems;
	
	std::ostream& operator<<(std::ostream &out, Problem &p){
		out << "(define " << std::endl;
		out << "\t(problem " << p.problem << ")" << std::endl << "\t(:domain " << p.domain->domain << ")" << std::endl << "\t(:objects" << std::endl;
		for(const std::pair<std::string,std::set<std::string>> &sets : p.sets){
			out << "\t\t";
			for(const std::string &obj : sets.second){
				out << obj << " ";
			}
			out << "- " << sets.first << std::endl;
		}
		out << "\t)" << std::endl << "\t(:init" << std::endl;
		for(const std::string &atom : p.init){
			out << "\t\t(" << atom << ")" << std::endl;
		}
		return out << "\t)" << std::endl << "\t(:goal (" << p.goal << "))" << std::endl << ")";
	}
	
	// global functions
	std::string loadFile(const std::string &filename){
		std::ifstream file(filename);
		std::stringstream buffer;
		buffer << file.rdbuf();
		return buffer.str();
	}
	
	std::vector<std::string> splitSections(const std::string &s){
		std::vector<std::string> sections;
		size_t anchor;
		for(size_t reader = 0;reader<s.size();){
			anchor = reader;
			if(s.at(reader)==' ' || s.at(reader)=='\n' || s.at(reader)=='\r'){
				reader++;
				continue;
			}else if(s.at(reader)=='('){
				anchor++;
				for(int pars=1;pars;){
					reader++;
					if(s[reader]=='('){ ++pars; }
					if(s[reader]==')'){ --pars; }
				}
			}else if(s.at(reader)==';' && s.at(reader+1)==';'){
				while(reader<s.size() && s.at(reader)!='\n' && s.at(reader)!='\r'){ reader++; }
			}else{
				while(reader<s.size() && s.at(reader)!=' ' && s.at(reader)!='\n' && s.at(reader)!='\r'){ reader++; }
			}
			sections.push_back(s.substr(anchor,reader-anchor));
			reader++;
		}
		return sections;
	}
	
	std::string flattenString(std::string s){
		std::string result;
		bool flag = false;
		for(char c : s){
			if(c!='\n' && c!='\r' && c!=' '){
				if(flag){ result += ' '; }
				result += c;
				flag = false;
			}else if(!flag){
				flag = true;
			}
		}
		return result;
	}
	
	// TODO: Make safe -> adds exceptions/errors
	Domain* parsePDDLDomain(const std::string &filename){
		Domain* domain = 0;
		std::string domainStr = loadFile(filename);
		size_t lPar = domainStr.find_first_of('(');
		size_t anchor = domainStr.find_last_of(')');
		domainStr = domainStr.substr(lPar+1, anchor-(lPar+1));
		std::vector<std::string> sections = splitSections(domainStr);
		for(std::string &section : sections){
			if(section=="define" || (section.at(0)==';' && section.at(1)==';')){ continue; }
			std::vector<std::string> subsections = splitSections(section);
			if(subsections.empty()){ continue; }
			if(subsections[0]=="domain"){
				Domain** domainPtr = &Domain::domains[subsections[1]];
				if(!*domainPtr){
					*domainPtr = new Domain();
					domain = *domainPtr;
				}else{
					domain = *domainPtr;
					break;
				}
				domain->domain = subsections[1];
			}
			if(!domain){ continue; }
			if(subsections[0]==":extends"){
				Domain* domainPtr = Domain::domains.at(subsections[1]);
				if(domainPtr){
					domain->requirements = domainPtr->requirements;
					domain->types = domainPtr->types;
					domain->itypes = domainPtr->itypes;
					domain->actions = domainPtr->actions;
					// domain->constants = domainPtr->constants;
					// domain->predicates = domainPtr->predicates;
				}else{
					// TODO: Throw exception
				}
			}
			if(subsections[0]==":requirements"){
				domain->requirements = std::vector<std::string>(++subsections.begin(),subsections.end());
				// TODO: Verify requirements, throw exception
			}
			if(subsections[0]==":types"){
				std::vector<std::string> types;
				bool parent = false;
				for(int i=1;i<subsections.size();i++){
					if(subsections[i]=="-"){
						parent = true;
					}else if(parent){
						for(const std::string &typ : types){
							domain->types[subsections[i]].insert(typ);
							domain->itypes[typ].insert(subsections[i]);
						}
						types.clear();
						parent = false;
					}else{
						types.push_back(subsections[i]);
					}
				}
				for(const std::string &typ : types){ domain->itypes[typ]; }
			}
			if(subsections[0]==":constants"){
				// TODO: Implement, constant objects present in all problems of the domain
			}
			if(subsections[0]==":action"){
				Domain::Action newAction;
				newAction.name = subsections[1];
				for(int i=2;i<subsections.size()-1;i++){
					if(subsections[i]==":parameters"){
						std::vector<std::string> params = splitSections(subsections[++i]);
						std::vector<std::string> queue;
						bool type = false;
						for(int j=0;j<params.size();j++){
							if(params[j]=="-"){
								type=true;
							}else if(type){
								// TODO: Add support for *either*
								type = false;
								for(const std::string &elem : queue){ newAction.parameters.push_back({elem,params[j]}); }
								queue.clear();
							}else{
								queue.push_back(params[j]);
							}
						}
						for(const std::string &elem : queue){ newAction.parameters.push_back({elem,""}); }
					}else if(subsections[i]==":precondition"){
						newAction.precondition = flattenString(subsections[++i]);
					}else if(subsections[i]==":effect"){
						newAction.effect = flattenString(subsections[++i]);
					}
				}
				domain->actions.push_back(newAction);
			}
			if(subsections[0]==":domain-variables" || subsections[0]==":timeless" || subsections[0]==":axiom" || subsections[0]==":safety"){
				// TODO: Throw unsupported
			}
			if(subsections[0]==":predicates"){
				// TODO: Support predicates
			}
		}
		return domain;
	}
	
	Problem* parsePDDLProblem(const std::string &filename){
		Problem* problem = 0;
		std::string problemStr = loadFile(filename);
		size_t lPar = problemStr.find_first_of('(');
		size_t anchor = problemStr.find_last_of(')');
		problemStr = problemStr.substr(lPar+1, anchor-(lPar+1));
		std::vector<std::string> sections = splitSections(problemStr);
		std::string tmpStr;
		for(std::string &section : sections){
			if(section=="define" || (section.at(0)==';' && section.at(1)==';')){ continue; }
			std::vector<std::string> subsections = splitSections(section);
			if(subsections.empty()){ continue; }
			if(subsections[0]=="problem" || subsections[0]==":domain"){
				if(tmpStr.empty()){
					tmpStr = subsections[1];
				}else{
					std::pair<std::string,std::string> key = (subsections[0].at(0)==':')?std::pair<std::string,std::string>{subsections[1],tmpStr}:std::pair<std::string,std::string>{tmpStr,subsections[1]};
					Problem** problemPtr = &Problem::problems[key];
					if(!*problemPtr){
						*problemPtr = new Problem();
						(*problemPtr)->domain = Domain::domains.at((subsections[0].at(0)==':')?subsections[1]:tmpStr);
						problem = *problemPtr;
					}else{
						problem = *problemPtr;
						break;
					}
				}
			}
			if(!problem){ continue; }
			if(subsections[0]==":objects"){
				std::vector<std::string> queue;
				bool type = false;
				for(int i=1;i<subsections.size();i++){
					if(subsections[i]=="-"){
						type=true;
					}else if(type){
						type = false;
						for(const std::string &elem : queue){
							problem->sets[subsections[i]].insert(elem);
							problem->sets[""].insert(elem);
						}
						queue.clear();
					}else{
						queue.push_back(subsections[i]);
					}
				}
				for(const std::string &elem : queue){ problem->sets[""].insert(elem); }
				for(int i=0;i<problem->domain->types.size();i++){
					for(const std::pair<std::string,std::set<std::string>> &set : problem->domain->types){
						for(const std::string &type : set.second){
							for(const std::string &elem : problem->sets.at(type)){
								problem->sets[set.first].insert(elem);
							}
						}
					}
				}
			}
			if(subsections[0]==":init"){
				for(int i=1;i<subsections.size();i++){
					problem->init.insert(subsections.at(i));
				}
			}
			if(subsections[0]==":goal"){
				problem->goal = flattenString(subsections[1]);
			}
		}
		return problem;
	}
};

#endif
