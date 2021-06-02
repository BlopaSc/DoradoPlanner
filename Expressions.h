#ifndef EXPRESSIONS_H
#define EXPRESSIONS_H

#include "ExpressionsDictionary.cpp"
#include <cstdint>
#include <functional>
#include <iostream>
#include <map>
#include <set>
#include <string>
#include <vector>

namespace Expressions{
	// Class, type and constants definitions
	class Expression;
	class Constant;
	class Variable;
	class LogicalExpression;
	class Atom;
	class And;
	class Or;
	class Not;
	class Equals;
	class Imply;
	class When;
	class Exists;
	class Forall;
	class World;
	
	using idexpr_t = uint64_t;
	using idtype_t = uint64_t;
	using WordMap = std::map<idexpr_t,std::string>;
	using ExpressionMap = std::map<idexpr_t,Expression*>;
	using ReverseWordMap = ExpressionsDictionary::Trie<const char,idexpr_t>;
	using ReverseExpressionMap = ExpressionsDictionary::Trie<idexpr_t,idexpr_t>;
	using Arguments = std::vector<idexpr_t>;
	using Atoms = std::set<idexpr_t>;
	using Groups = std::map<idexpr_t,Atoms>;
	
	World* make_world(std::set<std::string> atoms,std::map<std::string,std::set<std::string>> groups);
	Expression* make_expression(std::string expression);
	Expression* make_substitution(Expression* original,const std::string &oldValue,const std::string &newValue);
	inline idexpr_t get_idword(const std::string& s);
	
	extern const char* andStr;
	extern const char* orStr;
	extern const char* notStr;
	extern const char* equalsStr;
	extern const char* implyStr;
	extern const char* whenStr;
	extern const char* existsStr;
	extern const char* forallStr;
	
	class Expression{
		protected:
			static WordMap words;
			static ReverseWordMap iwords;
			static ExpressionMap exprs;
			static ReverseExpressionMap iexprs;
			static inline idexpr_t registerWord(const std::string &str);
			static inline Expression* registerConstant(const std::string &cnt);
			static inline Expression* registerVariable(const std::string &var,const std::string &grp);
			static inline Expression* registerExpression(idtype_t type, Arguments &args);
		public:
			idexpr_t key;
			idtype_t type;
			Expression();
			Expression(idexpr_t k, idtype_t t);
			virtual ~Expression();
			virtual bool isModeledBy(World* world);
			virtual void apply(World* world,Atoms &addList,Atoms &removeList);
			virtual void applyPositive(Atoms &addList,Atoms &removeList);
			virtual Expression* substitute(idexpr_t o,idexpr_t n);
			virtual std::ostream& print(std::ostream& out) const;
			friend std::ostream& operator<<(std::ostream &out, Expression &e);
			friend Expression* make_expression(std::string expression);
			friend World* make_world(std::set<std::string> atoms,std::map<std::string,std::set<std::string>> groups);
			friend inline idexpr_t get_idword(const std::string& s);
			friend void releaseMemory();
	};
	
	class World : public Expression{
		public:
			static Groups groups;
			Atoms atoms;
			World(idexpr_t k, Atoms &a);
			World* apply(Expression* action);
			bool operator==(const World &other) const;
			std::ostream& print(std::ostream& out) const;
	};
	
	class Constant : public Expression{
		public:
			idexpr_t constant;
			Constant(idexpr_t k,idexpr_t c);
			std::ostream& print(std::ostream& out) const;
	};
	
	class Variable : public Expression{
		public:
			idexpr_t variable;
			idexpr_t group;
			Variable(idexpr_t k,idexpr_t v,idexpr_t g);
			std::ostream& print(std::ostream& out) const;
	};
	
	class LogicalExpression : public Expression{
		public:
			Arguments args;
			LogicalExpression(idexpr_t k,idtype_t t,Arguments &a);
			virtual Expression* substitute(idexpr_t o,idexpr_t n);
			std::ostream& print(std::ostream& out) const;
	};
	
	class Atom : public LogicalExpression{
		public:
			Atom(idexpr_t k,Arguments &a);
			bool isModeledBy(World* world);
			void apply(World* world,Atoms &addList,Atoms &removeList);
			void applyPositive(Atoms &addList,Atoms &removeList);
			Expression* substitute(idexpr_t o,idexpr_t n);
	};
	
	class And : public LogicalExpression{
		public:
			And(idexpr_t k,Arguments &a);
			bool isModeledBy(World* world);
			void apply(World* world,Atoms &addList,Atoms &removeList);
			void applyPositive(Atoms &addList,Atoms &removeList);
	};
	
	class Or : public LogicalExpression{
		public:
			Or(idexpr_t k,Arguments &a);
			bool isModeledBy(World* world);
			// Can't be applied, should throw error
	};
	
	class Not : public LogicalExpression{
		public:
			Not(idexpr_t k,Arguments &a);
			bool isModeledBy(World* world);
			void apply(World* world,Atoms &addList,Atoms &removeList);
			void applyPositive(Atoms &addList,Atoms &removeList);
	};
	
	class Equals : public LogicalExpression{
		public:
			Equals(idexpr_t k,Arguments &a);
			bool isModeledBy(World* world);
			// Can't be applied, should throw error
	};
	
	class Imply : public LogicalExpression{
		public:
			Imply(idexpr_t k,Arguments &a);
			bool isModeledBy(World* world);
			// Can't be applied, should throw error (an applied Imply is a When)
	};
	
	class When : public LogicalExpression{
		public:
			When(idexpr_t k,Arguments &a);
			// Can't be modeled, should throw error (a modeled When is an Imply)
			void apply(World* world,Atoms &addList,Atoms &removeList);
			void applyPositive(Atoms &addList,Atoms &removeList);
	};
	
	class Exists : public LogicalExpression{
		public:
			Exists(idexpr_t k,Arguments &a);
			bool isModeledBy(World* world);
			// Can't be applied, should throw error
	};
	
	class Forall : public LogicalExpression{
		public:
			Forall(idexpr_t k,Arguments &a);
			bool isModeledBy(World* world);
			void apply(World* world,Atoms &addList,Atoms &removeList);
			void applyPositive(Atoms &addList,Atoms &removeList);
	};
	
};

#endif
