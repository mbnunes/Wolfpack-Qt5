#if !defined(__VECTOR_H__)
#define __VECTOR_H__

#include <math.h>

class QString;

// Vector class
class cVector {
public:
	float x, y, z; // Components of this vector

	// Create a new vector
	cVector(float x, float y, float z);
	cVector();

	// Set all three components at once
	void set(float x, float y, float z);

	// Add a vector to this
	cVector operator + (const cVector &vector) const;

	// Kreutz product
	cVector operator * (const cVector &vector) const;

	// Product
	cVector operator * (const float scalar) const;

	// Normalize the vector
	cVector &normalize();

    // Convert this vector to a string
	QString toString();
};

inline cVector::cVector() {
}

inline cVector cVector::operator * (const cVector &vector) const {
	// Calculate the kreutzproduct
	cVector result(y * vector.z - z * vector.y,
		z * vector.x - x * vector.z,
		x * vector.y - y * vector.x);

	return result;
}

inline cVector cVector::operator * (const float scalar) const {
	return cVector(x * scalar, y * scalar, z * scalar);
}

inline cVector cVector::operator + (const cVector &vector) const {
	return cVector(vector.x + x, vector.y + y, vector.z + z);
}

inline cVector &cVector::normalize() {
	float normFactor = 1.0f / sqrt(x * x + y * y + z * z);
	x *= normFactor;
	y *= normFactor;
	z *= normFactor;
	return (*this);
}

inline void cVector::set(float x, float y, float z) {
	this->x = x;
	this->y = y;
	this->z = z;
}

inline cVector::cVector(float x, float y, float z) {
	set(x, y, z);
}

#endif
