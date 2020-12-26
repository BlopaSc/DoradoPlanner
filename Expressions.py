# -*- coding: utf-8 -*-
"""
@author: Blopa
"""

from enum import Enum,auto

str2Key = {'': 0}
key2Str = {0: ''}

class World(object):
	def __init__(self, atoms,sets,actions):
		self.actions = actions
		self.atoms = atoms
		self.sets = sets
		self.id = hash(tuple(atoms))
	def __repr__(self):
		return (
			'Sets:\n\t' + 
			',\n\t'.join((key2Str[key]+': ['+', '.join(key2Str[w] for w in self.sets[key]) +']') for key in self.sets) +
			'\nAtoms:\n\t' +
			',\n\t'.join(str(a) for a in self.atoms)
		)
	def __eq__(self,other):
		return self.id == other.id
	def __hash__(self):
		return self.id

class NonDeterministicActionError(Exception):
    pass

class LogicalTypes(Enum):
	LOGEXP = auto()
	ATOM = auto()
	AND = auto()
	OR = auto()
	NOT = auto()
	EQUALS = auto()
	IMPLY = auto()
	WHEN = auto()
	EXISTS = auto()
	FORALL = auto()

class Constant(object):
	def __init__(self,key):
		self.key = key
	def __repr__(self):
		return key2Str[self.key]
	def __eq__(self,other):
		return self.key == other
	def __hash__(self):
		return self.key
	def substitute(self, variable, value):
		return self if variable!=self.key else Constant(value)

class Variable(Constant):
	def __init__(self,key,typ):
		self.key,self.type = key,typ
	def __repr__(self):
		return key2Str[self.key] + (' - '+key2Str[self.type] if self.type else '')

class LogicalFormula(object):
	def __init__(self,args,typ=LogicalTypes.LOGEXP):
		self.type,self.args,self.id = typ,args,hash((typ,*args))
	def __repr__(self):
		return "("+" ".join([self.type.name] + list(str(a) for a in self.args))+")"
	def __eq__(self,other):
		return self.id == other
	def __hash__(self):
		return self.id
	def isModeledBy(self,world):
		return False
	def substitute(self, variable, value):
		return self.__class__(tuple(map(lambda arg: arg.substitute(variable,value), self.args)))
	def apply(self, world, addList, removeList):
		return
	
class Atom(LogicalFormula):
	def __init__(self,args):
		super().__init__(args,LogicalTypes.ATOM)
	def __repr__(self):
		return "("+" ".join(str(a) for a in self.args)+")"
	def isModeledBy(self,world):
		return self in world.atoms
	def substitute(self, variable, value):
		return Atom(tuple(map(lambda arg: value if arg==variable else arg, self.args)))
	def apply(self,world,addList,removeList):
		addList.add(self)

class And(LogicalFormula):
	def __init__(self,args):
		super().__init__(args,LogicalTypes.AND)
	def isModeledBy(self,world):
		return all(map(lambda arg: arg.isModeledBy(world), self.args))
	def apply(self,world,addList,removeList):
		for arg in self.args:
			arg.apply(world,addList,removeList)

class Or(LogicalFormula):
	def __init__(self,args):
		super().__init__(args,LogicalTypes.OR)
	def isModeledBy(self,world):
		return any(map(lambda arg: arg.isModeledBy(world), self.args))
	def apply(self,world,addList,removeList):
		raise NonDeterministicActionError

class Not(LogicalFormula):
	def __init__(self,args):
		super().__init__(args,LogicalTypes.NOT)
	def isModeledBy(self,world):
		return not self.args[0].isModeledBy(world)
	def apply(self,world,addList,removeList):
		self.args[0].apply(world,removeList,addList)
		
class Equals(LogicalFormula):
	def __init__(self,args):
		super().__init__(args,LogicalTypes.EQUALS)
	def isModeledBy(self,world):
		return self.args[0] == self.args[1]
	# CAN'T BE APPLIED
	
