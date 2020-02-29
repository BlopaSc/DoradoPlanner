# -*- coding: utf-8 -*-
"""
@author: Blopa
"""

import time

def default_heuristic(n, edge):
    return 0

def astar(start_state, heuristic, goal,timeout = None, expanded_limit=None):
	if timeout is None:
		timeout = 115516800
	if expanded_limit is None:
		expanded_limit = 1401946112
	visited_cnt = 1
	expanded_cnt = 0
	duplicated_cnt = 0
	improved_cnt = 0
	expanded = set()
	item = [start_state,0,0,[]]
	frontier = [item]
	frontier_dic = {start_state.id:item}
	minimum_found = 999999999
	start_time = time.time()
	while frontier and (time.time()-start_time)<timeout and expanded_cnt<expanded_limit:
		current_state, current_dist, h, path = frontier[0]
		if goal(current_state):
#			return path,current_dist,(visited_cnt,expanded_cnt,duplicated_cnt,improved_cnt)
			return (path,current_dist,visited_cnt,expanded_cnt)
		else:
			del frontier_dic[current_state.id]
			frontier = frontier[1:]
		neighbors = current_state.get_neighbors()
		expanded.add(current_state.id)
		expanded_cnt += 1
		for neighbor in neighbors:
			new_state = neighbor[0]
			nid = new_state.id
			if nid not in expanded:
				new_path = path + [neighbor[2]]
				h = heuristic(current_state,neighbor)
				cost = current_dist+neighbor[1]
				old_cost = 0
				item = frontier_dic.get(nid)
				if item is not None:
					old_cost = item[1] + item[2]
					duplicated_cnt += 1
					if old_cost > cost + h:
						item[0],item[1],item[2],item[3] = new_state,cost,h,new_path
						improved_cnt += 1
				else:
					item = [new_state,cost,h,new_path]
					frontier.append(item)
					frontier_dic[new_state.id] = item
					visited_cnt += 1
				if (cost + h)<minimum_found:
					minimum_found = cost + h
		if frontier and minimum_found <= (frontier[0][1]+frontier[0][2]):
			frontier.sort(key=lambda x: x[1] + x[2])
			minimum_found = 9999999999
	return (None,0,visited_cnt,expanded_cnt)

def print_path(result):
	if len(result)==3:	
		(path, cost, counters) = result
		print("Visited nodes: %i\nExpanded nodes: %i\nDuplicated/improved nodes: %i/%i"%counters)
	elif len(result)==4:
		(path, cost, vn, en) = result
		print("Visited nodes: %i\nExpanded nodes: %i"%(vn,en))
	if path:
		print("Path found with cost:", cost)
		for step in path:
			print(step)
			pass
	else:
		print("No path found")
	print("\n")
