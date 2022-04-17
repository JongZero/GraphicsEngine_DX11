#pragma once

// x바이트 정렬
// 바이트 정렬하고 싶을 때 구조체 앞에 선언하면 됨
// EX)
// ALIGN(16) struct XMFLOAT2
#define ALIGN(x) __declspec(align(x))

/// <summary>
/// SimpleMath의 Vector와 Matrix를 랩핑한다.
/// (SimpleMath를 쓰기위해 DirectX를 알아야만하는 구조에서 벗어나기위해)
/// 2021. 10. 22. 정종영, 방소연
/// </summary>
namespace EMath
{
	struct Vector2;
	struct Vector3;
	struct Vector4;
	struct Matrix;
	struct Quaternion;
	
	struct FLOAT2
	{
	public:
		float x;
		float y;

	public:
		FLOAT2(const FLOAT2&) = default;
		FLOAT2& operator=(const FLOAT2&) = default;

		FLOAT2(FLOAT2&&) = default;
		FLOAT2& operator=(FLOAT2&&) = default;

		FLOAT2() : x(0), y(0) {}
		FLOAT2(float _x, float _y) : x(_x), y(_y) {}
	};

	struct FLOAT3
	{
	public:
		float x;
		float y;
		float z;

	public:
		FLOAT3(const FLOAT3&) = default;
		FLOAT3& operator=(const FLOAT3&) = default;

		FLOAT3(FLOAT3&&) = default;
		FLOAT3& operator=(FLOAT3&&) = default;

		FLOAT3() : x(0), y(0), z(0) {}
		FLOAT3(float _x, float _y, float _z) : x(_x), y(_y), z(_z) {}
	};

	struct FLOAT4
	{
	public:
		float x;
		float y;
		float z;
		float w;

	public:
		FLOAT4(const FLOAT4&) = default;
		FLOAT4& operator=(const FLOAT4&) = default;

		FLOAT4(FLOAT4&&) = default;
		FLOAT4& operator=(FLOAT4&&) = default;

		FLOAT4() : x(0), y(0), z(0), w(0) {}
		FLOAT4(float _x, float _y, float _z, float _w) : x(_x), y(_y), z(_z), w(_w) {}
	};

	struct FLOAT4X4
	{
	public:
		union
		{
			struct
			{
				float _11, _12, _13, _14;
				float _21, _22, _23, _24;
				float _31, _32, _33, _34;
				float _41, _42, _43, _44;
			};
			float m[4][4];
		};

	public:
		FLOAT4X4() noexcept
			: _11(1.0f), _12(0), _13(0), _14(0),
			_21(0), _22(1.0f), _23(0), _24(0),
			_31(0), _32(0), _33(1.0f), _34(0),
			_41(0), _42(0), _43(0), _44(1.0f) {}

		FLOAT4X4(float m00, float m01, float m02, float m03,
			float m10, float m11, float m12, float m13,
			float m20, float m21, float m22, float m23,
			float m30, float m31, float m32, float m33) noexcept
			: _11(m00), _12(m01), _13(m02), _14(m03),
			_21(m10), _22(m11), _23(m12), _24(m13),
			_31(m20), _32(m21), _33(m22), _34(m23),
			_41(m30), _42(m31), _43(m32), _44(m33) {}

		FLOAT4X4(const FLOAT4X4&) = default;
		FLOAT4X4& operator=(const FLOAT4X4&) = default;

		FLOAT4X4(FLOAT4X4&&) = default;
		FLOAT4X4& operator=(FLOAT4X4&&) = default;
	};

	struct Vector2 : public FLOAT2
	{
	public:
		Vector2(FLOAT2 f2) : FLOAT2(f2) {}
		Vector2() noexcept : FLOAT2(0,0) {}
		Vector2(float x, float y) noexcept : FLOAT2(x, y) {}

		Vector2(const Vector2&) = default;

	public:
		Vector2 operator= (const Vector2& v) noexcept;

		bool operator == (const Vector2& v) const noexcept;
		bool operator != (const Vector2& v) const noexcept;

		Vector2& operator+= (const Vector2& v) noexcept;
		Vector2& operator-= (const Vector2& v) noexcept;
		Vector2& operator*= (const Vector2& v) noexcept;
		Vector2& operator/= (const Vector2& v) noexcept;

		Vector2& operator+= (float s) noexcept;
		Vector2& operator-= (float s) noexcept;
		Vector2& operator*= (float s) noexcept;
		Vector2& operator/= (float s) noexcept;

