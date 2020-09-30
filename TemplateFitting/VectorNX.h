/*************************************************************************************************************************/
// This source is provided for NON-COMMERCIAL RESEARCH PURPOSES only, and is provided “as is” WITHOUT ANY WARRANTY; 
// without even the implied warranty of fitness for a particular purpose. The redistribution of the code is not permitted.
//
// If you use the source or part of it in a publication, cite the following paper:
// 
// A. Brunton, A. Salazar, T. Bolkart, S. Wuhrer
// Review of Statistical Shape Spaces for 3D Data with Comparative Analysis for Human Faces.
// Computer Vision and Image Understanding, 128:1-17, 2014
//
// Copyright (c) 2016 Timo Bolkart, Stefanie Wuhrer
/*************************************************************************************************************************/

#ifndef VECTORNX_H
#define VECTORNX_H

#include <math.h>
#include <assert.h>
#include <vector>
#include <float.h>

template<typename T, size_t DIM>
class VecNX
{
public:
	VecNX()
	{
		for(size_t i = 0; i < DIM; ++i)
		{
			m_values[i] = static_cast<T>(0.0);
		}
	}

	VecNX(T x, T y)
	{
		assert(DIM==2);
		m_values[0] = x;
		m_values[1] = y;
	}

	VecNX(T x, T y, T z)
	{
		assert(DIM==3);
		m_values[0] = x;
		m_values[1] = y;
		m_values[2] = z;
	}

	VecNX(const std::vector<T>& data)
	{
		assert(DIM==data.size());

		for(size_t i = 0; i < DIM; ++i)
		{
			m_values[i] = data[i];
		}
	}

	VecNX(const VecNX& vec)
	{
		assert(getDim()==vec.getDim());
		*this = vec;
	}

	~VecNX()
	{}

	VecNX& operator=(const VecNX& vec)
	{
		if(this!=&vec)
		{
			assert(DIM==vec.getDim());
			for(int i = 0; i < DIM; ++i)
			{
				m_values[i] = vec[i];
			}
		}

		return *this;
	}

	T& operator[](size_t i)
	{
		assert(i<DIM);
		return m_values[i];
	}

	const T& operator[](size_t i) const
	{
		assert(i<DIM);
		return m_values[i];
	}

	VecNX operator+(const VecNX& vec)
	{
		assert(DIM==vec.getDim());

		VecNX tmpVec(*this);
		tmpVec += vec;
		return tmpVec;
	}

	VecNX operator+(const VecNX& vec) const
	{
		assert(DIM==vec.getDim());

		VecNX tmpVec(*this);
		tmpVec += vec;
		return tmpVec;
	}

	VecNX& operator+=(const VecNX& vec)
	{
		assert(DIM==vec.getDim());

		for(int i = 0; i < DIM; ++i)
		{
			m_values[i] += vec[i];
		}

		return *this;
	}

	VecNX operator-(const VecNX& vec) const
	{
		assert(DIM==vec.getDim());

		VecNX tmpVec(*this);
		tmpVec -= vec;
		return tmpVec;
	}

	VecNX operator-=(const VecNX& vec)
	{
		assert(DIM==vec.getDim());

		for(int i = 0; i < DIM; ++i)
		{
			m_values[i] -= vec[i];
		}

		return *this;
	}

	VecNX operator*(const T& scalar) const
	{
		VecNX<T, DIM> vec = *this;
		vec.scalarMult(scalar);
		return vec;
	}

	T operator*(const VecNX<T,DIM>& vec) const
	{
		return dotProduct(vec);
	}


	VecNX& operator*=(const T& scalar)
	{
		scalarMult(scalar);
		return *this;
	}

	int getDim() const { return DIM; }

	bool normalize()
	{
		const T norm = length();
		if(norm <= static_cast<T>(DBL_EPSILON))
		{
			return false;
		}
		
		scalarMult(1.0 / norm);
		return true;
	}

	T sqrLength() const
	{
		return dotProduct(*this);
	}

	T length() const
	{
		return sqrt(sqrLength());	
	}

	void scalarMult(T mult)
	{
		for(size_t i = 0; i < DIM; ++i)
		{
			m_values[i]*=mult;
		}
	}

	T dotProduct(const VecNX<T,DIM>& vec) const
	{
		T val = static_cast<T>(0.0);
		for(size_t i = 0; i < DIM; ++i)
		{
			val += m_values[i]*vec[i];
		}

		return val;
	}

	//Returns clampled angle to domain 0°-90°
	double angle(const VecNX<T,DIM>& vec) const
	{
		const T product = dotProduct(vec);
		const T lengthA = length();
		const T lengthB = vec.length();
		
		const double param = static_cast<double>(product/(lengthA*lengthB));
		const double clampParam = std::min<double>(std::max<double>(param, 0.0), 1.0);
		const double angle = acos(clampParam);
		//const double angle = acos(param);
		return angle*180.0/M_PI;
	}

	VecNX<T,3> crossProduct(const VecNX<T,3>& vec) const
	{
		VecNX<T,3> outVec;
		crossProduct(vec, outVec);
		return outVec;
	}

	void crossProduct(const VecNX<T,3>& vec, VecNX<T,3>& outVec) const
	{
		assert(DIM==3);
		outVec[0] = m_values[1]*vec[2]-m_values[2]*vec[1];
		outVec[1] = m_values[2]*vec[0]-m_values[0]*vec[2];
		outVec[2] = m_values[0]*vec[1]-m_values[1]*vec[0];
	}

	void projectOnVector(const VecNX<T,DIM>& vec, VecNX<T,DIM>& outVec) const 
	{
		const T numerator = dotProduct(vec);
		const T length = vec.length();
		const T denominator = length*length;

		const T factor = numerator / denominator;
		outVec = vec;
		outVec.scalarMult(factor);
	}

private:
	T m_values[DIM];
};

typedef VecNX<double,2> Vec2d;
typedef VecNX<double,3> Vec3d;
typedef VecNX<double,4> Vec4d;

typedef VecNX<int,2> Vec2i;
typedef VecNX<int,3> Vec3i;
typedef VecNX<int,4> Vec4i;

#endif