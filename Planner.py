# -*- coding: utf-8 -*-
"""
@author: Blopa
"""

import itertools as it
import functools
import time
import Expressions
import Pathfinding
import PDDL

def addlist_heuristic(n, edge, target):
	return len(target - (target&edge[0].atoms))

def plan(domain, problem, use_heuristic=False,verbose=False):
	goal_expression = Expressions.make_expression(problem['goal'])
	grounded_actions = {}
	for action in domain['actions']:
		for combination in it.product(*(problem['sets'][domain['actions'][action]['parameters'][param]] for param in domain['actions'][action]['parameters'])):
			name = action+'('
			precond = Expressions.make_expression(domain['actions'][action]['precondition'])
			effect = Expressions.make_expression(domain['actions'][action]['effect'])
			for p,param in enumerate(domain['actions'][action]['parameters']):
				name += (',' if p>0 else '')+str(combination[p])
				precond = Expressions.substitute(precond, param, combination[p])
				effect = Expressions.substitute(effect, param, combination[p])
			name += ')'
			grounded_actions[name] = [precond,effect] 
	world = Expressions.make_world(problem['atoms'],problem['sets'],grounded_actions)
	heuristic = Pathfinding.default_heuristic
	if use_heuristic:
		add_list, neg_list,_ = Expressions.simplify(goal_expression,world)
		heuristic = functools.partial(addlist_heuristic,target=add_list)
		
	def goal_function(state):
		return Expressions.models(state,goal_expression)
	
	res = Pathfinding.astar(world, heuristic, goal_function)
	
	if verbose:
		Pathfinding.print_path(res)
	return res
	
if __name__ == "__main__":
	domain = PDDL.parse_domain('classical-domains/classical/elevators-00-adl/domain.pddl')
	problem = PDDL.parse_problem('classical-domains/classical/elevators-00-adl/s7-0.pddl')
	print("No heuristic:")
	time_start = time.time()
	x = plan(domain,problem,use_heuristic=False,verbose=True)
	print("Time taken:",time.time()-time_start)
	print("Heuristic1:")
	time_start = time.time()
	x = plan(domain,problem,use_heuristic=True,verbose=True)
	print("Time taken:",time.time()-time_start)