	public:
		float Length() noexcept;

		float Dot(const Vector2& v) noexcept;
		void Cross(const Vector2& v, Vector2& result) noexcept;
		Vector2 Cross(const Vector2& v) noexcept;

		void Normalize() noexcept;
		void Normalize(Vector2& result) noexcept;

		void Clamp(const Vector2& vmin, const Vector2& vmax) noexcept;
		void Clamp(const Vector2& vmin, const Vector2& vmax, Vector2& result) noexcept;

	public:
		/// Static Functions
		static float Distance(const Vector2& v1, const Vector2& v2) noexcept;

		static void Min(const Vector2& v1, const Vector2& v2, Vector2& result) noexcept;
		static Vector2 Min(const Vector2& v1, const Vector2& v2) noexcept;

		static void Max(const Vector2& v1, const Vector2& v2, Vector2& result) noexcept;
		static Vector2 Max(const Vector2& v1, const Vector2& v2) noexcept;

		static void Lerp(const Vector2& v1, const Vector2& v2, float t, Vector2& result) noexcept;
		static Vector2 Lerp(const Vector2& v1, const Vector2& v2, float t) noexcept;

		static void Transform(const Vector2& v, const Quaternion& quat, Vector2& result) noexcept;
		static Vector2 Transform(const Vector2& v, const Quaternion& quat) noexcept;

		static void Transform(const Vector2& v, const Matrix& m, Vector2& result) noexcept;
		static Vector2 Transform(const Vector2& v, const Matrix& m) noexcept;

		static void Transform(const Vector2& v, const Matrix& m, Vector4& result) noexcept;

		static void TransformNormal(const Vector2& v, const Matrix& m, Vector2& result) noexcept;
		static Vector2 TransformNormal(const Vector2& v, const Matrix& m) noexcept;

		static const Vector2 Identity;
	};

	Vector2 operator+ (const Vector2& v1, const Vector2& v2) noexcept;
	Vector2 operator- (const Vector2& v1, const Vector2& v2) noexcept;
	Vector2 operator* (const Vector2& v1, const Vector2& v2) noexcept;
	Vector2 operator* (const Vector2& v, float s) noexcept;
	Vector2 operator/ (const Vector2& v1, const Vector2& v2) noexcept;
	Vector2 operator/ (const Vector2& v, float s) noexcept;
	Vector2 operator* (float s, const Vector2& v) noexcept;

	struct Vector3 : public FLOAT3
	{
	public:
		Vector3(FLOAT3 f3) : FLOAT3(f3) {}
		Vector3() noexcept : FLOAT3(0, 0, 0) {}
		Vector3(float x, float y, float z) noexcept : FLOAT3(x, y, z) {}

		Vector3(const Vector3&) = default;

	public:
		Vector3 operator= (const Vector3& v) noexcept;

		bool operator == (const Vector3& v) const noexcept;
		bool operator != (const Vector3& v) const noexcept;

		Vector3& operator+= (const Vector3& v) noexcept;
		Vector3& operator-= (const Vector3& v) noexcept;
		Vector3& operator*= (const Vector3& v) noexcept;
		Vector3& operator/= (const Vector3& v) noexcept;

		Vector3& operator+= (float s) noexcept;
		Vector3& operator-= (float s) noexcept;
		Vector3& operator*= (float s) noexcept;
		Vector3& operator/= (float s) noexcept;

		float Length() noexcept;

		float Dot(const Vector3& v) noexcept;
		void Cross(const Vector3& v, Vector3& result) noexcept;
		Vector3 Cross(const Vector3& v) noexcept;

		void Normalize() noexcept;
		void Normalize(Vector3& result) noexcept;

		void Clamp(const Vector3& vmin, const Vector3& vmax) noexcept;
		void Clamp(const Vector3& vmin, const Vector3& vmax, Vector3& result) noexcept;

	public:
		/// Static Functions
		static float Distance(const Vector3& v1, const Vector3& v2) noexcept;

		static void Min(const Vector3& v1, const Vector3& v2, Vector3& result) noexcept;
		static Vector3 Min(const Vector3& v1, const Vector3& v2) noexcept;

		static void Max(const Vector3& v1, const Vector3& v2, Vector3& result) noexcept;
		static Vector3 Max(const Vector3& v1, const Vector3& v2) noexcept;

