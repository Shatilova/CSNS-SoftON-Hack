import numpy

# original idea of reversible shuffle algorithm from
# https://stackoverflow.com/a/3541940/15301691

def get_shuffle_exchange(size, key):
    exchanges = numpy.zeros(size, dtype=int)
    i = size - 1
    while i > 0:
        n = key % (i + 1)
        exchanges[size - i - 1] = n

        i -= 1

    return exchanges

def shuffle_array(arr, size, key):
    res = arr

    exchanges = get_shuffle_exchange(size, key)

    i = size - 1
    while i > 0:
        n = exchanges[size - i - 1]

        tmp = res[i]
        res[i] = res[n]
        res[n] = tmp

        i -= 1

    return res