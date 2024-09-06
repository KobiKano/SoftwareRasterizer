#pragma once

#include "geom.hpp"
#include <cmath>
#include <vector>
#include <stdexcept>

class Quaternion
{
	struct Complex
	{
		enum c_type {
			REAL, I_TYPE, J_TYPE, K_TYPE
		};
		c_type t;
		float val;
		Complex() { t = I_TYPE; val = 0.f; }
		Complex(c_type t, float val) { this->t = t; this->val = val; }
		inline Complex operator *(const Complex &c) const
		{
			//use quaternion multiplication rules to determine type of result
			if (t == REAL && c.t == REAL)
			{
				return Complex(REAL, val * c.val);
			}
			if (t == c.t)
			{
				return Complex(REAL, -(val * c.val));
			}
			if (t == REAL)
			{
				return Complex(c.t, val * c.val);
			}
			if (c.t == REAL)
			{
				return Complex(t, val * c.val);
			}
			if (t == I_TYPE && c.t == J_TYPE)
			{
				return Complex(K_TYPE, val * c.val);
			}
			if (t == I_TYPE && c.t == K_TYPE)
			{
				return Complex(J_TYPE, -(val * c.val));
			}
			if (t == J_TYPE && c.t == I_TYPE)
			{
				return Complex(K_TYPE, -(val * c.val));
			}
			if (t == J_TYPE && c.t == K_TYPE)
			{
				return Complex(I_TYPE, val * c.val);
			}
			if (t == K_TYPE && c.t == I_TYPE)
			{
				return Complex(J_TYPE, val * c.val);
			}
			if (t == K_TYPE && c.t == J_TYPE)
			{
				return Complex(I_TYPE, -(val * c.val));
			}
			//this code should never be reached
			throw std::invalid_argument("Unexpected Complex Type encountered\n");
		}
	};

public:
	Quaternion(float angle, Vec3f &axis) 
	{ 
		real = Complex(Complex::REAL, cosf(angle / 2));
		i_v = Complex(Complex::I_TYPE, sinf(angle / 2) * axis.x);
		j_v = Complex(Complex::J_TYPE, sinf(angle / 2) * axis.y);
		k_v = Complex(Complex::K_TYPE, sinf(angle / 2) * axis.z);
		this->angle = angle;
	}
	Quaternion(Vec3f &vec)
	{
		real = Complex(Complex::REAL, 0.f);
		i_v = Complex(Complex::I_TYPE, vec.x);
		j_v = Complex(Complex::J_TYPE, vec.y);
		k_v = Complex(Complex::K_TYPE, vec.z);
	}
	Quaternion()
	{
		real = Complex(Complex::REAL, 0.f);
		i_v = Complex(Complex::I_TYPE, 0.f);
		j_v = Complex(Complex::J_TYPE, 0.f);
		k_v = Complex(Complex::K_TYPE, 0.f);
	}
	inline Quaternion conjugate() const
	{
		Quaternion n;
		n.real.val = real.val;
		n.i_v.val = -i_v.val;
		n.j_v.val = -j_v.val;
		n.k_v.val = -k_v.val;
		return n;
	}
	Quaternion operator *(const Quaternion& q) const
	{
		//use foil to multiply out all values
		std::vector<Complex> foil;

		for (int i = 0; i < 4; i++)
		{
			for (int j = 0; j < 4; j++)
			{
				foil.push_back(raw[i] * q.raw[j]);
			}
		}

		//sum all values in foil into new quaternion
		Quaternion out;
		for (int i = 0; i < foil.size(); i++)
		{
			switch (foil[i].t)
			{
			case Complex::REAL:
			{
				out.real.val += foil[i].val;
				continue;
			}
			case Complex::I_TYPE:
			{
				out.i_v.val += foil[i].val;
				continue;
			}
			case Complex::J_TYPE:
			{
				out.j_v.val += foil[i].val;
				continue;
			}
			case Complex::K_TYPE:
			{
				out.k_v.val += foil[i].val;
				continue;
			}
			default:
				//shouldnt reach this
				throw std::invalid_argument("Invaide Quaternion Args??\n");
			}
		}

		//default return
		return out;
	}
	inline operator Vec3f() const { return Vec3f(i_v.val, j_v.val, k_v.val); }
	inline float get_angle() const { return angle; }
private:
	union {
		struct {
			Complex real;
			Complex i_v;
			Complex j_v;
			Complex k_v;
		};
		Complex raw[4];
	};

	float angle = 0.f;;
};

struct Rotation
{
	union
	{
		struct
		{
			Quaternion x;
			Quaternion y;
			Quaternion z;
		};
		Quaternion raw[3];
	};
	int order[3];

	Rotation() { x = Quaternion(0.f, Vec3f(1.f, 0.f, 0.f)); y = Quaternion(0.f, Vec3f(0.f, 1.f, 0.f)); z = Quaternion(0.f, Vec3f(0.f, 0.f, 1.f)); order[0] = 0; order[1] = 1; order[2] = 2; }
};
