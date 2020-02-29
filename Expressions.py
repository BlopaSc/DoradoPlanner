# -*- coding: utf-8 -*-
"""
@author: Blopa
"""

import functools

class World(object):
	words = dict()
	iwords = dict()
	def __init__(self, atoms,sets,actions):
		self.actions = actions
		self.atoms = atoms
		self.sets = sets
		self.id = hash(tuple(atoms))
	def __repr__(self):
		return (
			'Sets:\n\t' + 
			',\n\t'.join((World.iwords[key]+': ['+', '.join(World.iwords[w] for w in self.sets[key]) +']') for key in self.sets) +
			'\nAtoms:\n\t' +
			',\n\t'.join(str(atom) for atom in self.atoms)
		)
	def __eq__(self,other):
		return self.id == other.id
	def __hash__(self):
		return self.id
	# Get neighbors returns a list of 3-tuples: (world/node, cost, action/name)
	def get_neighbors(self):
		neighbors = []
		for action in self.actions:
			precond,effect = self.actions[action]
			if models(self,precond):
				neighbors.append((apply(self,effect),1,action))
		return neighbors
		
	@staticmethod
	def identify(word):
		if word not in World.words.keys():
			n = len(World.words)
			World.words[word] = n
			World.iwords[n] = word
		return World.words[word]

### ERRORS
class NonDeterministicActionError(Exception):
    pass

class LogicalFormula(object):
	def __init__(self,args):
		self.name,self.args = '',args
	def __repr__(self):
		return '('+self.name+' '+' '.join((World.iwords[arg] if (type(arg) is int) else str(arg)) for arg in self.args)+')'
	def __eq__(self,other):
		return hash(self)==hash(other)
	def __hash__(self):
		return hash((self.name,*self.args))
	def isModeledBy(self, world):
		return False
	def substitute(self,variable,value):
		return self
	def apply(self,world,add_list,remove_list):
		return
	
class Atom(LogicalFormula):
	def __init__(self,name,args):
		self.name,self.args = name,args
		self.id = hash((name,*args))
	def __repr__(self):
		return '('+World.iwords[self.name]+' '+' '.join((World.iwords[arg] if (type(arg) is int) else str(arg)) for arg in self.args)+')'
	def get_id(self):
		return (self.name,*self.args)
	def isModeledBy(self, world):
		return self in world.atoms
	def substitute(self,variable,value):
		return Atom(self.name,tuple(map(lambda arg: value if arg==variable else arg, self.args)))
	def apply(self,world,add_list,remove_list):
		add_list.add(self)
	
# "and" has arbitrarily many parameters
class And(LogicalFormula):
	def __init__(self,args):
		self.name,self.args = 'and',args
	def isModeledBy(self, world):
		return all(map(lambda arg: arg.isModeledBy(world), self.args))
	def substitute(self,variable,value):
		return And(tuple(map(lambda arg: arg.substitute(variable,value), self.args)))
	def apply(self,world,add_list,remove_list):
		for arg in self.args:
			arg.apply(world,add_list,remove_list)
	
# "or" has arbitrarily many parameters
class Or(LogicalFormula):
	def __init__(self,args):
		self.name,self.args = 'or',args
	def isModeledBy(self, world):
		return any(map(lambda arg: arg.isModeledBy(world), self.args))
	def substitute(self,variable,value):
		return Or(tuple(map(lambda arg: arg.substitute(variable,value), self.args)))
	def apply(self,world,add_list,remove_list):
		raise NonDeterministicActionError

# "not" has exactly one parameter 
class Not(LogicalFormula):
	def __init__(self,args):
		self.name,self.args = 'not',args
	def isModeledBy(self, world):
		return not self.args[0].isModeledBy(world)
	def substitute(self,variable,value):
		return Not((self.args[0].substitute(variable,value),))
	def apply(self,world,add_list,remove_list):
		self.args[0].apply(world,remove_list,add_list)

# "=" has exactly two parameters which are variables or constants, can't be applied
class Equals(LogicalFormula):
	def __init__(self,args):
		self.name,self.args = '=',args
	def isModeledBy(self,world):
		return self.args[0]==self.args[1]
	def substitute(self,variable,value):
		return Equals(tuple(map(lambda arg: value if arg==variable else arg, self.args)))

# "imply" has exactly two parameters, can't be applied
class Imply(LogicalFormula):
	def __init__(self,args):
		self.name,self.args = 'imply',args
	def isModeledBy(self, world):
		return (not self.args[0].isModeledBy(world)) or self.args[1].isModeledBy(world) 
	def substitute(self,variable,value):
		return Imply((self.args[0].substitute(variable,value),self.args[1].substitute(variable,value)))

