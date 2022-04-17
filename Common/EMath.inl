#pragma once

/// Vector2 ///////////////////////////////////////////////////////////////
/***********************************
		Operators
************************************/

inline EMath::Vector2 EMath::Vector2::operator=(const Vector2& v) noexcept
{
	this->x = v.x;
	this->y = v.y;

	return *this;
}

inline bool EMath::Vector2::operator==(const Vector2& v) const noexcept
{
	return ((this->x == v.x) && (this->y == v.y));
}

inline bool EMath::Vector2::operator!=(const Vector2& v) const noexcept
{
	return ((this->x != v.x) && (this->y != v.y));
}

inline EMath::Vector2 EMath::operator+(const Vector2& v1, const Vector2& v2) noexcept
{
	Vector2 _result;

	_result.x = v1.x + v2.x;
	_result.y = v1.y + v2.y;

	return _result;
}

inline EMath::Vector2 EMath::operator-(const Vector2& v1, const Vector2& v2) noexcept
{
	Vector2 _result;

	_result.x = v1.x - v2.x;
	_result.y = v1.y - v2.y;

	return _result;
}

inline EMath::Vector2 EMath::operator*(const Vector2& v1, const Vector2& v2) noexcept
{
	Vector2 _result;

	_result.x = v1.x * v2.x;
	_result.y = v1.y * v2.y;

	return _result;
}

inline EMath::Vector2 EMath::operator*(const Vector2& v, float s) noexcept
{
	Vector2 _result;

	_result.x = v.x * s;
	_result.y = v.y * s;

	return _result;
}

inline EMath::Vector2 EMath::operator/(const Vector2& v1, const Vector2& v2) noexcept
{
	Vector2 _result;

	_result.x = v1.x / v2.x;
	_result.y = v1.y / v2.y;

	return _result;
}

inline EMath::Vector2 EMath::operator/(const Vector2& v, float s) noexcept
{
	Vector2 _result;

	_result.x = v.x / s;
	_result.y = v.y / s;

	return _result;
}

inline EMath::Vector2 EMath::operator*(float s, const Vector2& v) noexcept
{
	Vector2 _result;

	_result.x = v.x * s;
	_result.y = v.y * s;

	return _result;
}

inline EMath::Vector2& EMath::Vector2::operator+=(const Vector2& v) noexcept
{
	this->x += v.x;
	this->y += v.y;

	return *this;
}

inline EMath::Vector2& EMath::Vector2::operator-=(const Vector2& v) noexcept
{
	this->x -= v.x;
	this->y -= v.y;

	return *this;
}

inline EMath::Vector2& EMath::Vector2::operator*=(const Vector2& v) noexcept
{
	this->x *= v.x;
	this->y *= v.y;

	return *this;
}

inline EMath::Vector2& EMath::Vector2::operator/=(const Vector2& v) noexcept
{
	this->x /= v.x;
	this->y /= v.y;

	return *this;
}

inline EMath::Vector2& EMath::Vector2::operator+=(float s) noexcept
{
	this->x += s;
	this->y += s;

	return *this;
}

inline EMath::Vector2& EMath::Vector2::operator-=(float s) noexcept
{
	this->x -= s;
	this->y -= s;

	return *this;
}

inline EMath::Vector2& EMath::Vector2::operator*=(float s) noexcept
{
	this->x *= s;
	this->y *= s;

	return *this;
}

inline EMath::Vector2& EMath::Vector2::operator/=(float s) noexcept
{
	this->x /= s;
	this->y /= s;

	return *this;
}
/// ////////////////////////////////////////////////////////////////////////

/// Vector3 ///////////////////////////////////////////////////////////////
/***********************************
		Operators
************************************/

inline EMath::Vector3 EMath::Vector3::operator=(const Vector3& v) noexcept
{
	this->x = v.x;
	this->y = v.y;
	this->z = v.z;

	return *this;
}

inline bool EMath::Vector3::operator==(const Vector3& v) const noexcept
{
	return ((this->x == v.x) && (this->y == v.y) && (this->z == v.z));
}

inline bool EMath::Vector3::operator!=(const Vector3& v) const noexcept
{
	return ((this->x != v.x) && (this->y != v.y) && (this->z != v.z));
}

inline EMath::Vector3 EMath::operator+(const Vector3& v1, const Vector3& v2) noexcept
{
	Vector3 _result;

	_result.x = v1.x + v2.x;
	_result.y = v1.y + v2.y;
	_result.z = v1.z + v2.z;

	return _result;
}

