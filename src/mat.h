#pragma once
#include "vec2.h"
#include "vector_arithmetic.h"
#include <stdexcept>
#include <type_traits>

template<typename C = double>
class mat {
public:
	mat(int ht = 2, int wt = 2) : values(ht, a_vector<C>(wt, 0)), w{ wt }, h{ ht } {
		if (w == h) {
			for (int y = 0; y < h; y++) {
				values[y][y] = 1;
			}
		}
	}
	template<typename... T, int wt>
	mat(T(&&... vals)[wt]) : w{ wt } {
		get_values(vals...);
		h = values.size();
	}

	template<typename T>
	mat(const a_vector<T>& vals) : values(vals), w{ (int)values[0].size() }, h{ (int)values.size() } {}

	template<typename T, typename = std::enable_if_t<std::is_integral_v<T> || std::is_floating_point_v<T>>>
	mat(const a_vector<T>& vals) : values({{ vals }}), w{ (int)values[0].size() }, h{ (int)values.size() } {}

	template<typename... T>
	mat(T... mtlst) : w{ sizeof...(mtlst) }, h{ 1 } {
		values.push_back(a_vector<C>{(C)mtlst...});
	}

	mat(const vec2& v) : values{ {v.x}, {v.y }, {1.0} }, w{ 1 }, h{ 3 } {}
	
	template<typename T>
	mat(const mat<T>& m) : values{ static_cast<a_vector<a_vector<C>>>(m.values) }, w{ m.w }, h{ m.h } {}
	
	template<typename T>
	mat(mat<T>&& m): values{ static_cast<a_vector<a_vector<C>>>(std::move(m.values)) }, w{ m.w }, h{ m.h } {
		m.w = m.h = 0;
	}

	void resize(int newh, int neww) {
		w = neww;
		h = newh;
		for (auto& row : values) {
			row.resize(w);
		}
		values.resize(h, a_vector<C>(w, 0));
	}

	void transpose() {
		a_vector<a_vector<C>> original(values);
		resize(w, h);
		for (int y = 0; y < w; y++) {
			for (int x = 0; x < h; x++) {
				values[x][y] = original[y][x];
			}
		}
	}

	template<typename T>
	void augment(const mat<T>& m) {
		for (int y = 0; y < m.h; y++) {
			values[y].insert(values[y].end(), m.values[y].begin(), m.values[y].end());
		}
		w += m.w;
	}

	int width() const {
		return w;
	}
	int height() const {
		return h;
	}

	auto det()->decltype(C{} + double{}) const {
		return (h == w) ? get_det(*this) : NAN;
	}

	auto begin() {
		return values.begin();
	}

	auto end() {
		return values.end();
	}

	template<typename T>
	auto operator*(const mat<T>& m)->mat<decltype(C{} * T{})> const {
		return mat<decltype(C{} * T{})>(*this) *= m;
	}

	auto operator*(const vec2& v)->mat<decltype(C{} * double{})> const {
		return mat<decltype(C{} * double{})>(*this) *= v;
	}

	template<typename T>
	auto operator*(T t)->mat<decltype(C{} * T{})> const {
		return mat<decltype(C{} * T{})>(*this) *= t;
	}

	template<typename T>
	mat<C>& operator*=(const mat<T>& m) {
		mat<decltype(C{} + T{})> tmp(h, m.w);
		a_vector<decltype(C{} + T{}) > mcol1(m.h);
		for (int y = 0; y < h; y++) {
			for (int mx1 = 0; mx1 < m.w; mx1++) {
				for (int cy = 0; cy < m.h; cy++) {
					mcol1[cy] = m.values[cy][mx1];
				}
				tmp[y][mx1] = dot(values[y], mcol1);
			}
		}
		return *this = std::move(tmp);
	}

	mat<C>& operator*=(const vec2& v) {
		mat<double> vmat = v;
		*this *= vmat;
	}

	template<typename T>
	mat<C>& operator*=(T t) {
		for (auto& row : values) {
			row *= t;
		}
		return *this;
	}

	template<typename T>
	auto operator/(T t)->mat<decltype(C{} / T{})> {
		return mat<decltype(C{} / T{})>(*this) /= t;
	}

	template<typename T>
	mat<C>& operator/=(T t) {
		for (auto& row : values) {
			row /= t;
		}
		return *this;
	}

	template<typename T>
	auto operator+(const mat<T>& m)->mat<decltype(C{} + T{}) > const {
		return mat<decltype(C{} + T{})>(*this) += m;
	}

	template<typename T>
	mat<C>& operator+=(const mat<T>& m) {
		values += m.values;
		return *this;
	}

	template<typename T>
	auto operator-(const mat<T>& m)->mat<decltype(C{} - T{})> const {
		return mat<decltype(C{} - T{})>(*this) -= m;
	}