class Imply(LogicalFormula):
	def __init__(self,args):
		super().__init__(args,LogicalTypes.IMPLY)
	def isModeledBy(self,world):
		return (not self.args[0].isModeledBy(world)) or self.args[1].isModeledBy(world)
	# CAN'T BE APPLIED (for apply use WHEN)
	
class When(LogicalFormula):
	def __init__(self,args):
		super().__init__(args,LogicalTypes.WHEN)
	def apply(self,world,addList,removeList):
		if self.args[0].isModeledBy(world):
			self.args[1].apply(world,addList,removeList)
	# CAN'T BE MODELED (for model use IMPLY)
	
class Exists(LogicalFormula):
	def __init__(self,args):
		super().__init__(args,LogicalTypes.EXISTS)
	def isModeledBy(self, world):
		return any(
			self.args[1].substitute(self.args[0].key, value).isModeledBy(world) for value in world.sets[self.args[0].type]
		)
	def apply(self,world,add_list,remove_list):
		raise NonDeterministicActionError
		
class Forall(LogicalFormula):
	def __init__(self,args):
		super().__init__(args,LogicalTypes.FORALL)
	def isModeledBy(self, world):
		return all(
			self.args[1].substitute(self.args[0].key, value).isModeledBy(world) for value in world.sets[self.args[0].type]
		)
	def apply(self,world,addList,removeList):
		for value in world.sets[self.args[0].type]:
			self.args[1].substitute(self.args[0].key, value).apply(world,addList,removeList)
	
def get_text_key(text):
	if text in str2Key:
		return str2Key[text]
	else:
		l = len(key2Str)
		str2Key[text] = l
		key2Str[l] = text
		return l
	
def make_expression(expression):
	keywords = {
		'and':And,
		'or':Or,
		'not':Not,
		'=':Equals,
		'imply':Imply,
		'when':When,
		'exists':Exists,
		'forall':Forall
	}
	if expression[0] in keywords:
		if expression[0] in ['forall','exists']:
			# First parameter is a variable always
			return keywords[expression[0]]((make_expression(expression[1]).args[0],) + tuple(map(make_expression,expression[2:])))
		else:
			return keywords[expression[0]](tuple(map(make_expression,expression[1:])))
	else:
		args = []
		i = 0
		read = 1
		while i < len(expression):
			if expression[i][0] == '?':
				typ = ''
				if i+1 < len(expression) and expression[i+1]=='-':
					if i+2 < len(expression):
						typ = expression[i+2]
						read = 3
					else:
						read = 2
				else:
					read = 1
				args.append(Variable(get_text_key(expression[i]), get_text_key(typ)))
			else:
				args.append(Constant(get_text_key(expression[i])))
			i += read
		atom = Atom(tuple(args))
		return atom

def make_world(atoms,sets,actions=[]):
	world = World(set(map(make_expression,atoms)),{get_text_key(key): [get_text_key(item) for item in sets[key]] for key in sets},actions)
	return world

def models(world, condition):
	return condition.isModeledBy(world)

def substitute(expression, variable, value):
	return expression.substitute(get_text_key(variable),get_text_key(value))

def apply(world, effect):
	add_list,delete_list = set(),set()
	effect.apply(world,add_list,delete_list)
	nWorld = World((world.atoms-delete_list)|add_list, world.sets, world.actions)
	return nWorld

if __name__ == "__main__":
	x = make_expression(("at","x","y"))
	y = make_expression(("boarded","?person","-","passenger","?floor","plane1"))

	print(x)
	print(y)
	
	exp = make_expression(("or", ("on", "a", "b"), ("on", "a", "d")))
	world = make_world([("on", "a", "b"), ("on", "b", "c"), ("on", "c", "d")], {})
	print("Should be True: ", end="")
	print(models(world, exp))
	change = make_expression(["and", ("not", ("on", "a", "b")), ("on", "a", "c")])
	print("Should be False: ", end="")
	print(models(apply(world, change), exp))
	