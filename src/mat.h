#pragma once
#include "vec2.h"
#include <vector>
#include <stdexcept>
#include <initializer_list>

template <class T> inline T dot(std::vector<T> v1, std::vector<T> v2) {
	static_assert(v1.size() == v2.size(), "vec dimensions must be equal");
	T result = 0;
	for (int i = 0; i < v1.size(); i++) {
		result += v1[i] * v2[i];
	}
	return result;
}

template<int h, int w = h>
class mat {
public:
	const int width = w;
	const int height = h;
	mat() : values(h, std::vector<double>(w)) {
		if (w == h) {
			int x = 0;
			for (int y = 0; y < h; y++) {
				values[y][x] = 1;
				x++;
			}
		}
	}
	mat(std::initializer_list<std::vector<double>> matvals){
		values = matvals;
	}
	mat(int val) : values(h, std::vector<double>(w, val)) {}
	mat(const mat& m) {
		values = m.values;
	}
	mat(mat&& m) {
		values = std::move(m.values);
	}
	template<int mw, int mh = mw>
	auto operator*(const mat<mw, mh>& m) -> mat<w, mh> {
		if (w != mh) throw std::logic_error{ "Matrices cannot be multiplied" };
		mat<w, mh> temp;
		std::vector<double> mcol(mh);
		for (int y = 0; y < mw; y++) {
			for (int my = 0; my < mh; my++) {
				mcol[my] = m.values[my];
			}
			for (int mx = 0; mx < mw; mx++) {
				temp.values[y % h][mx] = dot(values[y][mx], mcol);
			}
		}
		return temp;
	}
	mat operator+(const mat& m) {
		mat temp;
		for (int y = 0; y < h; y++) {
			for (int x = 0; x < w; x++) {
				temp.values[y][x] = values[y][x] + m.values[y][x];
			}
		}
		return temp;
	}
	std::vector<double>& operator[](int y) {
		return values[y];
	}
	mat& operator=(const mat& m) {
		values = m.values;
		return *this;
	}
	mat& operator=(mat&& m) {
		values = std::move(m.values);
	}
private:
	std::vector<std::vector<double>> values;
};