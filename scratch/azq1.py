import math

def median(int_list:list):
    _l = len(int_list)
    idx = int(_l / 2)
    if _l % 2 == 0:
        return (int_list[idx] + int_list[idx-1]) / 2
    return int_list[math.floor(_l/2)]


# print(median([1,2,3,4]))
# print(median([1,2,3,4,5]))

def find_max_qual(channels: int, inputs: list):
    _s = list(reversed(sorted(inputs)))

    remain = channels 
    count = len(_s)
    chosen = []
    while remain > 1:
        score = {}
        for _i in range(1, (count - len(chosen)) - remain):
            #print(f"Chosen: {chosen} remain: {remain} i is {_i} list is {_s}")
            candidate = _s[:_i]
            left = _s[_i:]
            sL = median(candidate)
            sR = median(left)
            score[_i] = sL + sR
            # print(f"{score}")
            if _i-1 in score and score[_i-1] > score[_i]:
                #print(f"Adding last: {_s[:_i-1]} leftover: {_s[_i-1:]}")
                chosen.append(_s[:_i-1])
                _s = _s[_i-1:]
                
                break
        if len(score) <= 1:
            #print(f"Couldn't do better than first for {_s}")
            chosen.append(_s[:1])
            _s = _s[1:]
        remain -= 1        
    
    #print(f"Adding remainder: {_s}")
    chosen.append(_s)
    return chosen

def a():
    _s = [1,2,3,4,5,6]
    x = find_max_qual(2, _s)
    print(x)

#exit(0)
def test(sample):
    for k in range(1, len(sample)+1):
        ans = find_max_qual(k, sample)
        print(f"K={k} Answer {ans} Qual: {sum(median(_x) for _x in ans)}")


if __name__ == '__main__':
    test([1,2,3,4,5,6])