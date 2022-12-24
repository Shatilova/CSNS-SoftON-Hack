// reversible shuffle algorithm
//
// original idea by https://stackoverflow.com/a/3541940/15301691

#pragma once

int* GetShuffleExchange(int size, int key) {
	int* exchanges = new int[size - 1];
	for (int i = size - 1; i > 0; i--) {
		int n = key % (i + 1);
		exchanges[size - i - 1] = n;
	}

	return exchanges;
}

template<class T>
T* ArrayShuffle(T * arr, int size, int key) {
	T* res = arr;

	int* exchanges = GetShuffleExchange(size, key);
	for (int i = size - 1; i > 0; i--) {
		int n = exchanges[size - i - 1];
		T tmp = res[i];
		res[i] = res[n];
		res[n] = tmp;
	}

	return res;
}

template<class T>
T* ArratDeshuffle(T * arr, int size, int key) {
	T* res = arr;

	int* exchanges = GetShuffleExchange(size, key);
	for (int i = 1; i < size; i++) {
		int n = exchanges[size - i - 1];
		T tmp = res[i];
		res[i] = res[n];
		res[n] = tmp;
	}

	return res;
}