		static void Lerp(const Vector3& v1, const Vector3& v2, float t, Vector3& result) noexcept;
		static Vector3 Lerp(const Vector3& v1, const Vector3& v2, float t) noexcept;

		static void Transform(const Vector3& v, const Quaternion& quat, Vector3& result) noexcept;
		static Vector3 Transform(const Vector3& v, const Quaternion& quat) noexcept;

		static void Transform(const Vector3& v, const Matrix& m, Vector3& result) noexcept;
		static Vector3 Transform(const Vector3& v, const Matrix& m) noexcept;
		static Vector4 TransformCoord(const Vector4& v, const Matrix& m) noexcept;

		static void Transform(const Vector3& v, const Matrix& m, Vector4& result) noexcept;

		static void TransformNormal(const Vector3& v, const Matrix& m, Vector3& result) noexcept;
		static Vector3 TransformNormal(const Vector3& v, const Matrix& m) noexcept;
		static Vector4 TransformNormal(const Vector4& v, const Matrix& m) noexcept;

		static const Vector3 Identity;
	};

	// Binary operators
	Vector3 operator+ (const Vector3& v1, const Vector3& v2) noexcept;
	Vector3 operator- (const Vector3& v1, const Vector3& v2) noexcept;
	Vector3 operator* (const Vector3& v1, const Vector3& v2) noexcept;
	Vector3 operator* (const Vector3& v, float s) noexcept;
	Vector3 operator/ (const Vector3& v1, const Vector3& v2) noexcept;
	Vector3 operator/ (const Vector3& v, float s) noexcept;
	Vector3 operator* (float s, const Vector3& v) noexcept;

	struct Vector4 : public FLOAT4
	{
	public:
		Vector4(FLOAT4 f4) : FLOAT4(f4) {}
		Vector4() noexcept : FLOAT4(0, 0, 0, 0) {}
		Vector4(float x, float y, float z, float w) noexcept : FLOAT4(x, y, z, w) {}
		Vector4(Vector3& v) noexcept : FLOAT4(v.x, v.y, v.z, 0) {}
		Vector4(const Vector3& v) noexcept : FLOAT4(v.x, v.y, v.z, 0) {}

		Vector4(const Vector4&) = default;

	public:
		Vector4 operator= (const Vector4& v) noexcept;

		bool operator == (const Vector4& v) const noexcept;
		bool operator != (const Vector4& v) const noexcept;

		Vector4& operator+= (const Vector4& v) noexcept;
		Vector4& operator-= (const Vector4& v) noexcept;
		Vector4& operator*= (const Vector4& v) noexcept;
		Vector4& operator/= (const Vector4& v) noexcept;

		Vector4& operator+= (float s) noexcept;
		Vector4& operator-= (float s) noexcept;
		Vector4& operator*= (float s) noexcept;
		Vector4& operator/= (float s) noexcept;

		float Length() noexcept;

		float Dot(const Vector4& v) noexcept;
		void Cross(const Vector4& v1, const Vector4& v2, Vector4& result) noexcept;
		Vector4 Cross(const Vector4& v1, const Vector4& v2) noexcept;

		void Normalize() noexcept;
		void Normalize(Vector4& result) noexcept;

		void Clamp(const Vector4& vmin, const Vector4& vmax) noexcept;
		void Clamp(const Vector4& vmin, const Vector4& vmax, Vector4& result) noexcept;

	public:
		/// Static Functions
		static float Distance(const Vector4& v1, const Vector4& v2) noexcept;

		static void Min(const Vector4& v1, const Vector4& v2, Vector4& result) noexcept;
		static Vector4 Min(const Vector4& v1, const Vector4& v2) noexcept;

		static void Max(const Vector4& v1, const Vector4& v2, Vector4& result) noexcept;
		static Vector4 Max(const Vector4& v1, const Vector4& v2) noexcept;

		static void Lerp(const Vector4& v1, const Vector4& v2, float t, Vector4& result) noexcept;
		static Vector4 Lerp(const Vector4& v1, const Vector4& v2, float t) noexcept;

		static void Transform(const Vector4& v, const Quaternion& quat, Vector4& result) noexcept;
		static Vector4 Transform(const Vector4& v, const Quaternion& quat) noexcept;

		static void Transform(const Vector4& v, const Matrix& m, Vector4& result) noexcept;
		static Vector4 Transform(const Vector4& v, const Matrix& m) noexcept;

