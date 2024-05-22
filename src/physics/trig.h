#ifndef TRIG_H
#define TRIG_H

#include <numbers>

namespace math
{
	namespace detail
	{
		constexpr double factorial(unsigned int n)
		{
			double res = 1;
			for (unsigned int i = 2; i <= n; ++i)
				res *= i;
			return res;
		}
		constexpr double power(double base, double exp)
		{
			double res = 1;
			for (int i = 0; i < exp; ++i)
				res *= base;
			return res;
		}

		constexpr double normalize_angle(double x)
		{
			constexpr double two_pi = 2.0 * std::numbers::pi;
			while (x > two_pi)
				x -= two_pi;
			while (x < -two_pi)
				x += two_pi;
			return x;
		}

		constexpr double sin_taylor(double x, int N = 10)
		{
			x = normalize_angle(x);
			double result = 0;
			for (int n = 0; n < N; ++n)
				result += power(-1, n) * power(x, 2 * n + 1) / factorial(2 * n + 1);
			return result;
		}

		constexpr double cos_taylor(double x, int N = 10)
		{
			x = normalize_angle(x);
			double result = 0;
			for (int n = 0; n < N; ++n)
				result += power(-1, n) * power(x, 2 * n) / factorial(2 * n);
			return result;
		}
	}

	constexpr double cos(double x) { return detail::cos_taylor(x, 15); }
	constexpr double sin(double x) { return detail::sin_taylor(x, 15); }
}

#endif