inline EMath::Vector3 EMath::operator-(const Vector3& v1, const Vector3& v2) noexcept
{
	Vector3 _result;

	_result.x = v1.x - v2.x;
	_result.y = v1.y - v2.y;
	_result.z = v1.z - v2.z;

	return _result;
}

inline EMath::Vector3 EMath::operator*(const Vector3& v1, const Vector3& v2) noexcept
{
	Vector3 _result;

	_result.x = v1.x * v2.x;
	_result.y = v1.y * v2.y;
	_result.z = v1.z * v2.z;

	return _result;
}

inline EMath::Vector3 EMath::operator*(const Vector3& v, float s) noexcept
{
	Vector3 _result;

	_result.x = v.x * s;
	_result.y = v.y * s;
	_result.z = v.z * s;

	return _result;
}

inline EMath::Vector3 EMath::operator/(const Vector3& v1, const Vector3& v2) noexcept
{
	Vector3 _result;

	_result.x = v1.x / v2.x;
	_result.y = v1.y / v2.y;
	_result.z = v1.z / v2.z;

	return _result;
}

inline EMath::Vector3 EMath::operator/(const Vector3& v, float s) noexcept
{
	Vector3 _result;

	_result.x = v.x / s;
	_result.y = v.y / s;
	_result.z = v.z / s;

	return _result;
}

inline EMath::Vector3 EMath::operator*(float s, const Vector3& v) noexcept
{
	Vector3 _result;

	_result.x = v.x * s;
	_result.y = v.y * s;
	_result.z = v.z * s;

	return _result;
}

inline EMath::Vector3& EMath::Vector3::operator+=(const Vector3& v) noexcept
{
	this->x += v.x;
	this->y += v.y;
	this->z += v.z;

	return *this;
}

inline EMath::Vector3& EMath::Vector3::operator-=(const Vector3& v) noexcept
{
	this->x -= v.x;
	this->y -= v.y;
	this->z -= v.z;

	return *this;
}

inline EMath::Vector3& EMath::Vector3::operator*=(const Vector3& v) noexcept
{
	this->x *= v.x;
	this->y *= v.y;
	this->z *= v.z;

	return *this;
}

inline EMath::Vector3& EMath::Vector3::operator/=(const Vector3& v) noexcept
{
	this->x /= v.x;
	this->y /= v.y;
	this->z /= v.z;

	return *this;
}

inline EMath::Vector3& EMath::Vector3::operator+=(float s) noexcept
{
	this->x += s;
	this->y += s;
	this->z += s;

	return *this;
}

inline EMath::Vector3& EMath::Vector3::operator-=(float s) noexcept
{
	this->x -= s;
	this->y -= s;
	this->z -= s;

	return *this;
}

inline EMath::Vector3& EMath::Vector3::operator*=(float s) noexcept
{
	this->x *= s;
	this->y *= s;
	this->z *= s;

	return *this;
}

inline EMath::Vector3& EMath::Vector3::operator/=(float s) noexcept
{
	this->x /= s;
	this->y /= s;
	this->z /= s;

	return *this;
}
/// ////////////////////////////////////////////////////////////////////////

/// Vector4 ///////////////////////////////////////////////////////////////
/***********************************
		Operators
************************************/

inline EMath::Vector4 EMath::Vector4::operator=(const Vector4& v) noexcept
{
	this->x = v.x;
	this->y = v.y;
	this->z = v.z;
	this->w = v.w;

	return *this;
}

inline bool EMath::Vector4::operator==(const Vector4& v) const noexcept
{
	return ((this->x == v.x) && (this->y == v.y) && (this->z == v.z) && (this->w == v.w));
}

inline bool EMath::Vector4::operator!=(const Vector4& v) const noexcept
{
	return ((this->x != v.x) && (this->y != v.y) && (this->z != v.z) && (this->w != v.w));
}

inline EMath::Vector4 EMath::operator+(const Vector4& v1, const Vector4& v2) noexcept
{
	Vector4 _result;

	_result.x = v1.x + v2.x;
	_result.y = v1.y + v2.y;
	_result.z = v1.z + v2.z;
	_result.w = v1.w + v2.w;

	return _result;
}

inline EMath::Vector4 EMath::operator-(const Vector4& v1, const Vector4& v2) noexcept
{
	Vector4 _result;

	_result.x = v1.x - v2.x;
	_result.y = v1.y - v2.y;
	_result.z = v1.z - v2.z;
	_result.w = v1.w - v2.w;

	return _result;
}

