
from pprint import pprint as pp
import itertools
import math

def sizes(set_count:int, item_count:int):
    nums = list(range(1,item_count+1))
    input = ''.join([str(x) for x in nums])
    combos = itertools.combinations_with_replacement(input, set_count)
    seen = set()
    def hash_x(x):
        _h = [0] * item_count
        for i in x:
            #print(x, i, _h)
            _h[i-1] += 1
        return tuple(_h)
        
    rval = []
    for x in combos:
        nums = [int(item) for item in x]
        if sum(nums) == item_count:
            _h = hash_x(nums)
            if _h not in seen:
                rval.append(nums)
                seen.add(_h)
    return rval

        
print(sizes(3, 6))  


def process(sz_left, avail, taken, so_far, results):
    #print(f"Enter path {path} taken {taken} avail {avail}")
    for i in itertools.combinations(avail, sz_left[0]):
        #print(f"Add entry {path}")
        tak = set(taken)
        tak.update(i)
        rem = avail - tak
        if len(sz_left) > 1:
            process(sz_left[1:], rem, tak, so_far+(i,), results)
        else:
            entry = tuple(so_far) + (i,)
            results.append(entry)

def possibilites(set_count: int, input:list):
    item_count = len(input)
    set_sizes = sizes(set_count, item_count)
    rval = []
    for sz_grp in set_sizes[:3]:
        taken = set()
        avail = set(input)
        #print(avail, taken)
        process(sz_grp, avail, taken, tuple(), rval)

    return rval
        

def prune(poss):
    deduped = []
    seen = set()
    for _x in poss:
        _y = tuple(sorted(_x))
        #print(_y)
        if _y in seen:
            continue
        seen.add(_y)
        deduped.append(_y)
    return deduped

def median(int_list:list):
    _l = len(int_list)
    idx = int(_l / 2)
    if _l % 2 == 0:
        return (int_list[idx] + int_list[idx-1]) / 2
    return int_list[math.floor(_l/2)]

# p = possibilites(3, [1,2,3,4,5,6])
# p = prune(p)
# pp(p)

def find_max_qual(channels: int, inputs: list):
    p = possibilites(channels, inputs)
    p = prune(p)
    #print(p)
    p = [tuple([tuple(sorted([z for z in x])) for x in c]) for c in p]
    score = {}
    _max = -1
    best = None
    for combo in p:
        this_score = sum([median(_x) for _x in combo])
        if this_score > _max:
            _max = this_score
            best = combo
        score[combo] = this_score
    return score, best

def a():
    tst = [1,2,3,4,5,6]
    s, b = find_max_qual(3, tst)
    pp(s)    

def test(sample):
    for k in range(1,len(sample)+1):
        score, best = find_max_qual(k, sample)
        print(f"K={k} Answer {best} Qual: {score[best]}")
    
from azq1 import test as test2

def run(test_list):
    print(f"Exhaustive search: ")
    test(test_list)
    print(f"Heuristic Approach: ")
    test2(test_list)
    print("\n\n")

for _s in [
    [1,2,3,4,5,6],
    [1,2,30,4,54,60, 87, 154],
    [1,2,3,400, 500, 6000],
    [1,2,3,51, 400, 5000, 100000],
    [1, 80, 300, 4000, 5000],
    [1, 80, 300, 4000, 5000, 200000],
]:
    run(_s)