	template<typename T>
	mat<C>& operator-=(const mat<T>& m) {
		values -= m.values;
		return *this;
	}

	a_vector<C>& operator[](int y) {
		return values[y];
	}
	const a_vector<C>& operator[](int y) const {
		return values[y];
	}
	bool operator==(const mat& m) const {
		return values == m.values;
	}
	bool operator!=(const mat& m) const {
		return values != m.values;
	}
	mat& operator=(const vec2& v) {
		mat vmat{ v };
		*this = vmat;
		return *this;
	}

	template<typename T>
	mat& operator=(const mat<T>& m) {
		values = static_cast<a_vector<a_vector<C>>>(m.values);
		w = m.w;
		h = m.h;
		return *this;
	}
	template<typename T>
	mat& operator=(mat<T>&& m) {
		values = static_cast<a_vector<a_vector<C>>>(std::move(m.values));
		w = m.w;
		h = m.h;
		m.w = m.h = 0;
		return *this;
	}
private:
	a_vector<a_vector<C>> values;

	int h;
	int w;

	template<typename T>
	void get_values(T last) {
		values.push_back(a_vector<C>(last, last + w));
	}

	template<typename T, typename... Ts>
	void get_values(T head, Ts... tail) {
		values.push_back(a_vector<C>(head, head + w));
		get_values(tail...);
	}

	//DO NOT USE... EVER!
	/*static double get_det_slow(const mat& m) {
		if (m.h == 2) return m.values[0][0] * m.values[1][1] - m.values[0][1] * m.values[1][0];
		if (m.h == 1) return m.values[0][0];
		int ID = 0;
		double total = 0;
		for (int x = 0; x < m.values[0].size(); x++) {
			mat sub(m.h - 1, m.w - 1);
			int subx_i = 0;
			for (int suby = 1; suby < m.h; suby++) {
				for (int subx = 0; subx < m.w; subx++) {
					if (subx != x) {
						sub.values[suby - 1][subx_i] = m.values[suby][subx];
						subx_i++;
					}
				}
				subx_i = 0;
			}
			double sub_det = get_det_slow(sub) * m.values[0][x];
			if (ID % 2 == 0) total += sub_det;
			else total -= sub_det;
			ID++;
		}
		return total;
	}*/

	template<typename T>
	static auto get_det(mat<decltype(T{} + double{})> m) -> decltype(C{} + double{}) {
		for (int x = 0; x < m.w; x++) {
			for (int y = x + 1; y < m.h; y++) {
				if (!m.values[x][x]) {
					std::swap(m.values[x], m.values[y]);
					m.values[y] *= -1;
				}
				if (!m.values[y][x]) continue;
				m.values[y] += (-1 * (m.values[y][x] / m.values[x][x])) * m.values[x];
			}
		}
		decltype(T{} + double{}) det = 1;
		for (int y = 0; y < m.h; y++) {
			det *= m.values[y][y];
		}
		return det;
	}

	template<typename T>
	friend auto transpose(const mat<T>& m);

	template<typename T, typename C>
	friend auto augment(const mat<T>& m1, const mat<C>& m2)->mat<decltype(T{} + C{})>;

	template<typename T>
	friend auto inverse(const mat<T>& m)->mat<decltype(T{} + double{})>;

	template<typename T>
	friend class mat;
};

template<typename T>
auto transpose(const mat<T>& m) {
	mat<T> tmp{ m };
	tmp.resize(m.w, m.h);
	for (int y = 0; y < tmp.w; y++) {
		for (int x = 0; x < tmp.h; x++) {
			tmp.values[x][y] = m.values[y][x];
		}
	}
	return tmp;
}

template<typename T, typename C>
auto augment(const mat<T>& m1, const mat<C>& m2)->mat<decltype(T{} + C{})> {
	mat<decltype(T{} + C{})> result = m1;
	for (int y = 0; y < m1.height(); y++) {
		result.values[y].insert(result.values[y].end(), m2.values[y].begin(), m2.values[y].end());
	}
	result.h = m1.h;
	result.w = m1.w + m2.w;
	return result;
}

template<typename C>
inline vec2::vec2(const mat<C>& m){
	x = m[0][0];
	y = m[1][0];
}

template<typename C>
inline auto vec2::operator*(const mat<C> &m) const {
	return vec2(*this) *= m;
}

template<typename C>
inline auto& vec2::operator*=(const mat<C>& m) {
	mat<C> vmat{ *this };
	vmat *= m;
	return *this = vmat;
}

mat<double> translate(double x, double y);
mat<double> scale(double x_scale, double y_scale);
mat<double> rotate(double radians);
mat<double> shear(double x_shear, double y_shear);
mat<double> reflect(const vec2& l);