# "when" has exactly two parameters and can't be modeled
class When(LogicalFormula):
	def __init__(self,args):
		self.name,self.args = 'when',args
	def substitute(self,variable,value):
		return When(tuple(map(lambda arg: arg.substitute(variable,value), self.args)))
	def apply(self,world,add_list,remove_list):
		if self.args[0].isModeledBy(world):
			self.args[1].apply(world,add_list,remove_list)

# "exists" has exactly two parameters, where the first one is a variable specification of ONE variable
class Exists(LogicalFormula):
	def __init__(self,args):
		self.name,self.args = 'exists',args
	def isModeledBy(self, world):
		return any(
			self.args[1].substitute(self.args[0].name, value).isModeledBy(world) for value in world.sets[self.args[0].args[1]]
		)
	def substitute(self,variable,value):
		return Exists(tuple(map(lambda arg: arg.substitute(variable,value), self.args)))
	def apply(self,world,add_list,remove_list):
		raise NonDeterministicActionError

# "forall" has exactly two parameters, where the first one is a variable specification of ONE variable, can't be applied
class Forall(LogicalFormula):
	def __init__(self,args):
		self.name,self.args = 'forall',args
	def isModeledBy(self, world):
		return all(
			self.args[1].substitute(self.args[0].name, value).isModeledBy(world) for value in world.sets[self.args[0].args[1]]
		)
	def substitute(self,variable,value):
		return Forall(tuple(map(lambda arg: arg.substitute(variable,value), self.args)))
	def apply(self,world,add_list,remove_list):
		for value in world.sets[self.args[0].args[1]]:
			self.args[1].substitute(self.args[0].name, value).apply(world,add_list,remove_list)
	
def make_expression(expression):
	keywords = {
		'and':And,
		'or':Or,
		'not':Not,
		'imply':Imply,
		'when':When,
		'exists':Exists,
		'forall':Forall
	}
	if expression[0]=='=':
		return Equals(tuple(map(World.identify,expression[1:])))
	elif expression[0] in keywords.keys():
		return keywords[expression[0]](tuple(map(make_expression,expression[1:])))
	else:
		return Atom(World.identify(expression[0]),tuple(map(World.identify,expression[1:])))

def make_world(atoms,sets,actions=[]):
	world = World(set(map(make_expression,atoms)),{World.identify(key): [World.identify(item) for item in sets[key]] for key in sets},actions)
#	added_list = functools.reduce(lambda a,b: a|b,[simplify(actions[act][1],world)[0] for act in actions]) | world.atoms
#	remove_list = []
#	for act in actions:
#		if len(simplify(actions[act][0],world)[0] - added_list)>0:
#			remove_list.append(act)
#	while remove_list:
#		del actions[remove_list.pop()]
	return world

def models(world, condition):
	return condition.isModeledBy(world)

def substitute(expression, variable, value):
	return expression.substitute(World.identify(variable),World.identify(value))

def apply(world, effect):
	add_list,delete_list = set(),set()
	effect.apply(world,add_list,delete_list)
	return World((world.atoms-delete_list)|add_list, world.sets, world.actions)

def simplify(expression,world):
	atoms,not_atoms = set(),set()
	valid = True
	typed = type(expression)
	if typed==Atom:
		atoms.add(expression)
	elif typed==And or typed==Or:
		for arg in expression.args:
			a,an,val = simplify(arg,world)
			if val:
				atoms |= a
				not_atoms |= an
	elif typed==Not:
		a,an,val = simplify(expression.args[0],world)
		if val:
			atoms |= an
			not_atoms |= a
	elif typed==Imply:
		a,an,val = simplify(expression.args[0],world)
		if val:
			atoms |= an
			not_atoms |= a
		a,an,val = simplify(expression.args[1],world)
		if val:
			atoms |= a
			not_atoms |= an
	elif typed==When:
		a,an,val = simplify(expression.args[1],world)
		if val:
			atoms |= a
			not_atoms |= an
	elif typed==Forall or typed==Exists:
		var = expression.args[0].name
		typed = expression.args[0].args[1]
		for obj in world.sets[typed]:
			a,an,val = simplify(expression.args[1].substitute(var,obj),world)
			if val:
				atoms |= a
				not_atoms |= an
	elif typed==Equals:
		if not expression.isModeledBy(world):
			valid = False
	return atoms,not_atoms,valid

if __name__ == "__main__":
	exp = make_expression(("or", ("on", "a", "b"), ("on", "a", "d")))
	world = make_world([("on", "a", "b"), ("on", "b", "c"), ("on", "c", "d")], {})
	print("Should be True: ", end="")
	print(models(world, exp))
	change = make_expression(["and", ("not", ("on", "a", "b")), ("on", "a", "c")])
	print("Should be False: ", end="")
	print(models(apply(world, change), exp))
