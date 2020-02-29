# -*- coding: utf-8 -*-
"""
@author: Blopa
"""

### ERROR
class PDDLSyntaxError(Exception):
	pass
class MissingRequirementError(Exception):
	pass
class UnclosedExpressionError(Exception):
	pass
class UnknownDomainError(Exception):
	pass
class UnknownTypeError(Exception):
	pass

def parseSExpression(s):
	tokens = s.replace('(',' ( ').replace(')',' ) ').replace(',',' ').replace('"',' ').replace('\n',' ').replace('\t','').split(' ')
	stack = []
	for token in tokens:
		if not token or token==' ':
			continue
		elif token==')':
			expr = []
			while stack and stack[-1]!='(':
				expr.insert(0,stack.pop())
			if stack:
				stack.pop()
				stack.append(expr)
			else:
				raise UnclosedExpressionError
		else:
			stack.append(token)
	return stack.pop()

support = [':adl',':strips',':typing',':disjunctive-preconditions',':equality',':existential-preconditions',':universal-preconditions',':quantified-preconditions',':conditional-effects']
domains = {}

def parse_typed_list(typed):
	objects = {}
	batch = []
	typing = False
	for obj in typed:
		if len(obj)==0:
			continue
		elif obj=='-':
			typing = True
		elif typing:
			typing = False
			while batch:
				objects[batch.pop()] = obj
		else:
			batch.append(obj)
	while batch:
		objects[batch.pop()] = ''
	return objects

def parse_domain(filename):
	with open(filename,'r') as file:
		try:
			pddl = ' '.join([line.replace('\n',' ') for line in file.readlines() if not ';;' in line])	
			domain_tree = parseSExpression(pddl)
			if domain_tree[0]!='define' or domain_tree[1][0]!='domain':
				raise PDDLSyntaxError
			domain_name = domain_tree[1][1]
			reqs = ''
			known_types = {'':''}
			known_constants = {}
			known_predicates = {}
			known_actions = {}
			for section in domain_tree:
				if section[0]==':requirements':
					reqs = section[1:]
					for req in reqs:
						if req not in support:
							raise MissingRequirementError
				elif section[0]==':types':
					known_types = parse_typed_list(section[1:])
					known_types[''] = ''
					types = list(known_types.keys())
					for subtype in types:
						if known_types[subtype] not in known_types:
							known_types[known_types[subtype]] = ''
				elif section[0]==':constants':
					known_constants = parse_typed_list(section[1:])
				elif section[0]==':predicates':
					for pred in section[1:]:
						known_predicates[pred[0]] = parse_typed_list(pred[1:])
				elif section[0]==':action':
					action_name = section[1]
					parameters = {}
					precondition = None
					effect = None
					for s in range(2,len(section)):
						if section[s]==':parameters':
							parameters = parse_typed_list(section[s+1])
						elif section[s]==':precondition':
							precondition = section[s+1]
						elif section[s]==':effect':
							effect = section[s+1]
					known_actions[action_name] = {
							'parameters': parameters,
							'precondition': precondition,
							'effect': effect
					}
			domains[domain_name] = {
					'name': domain_name,
					'requirements': reqs,
					'types': known_types,
					'constants': known_constants,
					'predicates': known_predicates,
					'actions': known_actions
			}
			return domains[domain_name]
		except:
			raise PDDLSyntaxError	
		
def parse_problem(filename):
	with open(filename,'r') as file:
		try:
			pddl = ' '.join([line.replace('\n',' ') for line in file.readlines() if not ';;' in line])	
			problem_tree = parseSExpression(pddl)
			if problem_tree[0]!='define' or problem_tree[1][0]!='problem':
				raise PDDLSyntaxError
			problem_name = problem_tree[1][1]
			domain_name = ''
			known_objects = []
			known_atoms = []
			known_goal = None
			for section in problem_tree:
				if section[0]==':domain':
					domain_name = section[1]
				elif section[0]==':objects':
					objects = []
					typing = False
					for obj in section[1:]:
						if len(obj)==0:
							continue
						elif obj=='-':
							typing = True
						elif typing:
							typing = False
							while objects:
								known_objects.append([objects.pop(),obj])
						else:
							objects.append(obj)
					while objects:
						known_objects.append([objects.pop(),''])
				elif section[0]==':init':
					for a in section[1:]:
						known_atoms.append(a)
				elif section[0]==':goal':
					known_goal = section[1]
			if domain_name not in domains.keys():
				raise UnknownDomainError
			domain = domains[domain_name]
			known_objects.extend([[key,domain['constants'][key]] for key in domain['constants']])
			known_sets = {key: [] for key in domain['types'].keys()}
			for obj in known_objects:
				if obj[1] not in known_sets.keys():
					raise UnknownTypeError
				known_sets[obj[1]].append(obj[0])
			for i in range(len(domain['types'])):
				for typ in domain['types']:
					for obj in known_sets[typ]:
						if obj not in known_sets[domain['types'][typ]]:
							known_sets[domain['types'][typ]].append(obj)
			problem = {
				'name': problem_name,
				'domain': domain,
				'atoms': known_atoms,
				'sets': known_sets,
				'goal': known_goal
			}
			return problem
		except:
			raise PDDLSyntaxError
			
			
if __name__ == "__main__":
	d = parse_domain('classical-domains/classical/miconic/domain.pddl')
	p = parse_problem('classical-domains/classical/miconic/s1-0.pddl')
	dx = parse_domain('classical-domains/classical/airport/p06-domain.pddl')
	px = parse_problem('classical-domains/classical/airport/p06-airport2-p2.pddl')