inline EMath::Vector4 EMath::operator*(const Vector4& v1, const Vector4& v2) noexcept
{
	Vector4 _result;

	_result.x = v1.x * v2.x;
	_result.y = v1.y * v2.y;
	_result.z = v1.z * v2.z;
	_result.w = v1.w * v2.w;

	return _result;
}

inline EMath::Vector4 EMath::operator*(const Vector4& v, float s) noexcept
{
	Vector4 _result;

	_result.x = v.x * s;
	_result.y = v.y * s;
	_result.z = v.z * s;
	_result.w = v.w * s;

	return _result;
}

inline EMath::Vector4 EMath::operator/(const Vector4& v1, const Vector4& v2) noexcept
{
	Vector4 _result;

	_result.x = v1.x / v2.x;
	_result.y = v1.y / v2.y;
	_result.z = v1.z / v2.z;
	_result.w = v1.w / v2.w;

	return _result;
}

inline EMath::Vector4 EMath::operator/(const Vector4& v, float s) noexcept
{
	Vector4 _result;

	_result.x = v.x / s;
	_result.y = v.y / s;
	_result.z = v.z / s;
	_result.w = v.w / s;

	return _result;
}

inline EMath::Vector4 EMath::operator*(float s, const Vector4& v) noexcept
{
	Vector4 _result;

	_result.x = v.x * s;
	_result.y = v.y * s;
	_result.z = v.z * s;
	_result.w = v.w * s;

	return _result;
}

inline EMath::Vector4& EMath::Vector4::operator+=(const Vector4& v) noexcept
{
	this->x += v.x;
	this->y += v.y;
	this->z += v.z;
	this->w += v.w;

	return *this;
}

inline EMath::Vector4& EMath::Vector4::operator-=(const Vector4& v) noexcept
{
	this->x -= v.x;
	this->y -= v.y;
	this->z -= v.z;
	this->w -= v.w;

	return *this;
}

inline EMath::Vector4& EMath::Vector4::operator*=(const Vector4& v) noexcept
{
	this->x *= v.x;
	this->y *= v.y;
	this->z *= v.z;
	this->w *= v.w;

	return *this;
}

inline EMath::Vector4& EMath::Vector4::operator/=(const Vector4& v) noexcept
{
	this->x /= v.x;
	this->y /= v.y;
	this->z /= v.z;
	this->w /= v.w;

	return *this;
}

inline EMath::Vector4& EMath::Vector4::operator+=(float s) noexcept
{
	this->x += s;
	this->y += s;
	this->z += s;
	this->w += s;

	return *this;
}

inline EMath::Vector4& EMath::Vector4::operator-=(float s) noexcept
{
	this->x -= s;
	this->y -= s;
	this->z -= s;
	this->w -= s;

	return *this;
}

inline EMath::Vector4& EMath::Vector4::operator*=(float s) noexcept
{
	this->x *= s;
	this->y *= s;
	this->z *= s;
	this->w *= s;

	return *this;
}

inline EMath::Vector4& EMath::Vector4::operator/=(float s) noexcept
{
	this->x /= s;
	this->y /= s;
	this->z /= s;
	this->w /= s;

	return *this;
}
/// ////////////////////////////////////////////////////////////////////////

/// Matrix /////////////////////////////////////////////////////////////////
/***********************************
		Operators
************************************/

inline bool EMath::Matrix::operator==(const Matrix& m) const noexcept
{
	return (Vector4(_11, _12, _13, _14) == Vector4(m._11, m._12, m._13, m._14)
		&& Vector4(_21, _22, _23, _24) == Vector4(m._21, m._22, m._23, m._24)
		&& Vector4(_31, _32, _33, _34) == Vector4(m._31, m._32, m._33, m._34)
		&& Vector4(_41, _42, _43, _44) == Vector4(m._41, m._42, m._43, m._44)) != 0;
}

inline bool EMath::Matrix::operator!=(const Matrix& m) const noexcept
{
	return (Vector4(_11, _12, _13, _14) != Vector4(m._11, m._12, m._13, m._14)
		|| Vector4(_21, _22, _23, _24) != Vector4(m._21, m._22, m._23, m._24)
		|| Vector4(_31, _32, _33, _34) != Vector4(m._31, m._32, m._33, m._34)
		|| Vector4(_41, _42, _43, _44) != Vector4(m._41, m._42, m._43, m._44)) != 0;
}

