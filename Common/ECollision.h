#pragma once
#include "EMath.h"

/// <summary>
/// DirectX Collision에 바로 접근하지 않고 ECollision을 한 번 거쳐 간다.
/// (게임 엔진과 DX의 종속성을 피하기 위해서, 랩핑)
/// 2021. 12. 23 정종영
/// </summary>
namespace ECollision
{
	struct BoundingBox;
	struct BoundingSphere;
	struct BoundingFrustum;

	struct Ray
	{
	public:
		EMath::Vector3 Position;
		EMath::Vector3 Direction;

	public:
		Ray() noexcept : Position(0, 0, 0), Direction(0, 0, 1) {}
		Ray(const EMath::Vector3& pos, const EMath::Vector3& dir) noexcept : Position(pos), Direction(dir) {}
		Ray(const EMath::Vector4& pos, const EMath::Vector4& dir) noexcept : Position(pos.x, pos.y, pos.z), Direction(dir.x, dir.y, dir.z) {}

		Ray(const Ray&) = default;
		Ray& operator=(const Ray&) = default;

		Ray(Ray&&) = default;
		Ray& operator=(Ray&&) = default;

		// Comparison operators
		bool operator == (const Ray & r) const noexcept;
		bool operator != (const Ray & r) const noexcept;

	public:
		// Ray operations
		bool Intersects(const BoundingSphere& sphere, float& dist) const noexcept;
		bool Intersects(const BoundingBox& box, float& dist) const noexcept;
		bool Intersects(const EMath::Vector3& tri0, const EMath::Vector3& tri1, const EMath::Vector3& tri2, float& Dist) const noexcept;
	};

	struct BoundingBox
	{
	public:
		EMath::Vector3 Center;		// 박스의 중심
		EMath::Vector3 Extents;		// 박스의 중심으로부터 각 면까지 거리

	public:
		BoundingBox() noexcept : Center(0, 0, 0), Extents(1.0f, 1.0f, 1.0f) {}

		BoundingBox(const BoundingBox&) = default;
		BoundingBox& operator= (const BoundingBox&) = default;

		BoundingBox(BoundingBox&&) = default;
		BoundingBox& operator= (BoundingBox&&) = default;

		BoundingBox(const EMath::Vector3& center, const EMath::Vector3& extents)
			: Center(center), Extents(extents) {}

	public:
		// Transform
		void Transform(BoundingBox& out, EMath::Matrix m) const;
		void Transform(BoundingBox& out, float scale, EMath::Quaternion rotation, EMath::Vector3 translation) const;

	public:
		// Collision Check
		bool Intersects(const BoundingSphere& bs) const;
		bool Intersects(const BoundingBox& box) const;
		bool Intersects(const BoundingFrustum& fr) const;

		bool Intersects(EMath::Vector4 v0, EMath::Vector4 v1, EMath::Vector4 v2) const;			// Triangle - Box
		bool Intersects(EMath::Vector4 origin, EMath::Vector4 direction, float& dist) const;	// Ray - Box

	public:
		// Static Func
		static void CreateFromSphere(BoundingBox& out, const BoundingSphere& bs);
	};

	struct BoundingSphere
	{
	public:
		EMath::Vector3 Center;      // 구의 중심
		float Radius;               // 구의 반지름

	public:
		BoundingSphere() noexcept : Center(0, 0, 0), Radius(1.f) {}

		BoundingSphere(const BoundingSphere&) = default;
		BoundingSphere& operator=(const BoundingSphere&) = default;

		BoundingSphere(BoundingSphere&&) = default;
		BoundingSphere& operator=(BoundingSphere&&) = default;

		BoundingSphere(const EMath::Vector3& center, float radius)
			: Center(center), Radius(radius) {}

	public:
		// Transform
		void Transform(BoundingSphere& out, EMath::Matrix m) const;
		void Transform(BoundingSphere& out, float scale, EMath::Quaternion rotation, EMath::Vector4 translation) const;

	public:
		// Collision Check
		bool Intersects(const BoundingSphere& sh) const;
		bool Intersects(const BoundingBox& box) const;
		bool Intersects(const BoundingFrustum& fr) const;

		bool Intersects(EMath::Vector4 v0, EMath::Vector4 v1, EMath::Vector4 v2) const;			// Triangle - Sphere
		bool Intersects(EMath::Vector4 origin, EMath::Vector4 direction, float& dist) const;	// Ray - Sphere

	public:
		// Static Func
		static void CreateFromBoundingBox(BoundingSphere& out, const BoundingBox& box);
	};

	struct BoundingFrustum
	{
	public:
		EMath::Vector3 Origin;            // Origin of the frustum (and projection).
		EMath::Vector4 Orientation;       // Quaternion representing rotation.

		float RightSlope;           // Positive X (X/Z)
		float LeftSlope;            // Negative X
		float TopSlope;             // Positive Y (Y/Z)
		float BottomSlope;          // Negative Y
		float Near, Far;            // Z of the near plane and far plane.

	public:
		BoundingFrustum() noexcept :
			Origin(0, 0, 0), Orientation(0, 0, 0, 1.f), RightSlope(1.f), LeftSlope(-1.f),
			TopSlope(1.f), BottomSlope(-1.f), Near(0), Far(1.f) {}

		BoundingFrustum(const BoundingFrustum&) = default;
		BoundingFrustum& operator=(const BoundingFrustum&) = default;

		BoundingFrustum(BoundingFrustum&&) = default;
		BoundingFrustum& operator=(BoundingFrustum&&) = default;

		BoundingFrustum(const EMath::Vector3& _Origin, const EMath::Vector4& _Orientation,
			float _RightSlope, float _LeftSlope, float _TopSlope, float _BottomSlope,
			float _Near, float _Far)
			: Origin(_Origin), Orientation(_Orientation),
			RightSlope(_RightSlope), LeftSlope(_LeftSlope), TopSlope(_TopSlope), BottomSlope(_BottomSlope),
			Near(_Near), Far(_Far) {}

	public:
		// Transform
		void Transform(BoundingFrustum& out, EMath::Matrix m) const;
		void Transform(BoundingFrustum& out, float scale, EMath::Vector4 rotation, EMath::Vector4 translation) const;

	public:
		// Collision Check
		bool Intersects(const BoundingSphere& sh) const;
		bool Intersects(const BoundingBox& box) const;
		bool Intersects(const BoundingFrustum& fr) const;

	public:
		// Static Func
		static void CreateFromMatrix(BoundingFrustum& out, EMath::Matrix projection);
	};

#include "ECollision.inl"
}