		static const Vector4 Identity;
	};

	// Binary operators
	Vector4 operator+ (const Vector4& v1, const Vector4& v2) noexcept;
	Vector4 operator- (const Vector4& v1, const Vector4& v2) noexcept;
	Vector4 operator* (const Vector4& v1, const Vector4& v2) noexcept;
	Vector4 operator* (const Vector4& v, float s) noexcept;
	Vector4 operator/ (const Vector4& v1, const Vector4& v2) noexcept;
	Vector4 operator/ (const Vector4& v, float s) noexcept;
	Vector4 operator* (float s, const Vector4& v) noexcept;

	struct Matrix : public FLOAT4X4
	{
	public:
		Matrix() noexcept
			: FLOAT4X4(1.0f, 0, 0, 0,
						0, 1.0f, 0, 0,
						0, 0, 1.0f, 0,
						0, 0, 0, 1.0f) {}

		Matrix(float m00, float m01, float m02, float m03,
			float m10, float m11, float m12, float m13,
			float m20, float m21, float m22, float m23,
			float m30, float m31, float m32, float m33) noexcept
			: FLOAT4X4(m00, m01, m02, m03,
						m10, m11, m12, m13,
						m20, m21, m22, m23,
						m30, m31, m32, m33) {}

		Matrix(FLOAT4X4 m) : FLOAT4X4(m) {}

		Matrix(const Vector3& r0, const Vector3& r1, const Vector3& r2) noexcept
			: Matrix(r0.x, r0.y, r0.z, 0.0f,
				r1.x, r1.y, r1.z, 0.0f,
				r2.x, r2.y, r2.z, 0.0f,
				0.0f, 0.0f, 0.0f, 1.f) {}

		Matrix(const Vector4& r0, const Vector4& r1, const Vector4& r2, const Vector4& r3) noexcept
			: Matrix(r0.x, r0.y, r0.z, r0.w,
				r1.x, r1.y, r1.z, r1.w,
				r2.x, r2.y, r2.z, r2.w,
				r3.x, r3.y, r3.z, r3.w) {}

		Matrix(const Matrix&) = default;
		Matrix& operator=(const Matrix&) = default;

	public:
		bool operator == (const Matrix& m) const noexcept;
		bool operator != (const Matrix& m) const noexcept;

		Matrix& operator+= (const Matrix& m) noexcept;
		Matrix& operator-= (const Matrix& m) noexcept;
		Matrix& operator*= (const Matrix& m) noexcept;
		Matrix& operator/= (const Matrix& m) noexcept;

		Matrix& operator*= (float s) noexcept;
		Matrix& operator/= (float s) noexcept;

		Vector3 Translation() const  noexcept { return Vector3(_41, _42, _43); }
		void Translation(const Vector3& v) noexcept { _41 = v.x; _42 = v.y; _43 = v.z; }

	public:
		// Matrix operations
		bool Decompose(Vector3& scale, Quaternion& rotation, Vector3& translation) noexcept;

		Matrix Transpose() const noexcept;
		void Transpose(Matrix& result) const noexcept;

		Matrix Invert() const noexcept;
		void Invert(Matrix& result) const noexcept;

		float Determinant() const noexcept;

	public:
		// Static functions
		static Matrix CreateTranslation(const Vector3& position) noexcept;
		static Matrix CreateTranslation(float x, float y, float z) noexcept;

		static Matrix CreateScale(const Vector3& scales) noexcept;
		static Matrix CreateScale(float xs, float ys, float zs) noexcept;
		static Matrix CreateScale(float scale) noexcept;

		static Matrix CreateRotationX(float radians) noexcept;
		static Matrix CreateRotationY(float radians) noexcept;
		static Matrix CreateRotationZ(float radians) noexcept;

		static Matrix CreateFromAxisAngle(const Vector3& axis, float angle) noexcept;

		static Matrix CreatePerspectiveFieldOfView(float fov, float aspectRatio, float nearPlane, float farPlane) noexcept;
		static Matrix CreatePerspective(float width, float height, float nearPlane, float farPlane) noexcept;
		static Matrix CreatePerspectiveOffCenter(float left, float right, float bottom, float top, float nearPlane, float farPlane) noexcept;
		static Matrix CreateOrthographic(float width, float height, float zNearPlane, float zFarPlane) noexcept;
		static Matrix CreateOrthographicOffCenter(float left, float right, float bottom, float top, float zNearPlane, float zFarPlane) noexcept;

