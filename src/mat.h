#pragma once
#include <vector>

template<int rows, int cols = rows>
class mat {
public:
	mat() : values( rows, std::vector<double>(cols) ) {
		if (rows == cols) {
			int y = 0;
			for (int x = 0; x < rows; x++) {
				values[x][y] = 1;
				y++;
			}
		}
	}
	mat(int val) : values( rows, std::vector<double>(cols, val) ) {}
	std::vector<double> operator[](int x) {
		return values[x];
	}
private:
	std::vector<std::vector<double>> values;
};