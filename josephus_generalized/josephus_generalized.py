from math import ceil


def josephus_n_array(mod, participants):
    if mod == 0:
        return
    
    if mod == 1:
        return 0
    
    array = [_ for _ in range(1, participants+1)]

    i = 0
    while i < len(array):
        if (i + 1) % mod != 0:
            array.append(array[i])
        i += 1
    
    return array[-1]


def josephus_n(mod, participants):
    if mod == 0:
        return
    
    if mod == 1:
        return 0

    D = 1

    while D <= (mod - 1)*participants:
        D = ceil( mod/(mod-1) * D )

    return mod*participants + 1 - D


if __name__ == "__main__":
    # for mod 3 and participants 10 answer == 4
    mod = 3
    participants = 10

    josephus = josephus_n(mod, participants)
    print(josephus)

    josephus = josephus_n_array(mod, participants)
    print(josephus)