		static Matrix CreateLookAt(const Vector3& position, const Vector3& target, const Vector3& up) noexcept;
		static Matrix CreateWorld(const Vector3& position, const Vector3& forward, const Vector3& up) noexcept;

		static Matrix CreateFromQuaternion(const Quaternion& quat) noexcept;

		static Matrix CreateFromYawPitchRoll(float yaw, float pitch, float roll) noexcept;

		static void Lerp(const Matrix& M1, const Matrix& M2, float t, Matrix& result) noexcept;
		static Matrix Lerp(const Matrix& M1, const Matrix& M2, float t) noexcept;

		static void Transform(const Matrix& M, const Quaternion& rotation, Matrix& result) noexcept;
		static Matrix Transform(const Matrix& M, const Quaternion& rotation) noexcept;
		
		// Constants
		static const Matrix Identity;
	};

	// Binary operators
	Matrix operator+ (const Matrix& m1, const Matrix& m2) noexcept;
	Matrix operator- (const Matrix& m1, const Matrix& m2) noexcept;
	Matrix operator* (const Matrix& m1, const Matrix& m2) noexcept;
	Matrix operator* (const Matrix& m, float s) noexcept;
	Matrix operator/ (const Matrix& m, float s) noexcept;
	Matrix operator/ (const Matrix& m1, const Matrix& m2) noexcept;
	Matrix operator* (float s, const Matrix& m) noexcept;

	struct Quaternion : public FLOAT4
	{
	public:
		Quaternion() noexcept : FLOAT4(0, 0, 0, 1) {}
		Quaternion(float ix, float iy, float iz, float iw) noexcept : FLOAT4(ix, iy, iz, iw) {}
		Quaternion(const Vector3& v, float scalar) noexcept : FLOAT4(v.x, v.y, v.z, scalar) {}
		Quaternion(const Vector4& v) noexcept : FLOAT4(v.x, v.y, v.z, v.w) {}

		Quaternion(const Quaternion&) = default;
		Quaternion& operator=(const Quaternion&) = default;

	public:
		// Comparison operators
		bool operator == (const Quaternion& q) const noexcept;
		bool operator != (const Quaternion& q) const noexcept;

		Quaternion operator+ (const Quaternion& m) noexcept;
		Quaternion operator- (const Quaternion& m) noexcept;
		Quaternion operator* (const Quaternion& m) noexcept;
		Quaternion operator/ (const Quaternion& m) noexcept;

		// Assignment operators
		Quaternion& operator+= (const Quaternion& q) noexcept;
		Quaternion& operator-= (const Quaternion& q) noexcept;
		Quaternion& operator*= (const Quaternion& q) noexcept;
		Quaternion& operator*= (float S) noexcept;
		Quaternion& operator/= (const Quaternion& q) noexcept;

	public:
		// Quaternion operations
		float Length() const noexcept;
		float LengthSquared() const noexcept;

		void Normalize() noexcept;
		void Normalize(Quaternion& result) const noexcept;

		void Conjugate() noexcept;
		void Conjugate(Quaternion& result) const noexcept;

		void Inverse(Quaternion& result) const noexcept;

		float Dot(const Quaternion& q) const noexcept;

	public:
		// Static functions
		static Quaternion CreateFromAxisAngle(const Vector3& axis, float angle) noexcept;
		static Quaternion CreateFromYawPitchRoll(float yaw, float pitch, float roll) noexcept;
		static Quaternion CreateFromRotationMatrix(const Matrix& M) noexcept;

		static void Lerp(const Quaternion& q1, const Quaternion& q2, float t, Quaternion& result) noexcept;
		static Quaternion Lerp(const Quaternion& q1, const Quaternion& q2, float t) noexcept;

		static void Slerp(const Quaternion& q1, const Quaternion& q2, float t, Quaternion& result) noexcept;
		static Quaternion Slerp(const Quaternion& q1, const Quaternion& q2, float t) noexcept;

		static void Concatenate(const Quaternion& q1, const Quaternion& q2, Quaternion& result) noexcept;
		static Quaternion Concatenate(const Quaternion& q1, const Quaternion& q2) noexcept;

		// Constants
		static const Quaternion Identity;
	};

	static const float PI = 3.1415926535f;
	static const float PI2 = 6.283185307f;
	float ConvertToRadians(float d);


	// 인라인 함수
	#include "EMath.inl"
} // EMath
