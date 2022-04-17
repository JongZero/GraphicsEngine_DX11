inline bool ECollision::Ray::operator==(const Ray& r) const noexcept
{
	EMath::Vector3 _r1p = Position;
	EMath::Vector3 _r2p = r.Position;

	EMath::Vector3 _r1d = Direction;
	EMath::Vector3 _r2d = r.Direction;
	return _r1p == _r2p && _r1d == _r2d;
}

inline bool ECollision::Ray::operator!=(const Ray& r) const noexcept
{
	EMath::Vector3 _r1p = Position;
	EMath::Vector3 _r2p = r.Position;

	EMath::Vector3 _r1d = Direction;
	EMath::Vector3 _r2d = r.Direction;
	return _r1p != _r2p && _r1d != _r2d;
}

inline bool ECollision::Ray::Intersects(const BoundingSphere& sphere, float& dist) const noexcept
{
	return sphere.Intersects(Position, Direction, dist);
}

inline bool ECollision::Ray::Intersects(const BoundingBox& box, float& dist) const noexcept
{
	return box.Intersects(Position, Direction, dist);
}
