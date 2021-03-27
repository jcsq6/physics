#pragma once
#include <vector>
#include <numeric>
#include <type_traits>

template<typename T>
class a_vector : public std::vector<T> {
public:
	using std::vector<T>::vector;

	template<typename C>
	operator a_vector<C>() const & {
		a_vector<C> tmp;
		tmp.reserve(this->size());
		for (const auto& t : *this) {
			tmp.push_back(static_cast<C>(t));
		}
		return tmp;
	}

	template<typename C>
	operator a_vector<C>() && {
		a_vector<C> tmp;
		tmp.reserve(this->size());
		for (const auto& t : *this) {
			tmp.push_back(std::move(static_cast<C>(t)));
		}
		this->clear();
		return tmp;
	}

	template<typename C>
	operator std::vector<C>() const {
		std::vector<C> tmp;
		tmp.reserve(this->size());
		for (const auto& t : *this) {
			tmp.push_back(static_cast<C>(t));
		}
		return tmp;
	}

	template<typename C>
	a_vector& operator-=(const a_vector<C>& v) {
		for (int i = 0; i < this->size(); i++) {
			this->at(i) -= v.at(i);
		}
		return *this;
	}
	template<typename C>
	a_vector& operator-=(const C& e) {
		for (int i = 0; i < this->size(); i++) {
			this->at(i) -= e;
		}
		return *this;
	}
	template<typename C>
	a_vector operator-(const a_vector<C>& v) const {
		return a_vector(*this) -= v;
	}
	template<typename C>
	a_vector operator-(const C& e) const {
		return a_vector(*this) -= e;
	}
	template<typename C>
	a_vector& operator+=(const a_vector<C>& v) {
		for (int i = 0; i < this->size(); i++) {
			this->at(i) += v.at(i);
		}
		return *this;
	}
	template<typename C>
	a_vector& operator+=(const C& e) {
		for (int i = 0; i < this->size(); i++) {
			this->at(i) += e;
		}
		return *this;
	}
	template<typename C>
	a_vector operator+(const a_vector<C>& v) const {
		return a_vector(*this) += v;
	}
	template<typename C>
	a_vector operator+(const C& e) const {
		return a_vector(*this) += e;
	}
	template<typename C>
	a_vector& operator*=(const a_vector<C>& v) {
		for (int i = 0; i < this->size(); i++) {
			this->at(i) *= v.at(i);
		}
		return *this;
	}
	template<typename C>
	a_vector& operator*=(const C& e) {
		for (int i = 0; i < this->size(); i++) {
			this->at(i) *= e;
		}
		return *this;
	}
	template<typename C>
	a_vector operator*(const a_vector<C>& v) const {
		return a_vector(*this) *= v;
	}
	template<typename C>
	a_vector operator*(const C& e) const {
		return a_vector(*this) *= e;
	}
	template<typename C>
	a_vector& operator/=(const a_vector<C>& v) {
		for (int i = 0; i < this->size(); i++) {
			this->at(i) /= v.at(i);
		}
		return *this;
	}
	template<typename C>
	a_vector& operator/=(const C& e) {
		for (int i = 0; i < this->size(); i++) {
			this->at(i) /= e;
		}
		return *this;
	}
	template<typename C>
	a_vector operator/(const a_vector<C>& v) const {
		return a_vector(*this) /= v;
	}
	template<typename C>
	a_vector operator/(const C& e) const {
		return a_vector(*this) /= e;
	}
};

template<typename C, typename T>
auto operator*(const C& e, a_vector<T> v) {
	return v *= e;
}

template <class T> inline T dot(const a_vector<T>& v1, const a_vector<T>& v2) {
	a_vector multiplied = v1 * v2;
	return std::accumulate(multiplied.begin(), multiplied.end(), 0.0);
}