inline EMath::Matrix& EMath::Matrix::operator+=(const Matrix& m) noexcept
{
	this->_11 += m._11; this->_12 += m._12; this->_13 += m._13; this->_14 += m._14;
	this->_21 += m._21; this->_22 += m._22; this->_23 += m._23; this->_24 += m._24;
	this->_31 += m._31; this->_32 += m._32; this->_33 += m._33; this->_34 += m._34;
	this->_41 += m._41; this->_42 += m._42; this->_43 += m._43; this->_44 += m._44;

	return *this;
}

inline EMath::Matrix& EMath::Matrix::operator-=(const Matrix& m) noexcept
{
	this->_11 -= m._11; this->_12 -= m._12; this->_13 -= m._13; this->_14 -= m._14;
	this->_21 -= m._21; this->_22 -= m._22; this->_23 -= m._23; this->_24 -= m._24;
	this->_31 -= m._31; this->_32 -= m._32; this->_33 -= m._33; this->_34 -= m._34;
	this->_41 -= m._41; this->_42 -= m._42; this->_43 -= m._43; this->_44 -= m._44;

	return *this;
}

inline EMath::Matrix& EMath::Matrix::operator*=(float s) noexcept
{
	this->_11 *= s; this->_12 *= s; this->_13 *= s; this->_14 *= s;
	this->_21 *= s; this->_22 *= s; this->_23 *= s; this->_24 *= s;
	this->_31 *= s; this->_32 *= s; this->_33 *= s; this->_34 *= s;
	this->_41 *= s; this->_42 *= s; this->_43 *= s; this->_44 *= s;

	return *this;
}

inline EMath::Matrix& EMath::Matrix::operator/=(float s) noexcept
{
	this->_11 /= s; this->_12 /= s; this->_13 /= s; this->_14 /= s;
	this->_21 /= s; this->_22 /= s; this->_23 /= s; this->_24 /= s;
	this->_31 /= s; this->_32 /= s; this->_33 /= s; this->_34 /= s;
	this->_41 /= s; this->_42 /= s; this->_43 /= s; this->_44 /= s;

	return *this;
}
/// ////////////////////////////////////////////////////////////////////////

/// Quaternion /////////////////////////////////////////////////////////////
/***********************************
		Operators
************************************/
inline bool EMath::Quaternion::operator==(const Quaternion& q) const noexcept
{
	return ((this->x == q.x)
		&& (this->y == q.y)
		&& (this->z == q.z)
		&& (this->w == q.w));
}

inline bool EMath::Quaternion::operator!=(const Quaternion& q) const noexcept
{
	return ((this->x != q.x)
		|| (this->y != q.y)
		|| (this->z != q.z)
		|| (this->w != q.w));
}

inline EMath::Quaternion EMath::Quaternion::operator+(const Quaternion& m) noexcept
{
	return Quaternion(this->x + m.x, this->y + m.y, this->z + m.z, this->w + m.w);
}

inline EMath::Quaternion EMath::Quaternion::operator-(const Quaternion& m) noexcept
{
	return Quaternion(this->x - m.x, this->y - m.y, this->z - m.z, this->w - m.w);
}

inline EMath::Quaternion EMath::Quaternion::operator*(const Quaternion& m) noexcept
{
	return Quaternion(this->x * m.x, this->y * m.y, this->z * m.z, this->w * m.w);
}

inline EMath::Quaternion EMath::Quaternion::operator/(const Quaternion& m) noexcept
{
	return Quaternion(this->x / m.x, this->y / m.y, this->z / m.z, this->w / m.w);
}

inline EMath::Quaternion& EMath::Quaternion::operator+=(const Quaternion& q) noexcept
{
	this->x += q.x;
	this->y += q.y;
	this->z += q.z;
	this->w += q.w;
}

inline EMath::Quaternion& EMath::Quaternion::operator-=(const Quaternion& q) noexcept
{
	this->x -= q.x;
	this->y -= q.y;
	this->z -= q.z;
	this->w -= q.w;
}

inline EMath::Quaternion& EMath::Quaternion::operator*=(const Quaternion& q) noexcept
{
	this->x *= q.x;
	this->y *= q.y;
	this->z *= q.z;
	this->w *= q.w;
}

inline EMath::Quaternion& EMath::Quaternion::operator*=(float S) noexcept
{
	this->x *= S;
	this->y *= S;
	this->z *= S;
	this->w *= S;
}

inline EMath::Quaternion& EMath::Quaternion::operator/=(const Quaternion& q) noexcept
{
	this->x /= q.x;
	this->y /= q.y;
	this->z /= q.z;
	this->w /= q.w;
}
/// ////////////////////////////////////////////////////////////////////////
