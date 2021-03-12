#pragma once
#include "vec2.h"
#include <vector>
#include <stdexcept>
#include <initializer_list>
#include <type_traits>

template <class T> inline T dot(const std::vector<T> &v1, const std::vector<T> &v2) {
	if (v1.size() != v2.size()) throw std::logic_error{"vec dimensions must be equal" };
	T result = 0;
	for (int i = 0; i < v1.size(); i++) {
		result += v1[i] * v2[i];
	}
	return result;
}

namespace mat_helper {
	template<typename T, typename = void>
	struct row;

	template<int w1, int... ws>
	struct row<std::integer_sequence<int, w1, ws...>, std::enable_if_t<((w1 == ws) && ...)>> {
		static constexpr int w = w1;
		static constexpr int h = sizeof...(ws) + 1;
	};
	template<int... ws>
		inline constexpr int row_w = row<std::integer_sequence<int, ws...>>::w;

	template<int... ws>
	inline constexpr int row_h = row<std::integer_sequence<int, ws...>>::h;
};

template<int h, int w = h>
class mat {
public:
	mat() : values(h, std::vector<double>(w)) {
		if (w == h) {
			int x = 0;
			for (int y = 0; y < h; y++) {
				values[y][x] = 1;
				x++;
			}
		}
	}
	template<int... ws, typename = std::enable_if<h + 1 == mat_helper::row_h<w, ws...>>>
	mat(double (&&... head)[ws]){

	}
	mat(int val) : values(h, std::vector<double>(w, val)) {}
	mat(const mat& m) {
		values = m.values;
	}
	mat(mat&& m) {
		values = std::move(m.values);
	}
	int width() {
		return w;
	}
	int height() {
		return h;
	}
	template<int mh, int mw = mh>
	auto operator*(const mat<mh, mw>& m) -> mat<h, mw> const {
		if (w != mh) throw std::logic_error{ "Matrices cannot be multiplied" };
		mat<h, mw> temp;
		std::vector<double> mcol(mh);
		for (int y = 0; y < mw; y++) {
			for (int mx = 0; mx < mw; mx++) {
				for (int my = 0; my < mh; my++) {
					mcol[my] = m.values[my][mx];
				}
				temp.values[y % h][mx] = dot(values[y % h], mcol);
			}
		}
		return temp;
	}
	mat operator+(const mat& m) const {
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
		return *this;
	}
private:
	std::vector<std::vector<double>> values;

	template<int mw, int mh>
	friend class mat;
};

template<int... ws>
mat(double (&&... head)[ws]) -> mat<mat_helper::row_h<ws...>, mat_helper::row_w<ws...>>;