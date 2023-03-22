#include "Engine/Core/VertexUtils.hpp"
#include "Engine/Core/EngineCommon.hpp"

constexpr int NUM_CIRCLE_TRIANGLES = 16;

void TransformVertexArrayXY3D(int numberVerts, Vertex_PCU* verts, float scaleXY, float zRotationDegrees, Vec2 const& translationXY)
{
	for (int vertexIndex = 0; vertexIndex < numberVerts; vertexIndex++)
	{
		Vertex_PCU& vertex = verts[vertexIndex];
		TransformPositionXY3D( vertex.m_position, scaleXY, zRotationDegrees, translationXY);
	}
}


void TransformVertexArrayXYZ3D(int numberVerts, Vertex_PCU* verts, Vec3 const& iBasis, Vec3 const& jBasis, Vec3 const& kBasis, Vec3 const& translationXYZ)
{
	for (int vertexIndex = 0; vertexIndex < numberVerts; vertexIndex++)
	{
		Vertex_PCU& vertex = verts[vertexIndex];
		TransformPositionXYZ3D(vertex.m_position, iBasis, jBasis, kBasis, translationXYZ);
	}
}


void AddVertsForCapsule2D(std::vector<Vertex_PCU>& verts, Capsule2 const& capsule, Rgba8 const& color, const AABB2& UVs)
{
	UNUSED(UVs)

	Vec2 boneDisplacement = capsule.m_bone.m_end - capsule.m_bone.m_start;
	Vec2 center = capsule.m_bone.m_start + 0.5f * boneDisplacement;
	Vec2 iNormal = boneDisplacement.GetRotatedMinus90Degrees().GetNormalized();
	Vec2 halfDimension(capsule.m_radius, 0.5f * boneDisplacement.GetLength());
	
	OBB2 box(center, iNormal, halfDimension);
	AddVertsForOBB2D(verts, box, color);
	AddVertsForOrientedSector2D(verts, capsule.m_bone.m_end, boneDisplacement.GetOrientationDegrees(), 180.f, capsule.m_radius, color);
	AddVertsForOrientedSector2D(verts, capsule.m_bone.m_start, boneDisplacement.GetOrientationDegrees() + 180.f, 180.f, capsule.m_radius, color);
}


void AddVertsForDiscs2D(std::vector<Vertex_PCU>& verts, Vec2 const& center, float radius, Rgba8 const& color, const AABB2& UVs)
{

	constexpr float DEGREES_PER_CIRLE_SECTOR = 360.f / NUM_CIRCLE_TRIANGLES;
	Vec3 discCenter(center.x, center.y, 0.f);
	Vec3 firstTip = discCenter + Vec3(radius * CosDegrees(0.f), radius * SinDegrees(0.f), 0.f);
	float firstTipU = RangeMap(.5f * (CosDegrees(0.f) + 1.f), 0.f, 1.f, UVs.m_mins.x, UVs.m_maxs.x);
	float firstTipV = RangeMap(.5f * (SinDegrees(0.f) + 1.f), 0.f, 1.f, UVs.m_mins.y, UVs.m_maxs.y);
	Vec2 firstTipUVs = Vec2(firstTipU, firstTipV);
	for (int triangleIndex = 0; triangleIndex < NUM_CIRCLE_TRIANGLES; triangleIndex++)
	{
		float curDegrees = (triangleIndex + 1) * DEGREES_PER_CIRLE_SECTOR;
		float secondTipU = RangeMap(.5f * (CosDegrees(curDegrees) + 1.f), 0.f, 1.f, UVs.m_mins.x, UVs.m_maxs.x);
		float secondTipV = RangeMap(.5f * (SinDegrees(curDegrees) + 1.f), 0.f, 1.f, UVs.m_mins.y, UVs.m_maxs.y);
		Vec2 secondTipUVs = Vec2(secondTipU, secondTipV);
		Vec3 secondTip = discCenter + Vec3(radius * CosDegrees(curDegrees), radius * SinDegrees(curDegrees), 0.f);
		verts.emplace_back(discCenter, color, Vec2(0.5f, 0.5f));
		verts.emplace_back(firstTip, color, firstTipUVs);
		verts.emplace_back(secondTip, color, secondTipUVs);
		firstTip = secondTip;
		firstTipUVs = secondTipUVs;
	}
}


void AddVertsForAABB2D(std::vector<Vertex_PCU>& verts, AABB2 const& bounds, Rgba8 const& color, Vec2 const& uvMins, Vec2 const& uvMaxs)
{
	Vec3 pos0(Vec2(bounds.m_mins.x, bounds.m_mins.y));
	Vec3 pos1(Vec2(bounds.m_maxs.x, bounds.m_mins.y));
	Vec3 pos2(Vec2(bounds.m_maxs.x, bounds.m_maxs.y));
	Vec3 pos3(Vec2(bounds.m_mins.x, bounds.m_maxs.y));

	verts.emplace_back(pos0, color, Vec2(uvMins.x, uvMins.y));
	verts.emplace_back(pos1, color, Vec2(uvMaxs.x, uvMins.y));
	verts.emplace_back(pos2, color, Vec2(uvMaxs.x, uvMaxs.y));

	verts.emplace_back(pos0, color, Vec2(uvMins.x, uvMins.y));
	verts.emplace_back(pos2, color, Vec2(uvMaxs.x, uvMaxs.y));
	verts.emplace_back(pos3, color, Vec2(uvMins.x, uvMaxs.y));
}


void AddVertsForAABB2D(std::vector<Vertex_PCU>& verts, AABB2 const& bounds, Rgba8 const& color, AABB2 const& UVs)
{
	AddVertsForAABB2D(verts, bounds, color, UVs.m_mins, UVs.m_maxs);
}


void AddVertsForHollowAABB2D(std::vector<Vertex_PCU>& verts, AABB2 const& bounds, float thickness, Rgba8 const& color)
{
	float halfThickness = 0.5f * thickness;
	Vec2 pos0(bounds.m_mins.x + halfThickness, bounds.m_mins.y + halfThickness);
	Vec2 pos1(bounds.m_maxs.x - halfThickness, bounds.m_mins.y + halfThickness);
	Vec2 pos2(bounds.m_maxs.x - halfThickness, bounds.m_maxs.y - halfThickness);
	Vec2 pos3(bounds.m_mins.x + halfThickness, bounds.m_maxs.y - halfThickness);

	AddVertsForLineSegment2D(verts, LineSegment2(pos0, pos1), thickness, color);
	AddVertsForLineSegment2D(verts, LineSegment2(pos1, pos2), thickness, color);
	AddVertsForLineSegment2D(verts, LineSegment2(pos2, pos3), thickness, color);
	AddVertsForLineSegment2D(verts, LineSegment2(pos3, pos0), thickness, color);
}


//void AddVertsForAABB2D(std::vector<Vertex_PCU>& verts, AABB2 const& bounds, Rgba8 const& color, AABB2 const& UVs)
//{
//	AddVertsForAABB2D(verts, bounds, color, UVs.m_mins, UVs.m_maxs);
//}


void AddVertsForOBB2D(std::vector<Vertex_PCU>& verts, OBB2 const& box, Rgba8 const& color, Vec2 const& uvMins, Vec2 const& uvMaxs)
{
	std::vector<Vec2> corners;
	corners.resize(4);
	box.GetCornerPoints(&corners[0]);

	Vec3 pos0(corners[0]);
	Vec3 pos1(corners[1]);
	Vec3 pos2(corners[2]);
	Vec3 pos3(corners[3]);

	verts.emplace_back(pos0, color, Vec2(uvMins.x, uvMins.y));
	verts.emplace_back(pos1, color, Vec2(uvMaxs.x, uvMins.y));
	verts.emplace_back(pos2, color, Vec2(uvMaxs.x, uvMaxs.y));

	verts.emplace_back(pos0, color, Vec2(uvMins.x, uvMins.y));
	verts.emplace_back(pos2, color, Vec2(uvMaxs.x, uvMaxs.y));
	verts.emplace_back(pos3, color, Vec2(uvMins.x, uvMaxs.y));
}


void AddVertsForConvex2D(std::vector<Vertex_PCU>& verts, ConvexPoly2D const& convex, Rgba8 const& color, AABB2 const& UVs)
{
	UNUSED(UVs)

	std::vector<Vec2> points = convex.GetPoints();
	if (points.size() < 3)
	{
		ERROR_AND_DIE("Invalid Convex Polygon");
	}

	Vec3 end(points[points.size() - 1]);
	for (int index = 0; index < (int)points.size() - 2; index++)
	{
		Vec3 pos1(points[index]);
		Vec3 pos2(points[index + 1]);

		verts.emplace_back(pos1, color, Vec2::ZERO);
		verts.emplace_back(pos2, color, Vec2::ZERO);
		verts.emplace_back(end, color, Vec2::ZERO);
	}
}


void AddVertsForConvexOutline2D(std::vector<Vertex_PCU>& verts, ConvexPoly2D const& convex, float thickness, Rgba8 const& color, AABB2 const& UVs)
{
	std::vector<Vec2> points = convex.GetPoints();
	if (points.size() < 3)
	{
		ERROR_AND_DIE("Invalid Convex Polygon");
	}

	for (int index = 0; index < (int)points.size() - 1; index++)
	{
		Vec2 const& pos1 = points[index];
		Vec2 const& pos2 = points[index + 1];
		AddVertsForLineSegment2D(verts, LineSegment2(pos1, pos2), thickness, color, UVs);
	}

	AddVertsForLineSegment2D(verts, LineSegment2(points[points.size() - 1], points[0]), thickness, color, UVs);
}


//void AddVertsForOBB2D(std::vector<Vertex_PCU>& verts, OBB2 const& box, Rgba8 const& color, AABB2 const& UVs)
//{
//	AddVertsForOBB2D(verts, box, color, UVs.m_mins, UVs.m_maxs);
//}


void AddVertsForLineSegment2D(std::vector<Vertex_PCU>& verts, LineSegment2 const& lineSegment, float thickness, Rgba8 const& color, AABB2 const& UVs)
{
	UNUSED(UVs)

	Vec2 directional = (lineSegment.m_end - lineSegment.m_start).GetNormalized() * thickness * 0.5f;
	Vec2 perpendicular = directional.GetRotated90Degrees();

	Vec2 bottomLeft = lineSegment.m_start - directional + perpendicular;
	Vec2 bottomRight = lineSegment.m_start - directional - perpendicular;
	Vec2 topRight = lineSegment.m_end + directional - perpendicular;
	Vec2 topLeft = lineSegment.m_end + directional + perpendicular;

	Vec3 pos0(bottomLeft);
	Vec3 pos1(bottomRight);
	Vec3 pos2(topRight);
	Vec3 pos3(topLeft);

	verts.emplace_back(pos0, color, Vec2(0.f, 0.f));
	verts.emplace_back(pos1, color, Vec2(1.f, 0.f));
	verts.emplace_back(pos2, color, Vec2(1.f, 1.f));

	verts.emplace_back(pos0, color, Vec2(0.f, 0.f));
	verts.emplace_back(pos2, color, Vec2(1.f, 1.f));
	verts.emplace_back(pos3, color, Vec2(0.f, 1.f));
}


void AddVertsForRing2D(std::vector<Vertex_PCU>& verts, Vec2 const& center, float radius, float thickness, float slices, Rgba8 const& color, AABB2 const& UVs)
{
	float pieDegrees = 360.f / slices;
	float halfWdith = 0.5f * thickness;
	for (float curDegress = 0.f; curDegress < 359.f; curDegress += pieDegrees)
	{
		float lesserDegrees = curDegress;
		float moreDegrees = curDegress + pieDegrees;
		Vec3 closerLess = Vec3(center + Vec2((radius - halfWdith) * CosDegrees(lesserDegrees),
			(radius - halfWdith) * SinDegrees(lesserDegrees)));
		Vec3 farLess = Vec3(center + Vec2((radius + halfWdith) * CosDegrees(lesserDegrees),
			(radius + halfWdith) * SinDegrees(lesserDegrees)));
		Vec3 closerMore = Vec3(center + Vec2((radius - halfWdith) * CosDegrees(moreDegrees),
			(radius - halfWdith) * SinDegrees(moreDegrees)));
		Vec3 farMore = Vec3(center + Vec2((radius + halfWdith) * CosDegrees(moreDegrees),
			(radius + halfWdith) * SinDegrees(moreDegrees)));

		verts.emplace_back(closerMore, color, UVs.m_mins);
		verts.emplace_back(closerLess, color, Vec2(UVs.m_maxs.x, UVs.m_mins.y));
		verts.emplace_back(farLess, color, UVs.m_maxs);
		verts.emplace_back(closerMore, color, UVs.m_mins);
		verts.emplace_back(farLess, color, UVs.m_maxs);
		verts.emplace_back(farMore, color, Vec2(UVs.m_mins.x, UVs.m_maxs.y));
	}
}


void AddVertsForOrientedSector2D(std::vector<Vertex_PCU>& verts, Vec2 const& sectorTip, float sectorForwardDegrees, float sectorApertureDegrees, float sectorRadius, Rgba8 const& color, AABB2 const& UVs)
{
	UNUSED(UVs)

	int sectorTriangleCount = RoundDownToInt(RangeMap(sectorApertureDegrees, 0.f, 360.f, 0.f, static_cast<float>(NUM_CIRCLE_TRIANGLES)));
	float degreesPerTriangle = sectorApertureDegrees / sectorTriangleCount;
	Vec3 sectorTipPos(sectorTip.x, sectorTip.y, 0.f);
	float startDegrees = sectorForwardDegrees - 0.5f * sectorApertureDegrees;
	Vec3 firstTip = sectorTipPos + Vec3(sectorRadius * CosDegrees(startDegrees), sectorRadius * SinDegrees(startDegrees), 0.f);
	for (int triangleIndex = 0; triangleIndex < sectorTriangleCount; triangleIndex++)
	{
		float curDegrees = startDegrees + (triangleIndex + 1) * degreesPerTriangle;
		Vec3 secondTip = sectorTipPos + Vec3(sectorRadius * CosDegrees(curDegrees), sectorRadius * SinDegrees(curDegrees), 0.f);
		verts.emplace_back(sectorTipPos, color, Vec2(0.f, 0.f));
		verts.emplace_back(firstTip, color, Vec2(0.f, 0.f));
		verts.emplace_back(secondTip, color, Vec2(0.f, 0.f));
		firstTip = secondTip;
	}
}


void AddVertsForArrow2D(std::vector<Vertex_PCU>& verts, Vec2 const& tailPos, Vec2 const& tipPos, float arrowSize, float lineThickness, Rgba8 const& color)
{
	LineSegment2 line(tailPos, tipPos);
	AddVertsForLineSegment2D(verts, line, lineThickness, color);

	Vec2 arrowLine = (tailPos - tipPos).GetNormalized();
	arrowLine.SetLength(arrowSize);
	Vec2 leftArrowLineDisplacement = arrowLine.GetRotatedDegrees(-45.f);
	Vec2 rightArrowLineDisplacement = arrowLine.GetRotatedDegrees(45.f);
	LineSegment2 leftArrowLine(tipPos, tipPos + leftArrowLineDisplacement);
	LineSegment2 rightArrowLine(tipPos, tipPos + rightArrowLineDisplacement);
	AddVertsForLineSegment2D(verts, leftArrowLine, lineThickness, color);
	AddVertsForLineSegment2D(verts, rightArrowLine, lineThickness, color);
}


void AddVertsForQuad3D(std::vector<Vertex_PCU>& verts, const Vec3& bottomLeft, const Vec3& bottomRight, const Vec3& topRight, const Vec3& topLeft, const Rgba8& color, const AABB2& UVs)
{
	verts.emplace_back(bottomLeft, color, UVs.m_mins);
	verts.emplace_back(bottomRight, color, Vec2(UVs.m_maxs.x, UVs.m_mins.y));
	verts.emplace_back(topRight, color, UVs.m_maxs);

	verts.emplace_back(bottomLeft, color, UVs.m_mins);
	verts.emplace_back(topRight, color, UVs.m_maxs);
	verts.emplace_back(topLeft, color, Vec2(UVs.m_mins.x, UVs.m_maxs.y));
}


void AddVertsForAABB3D(std::vector<Vertex_PCU>& verts, const AABB3& bounds, const Rgba8& color, const AABB2& UVs)
{
	Vec3 corners[8] = {};
	bounds.GetCorners(corners);
	Vec3 nearBL = corners[0];
	Vec3 nearBR = corners[1];
	Vec3 nearTR = corners[2];
	Vec3 nearTL = corners[3];
	Vec3 farBL = corners[4];
	Vec3 farBR = corners[5];
	Vec3 farTR = corners[6];
	Vec3 farTL = corners[7];

	// x forward, y left, z up
	AddVertsForQuad3D(verts, farBR, farBL, farTL, farTR, color, UVs); //back quad +y
	AddVertsForQuad3D(verts, nearBL, nearBR, nearTR, nearTL, color, UVs); //front quad -y
	AddVertsForQuad3D(verts, farBL, nearBL, nearTL, farTL, color, UVs); //left quad -x
	AddVertsForQuad3D(verts, nearBR, farBR, farTR, nearTR, color, UVs); //right quad +x
	AddVertsForQuad3D(verts, nearTL, nearTR, farTR, farTL, color, UVs); //top quad +z
	AddVertsForQuad3D(verts, farBL, farBR, nearBR, nearBL, color, UVs); //bottom quad -z
}


void AddVertsForInvertedAABB3D(std::vector<Vertex_PCU>& verts, AABB3 const& bounds, Rgba8 const& color, AABB2 const& UVs)
{
	Vec3 corners[8] = {};
	bounds.GetCorners(corners);
	Vec3 nearBL = corners[0];
	Vec3 nearBR = corners[1];
	Vec3 nearTR = corners[2];
	Vec3 nearTL = corners[3];
	Vec3 farBL = corners[4];
	Vec3 farBR = corners[5];
	Vec3 farTR = corners[6];
	Vec3 farTL = corners[7];

	// x forward, y left, z up
	AddVertsForQuad3D(verts, farBL, farBR, farTR, farTL, color, UVs); //back quad +y
	AddVertsForQuad3D(verts, nearBR, nearBL, nearTL, nearTR, color, UVs); //front quad -y
	AddVertsForQuad3D(verts, nearBL, farBL, farTL, nearTL, color, UVs); //left quad -x
	AddVertsForQuad3D(verts, farBR, nearBR, nearTR, farTR, color, UVs); //right quad +x
	AddVertsForQuad3D(verts, nearTR, nearTL, farTL, farTR, color, UVs); //top quad +z
	AddVertsForQuad3D(verts, farBR, farBL, nearBL, nearBR, color, UVs); //bottom quad -z
}


void AddVertsForOBB3D(std::vector<Vertex_PCU>& verts, OBB3 const& bounds, Rgba8 const& color, AABB2 const& UVs)
{
	Vec3 corners[8] = {};
	bounds.GetCornerPoints(corners);
	Vec3 nearBL = corners[0];
	Vec3 nearBR = corners[1];
	Vec3 nearTR = corners[2];
	Vec3 nearTL = corners[3];
	Vec3 farBL = corners[4];
	Vec3 farBR = corners[5];
	Vec3 farTR = corners[6];
	Vec3 farTL = corners[7];

	// x forward, y left, z up
	AddVertsForQuad3D(verts, farBR, farBL, farTL, farTR, color, UVs); //back quad +y
	AddVertsForQuad3D(verts, nearBL, nearBR, nearTR, nearTL, color, UVs); //front quad -y
	AddVertsForQuad3D(verts, farBL, nearBL, nearTL, farTL, color, UVs); //left quad -x
	AddVertsForQuad3D(verts, nearBR, farBR, farTR, nearTR, color, UVs); //right quad +x
	AddVertsForQuad3D(verts, nearTL, nearTR, farTR, farTL, color, UVs); //top quad +z
	AddVertsForQuad3D(verts, farBL, farBR, nearBR, nearBL, color, UVs); //bottom quad -z
}


void AddVertsForCapsule3D(std::vector<Vertex_PCU>& verts, Capsule3 const& capsule, float longitudeSlices, float latitudeSlices, Rgba8 const& color, AABB2 const& UVs)
{
	Vec3 boneStart = capsule.m_bone.m_start;
	Vec3 boneEnd = capsule.m_bone.m_end;
	float radius = capsule.m_radius;

	// hacky version, better to have only the surface verts
	AddVertsForSphere3D(verts, boneStart, radius, longitudeSlices, latitudeSlices, color, UVs);
	AddVertsForSphere3D(verts, boneEnd, radius, longitudeSlices, latitudeSlices, color, UVs);
	AddVertsForCylinder3D(verts, boneStart, boneEnd, radius, longitudeSlices, color, UVs);
}


void AddVertsForWireAABB3D(std::vector<Vertex_PCU>& verts, AABB3 const& bounds, float wireWidth, Rgba8 const& color, AABB2 const& UVs)
{
	float wireHalfWidth = 0.5f * wireWidth;
	Vec3 corners[8] = {};
	bounds.GetCorners(corners);
	Vec3 nearBL = corners[0];
	Vec3 nearBR = corners[1];
	Vec3 nearTR = corners[2];
	Vec3 nearTL = corners[3];
	Vec3 farBL = corners[4];
	Vec3 farBR = corners[5];
	Vec3 farTR = corners[6];
	Vec3 farTL = corners[7];

	//top
	AddVertsForLine3D(verts, nearTL, nearTR, wireHalfWidth, color, UVs);
	AddVertsForLine3D(verts, nearTR, farTR, wireHalfWidth, color, UVs);
	AddVertsForLine3D(verts, farTL, farTR, wireHalfWidth, color, UVs);
	AddVertsForLine3D(verts, nearTL, farTL, wireHalfWidth, color, UVs);

	//bottom
	AddVertsForLine3D(verts, nearBL, nearBR, wireHalfWidth, color, UVs);
	AddVertsForLine3D(verts, nearBR, farBR, wireHalfWidth, color, UVs);
	AddVertsForLine3D(verts, farBL, farBR, wireHalfWidth, color, UVs);
	AddVertsForLine3D(verts, nearBL, farBL, wireHalfWidth, color, UVs);

	//side
	AddVertsForLine3D(verts, nearBL, nearTL, wireHalfWidth, color, UVs);
	AddVertsForLine3D(verts, nearBR, nearTR, wireHalfWidth, color, UVs);
	AddVertsForLine3D(verts, farBL, farTL, wireHalfWidth, color, UVs);
	AddVertsForLine3D(verts, farBR, farTR, wireHalfWidth, color, UVs);
}


void AddVertsForLine3D(std::vector<Vertex_PCU>& verts, Vec3 const& start, Vec3 const& end, float width, Rgba8 const& color, AABB2 const& UVs)
{
	Vec3 kBasis = (end - start).GetNormalized();
	Vec3 iBasis, jBasis;
	if (fabsf(DotProduct3D(kBasis, Vec3(1.f, 0.f, 0.f))) < .99f)
	{
		jBasis = CrossProduct3D(kBasis, Vec3(1.f, 0.f, 0.f)).GetNormalized();
		iBasis = CrossProduct3D(jBasis, kBasis).GetNormalized();
	}
	else
	{
		jBasis = CrossProduct3D(kBasis, Vec3(0.f, 1.f, 0.f)).GetNormalized();
		iBasis = CrossProduct3D(jBasis, kBasis).GetNormalized();
	}

	iBasis *= width;
	jBasis *= width;
	kBasis *= width;

	Vec3 nearBL = start - iBasis - jBasis - kBasis;
	Vec3 nearBR = start + iBasis - jBasis - kBasis;
	Vec3 nearTL = end - iBasis - jBasis + kBasis;
	Vec3 nearTR = end + iBasis - jBasis + kBasis;
	Vec3 farBL = start - iBasis + jBasis - kBasis;
	Vec3 farBR = start + iBasis + jBasis - kBasis;
	Vec3 farTL = end - iBasis + jBasis + kBasis;
	Vec3 farTR = end + iBasis + jBasis + kBasis;

	// x forward, y left, z up
	AddVertsForQuad3D(verts, farBR, farBL, farTL, farTR, color, UVs); //back quad +y
	AddVertsForQuad3D(verts, nearBL, nearBR, nearTR, nearTL, color, UVs); //front quad -y
	AddVertsForQuad3D(verts, farBL, nearBL, nearTL, farTL, color, UVs); //left quad -x
	AddVertsForQuad3D(verts, nearBR, farBR, farTR, nearTR, color, UVs); //right quad +x
	AddVertsForQuad3D(verts, nearTL, nearTR, farTR, farTL, color, UVs); //top quad +z
	AddVertsForQuad3D(verts, farBL, farBR, nearBR, nearBL, color, UVs); //bottom quad -z
}


void AddVertsForSphere3D(std::vector<Vertex_PCU>& verts, Vec3 const& center, float radius, float longitudeSlices, float latitudeSlices, Rgba8 const& color, const AABB2& UVs)
{
	float yawSlice = 360.f / longitudeSlices;
	float pitchSlice = 180.f / latitudeSlices;

	float prevYaw = 0.f;
	float prevPitch = -90.f;
	float prevU = 0.f;
	float prevV =  UVs.m_maxs.y;
	float prevCosYaw = CosDegrees(prevYaw);
	float prevSinYaw = SinDegrees(prevYaw);

	for (float yaw = yawSlice; yaw <= 361.f; yaw += yawSlice)
	{
		float cosYaw = CosDegrees(yaw);
		float sinYaw = SinDegrees(yaw);
		float u = RangeMap(yaw, 0.f, 360.f, UVs.m_mins.x, UVs.m_maxs.x);
		float prevCosPitch = CosDegrees(prevPitch);
		float prevSinPitch = SinDegrees(prevPitch);
		for (float pitch = -90.f + pitchSlice; pitch <= 91.f; pitch += pitchSlice)
		{
			float cosPitch = CosDegrees(pitch);
			float sinPitch = SinDegrees(pitch);
			float v = RangeMap(pitch, 90.f, -90.f, 0.f, 1.f);

			Vec3 bottomLeft(cosPitch * prevCosYaw, cosPitch * prevSinYaw, -sinPitch);
			Vec3 bottomRight(cosPitch * cosYaw, cosPitch * sinYaw, -sinPitch);
			Vec3 topRight(prevCosPitch * cosYaw, prevCosPitch * sinYaw, -prevSinPitch);
			Vec3 topLeft(prevCosPitch * prevCosYaw, prevCosPitch * prevSinYaw, -prevSinPitch);

			bottomLeft *= radius;
			bottomRight *= radius;
			topRight *= radius;
			topLeft *= radius;

			bottomLeft += center;
			bottomRight += center;
			topRight += center;
			topLeft += center;

			verts.emplace_back(bottomLeft, color, Vec2(prevU, v));
			verts.emplace_back(bottomRight, color, Vec2(u, v));
			verts.emplace_back(topRight, color, Vec2(u, prevV));
			
			verts.emplace_back(bottomLeft, color, Vec2(prevU, v));
			verts.emplace_back(topRight, color, Vec2(u, prevV));
			verts.emplace_back(topLeft, color, Vec2(prevU, prevV));

			prevCosPitch = cosPitch;
			prevSinPitch = sinPitch;
			prevV = v;
		}
		prevCosYaw = cosYaw;
		prevSinYaw = sinYaw;
		prevU = u;
		prevV = UVs.m_maxs.y;
	}
}


void AddVertsForWireSphere3D(std::vector<Vertex_PCU>& verts, Vec3 const& center, float radius, float longitudeSlices, float latitudeSlices, float wireWidth, Rgba8 const& color)
{
	float wireHalfWidth = 0.5f * wireWidth;
	float yawSlice = 360.f / longitudeSlices;
	float pitchSlice = 180.f / latitudeSlices;

	float prevYaw = 0.f;
	float prevPitch = -90.f;
	float prevCosYaw = CosDegrees(prevYaw);
	float prevSinYaw = SinDegrees(prevYaw);

	for (float yaw = yawSlice; yaw <= 361.f; yaw += yawSlice)
	{
		float cosYaw = CosDegrees(yaw);
		float sinYaw = SinDegrees(yaw);
		float prevCosPitch = CosDegrees(prevPitch);
		float prevSinPitch = SinDegrees(prevPitch);
		for (float pitch = -90.f + pitchSlice; pitch <= 91.f; pitch += pitchSlice)
		{
			float cosPitch = CosDegrees(pitch);
			float sinPitch = SinDegrees(pitch);

			Vec3 bottomLeft(cosPitch * prevCosYaw, cosPitch * prevSinYaw, -sinPitch);
			Vec3 bottomRight(cosPitch * cosYaw, cosPitch * sinYaw, -sinPitch);
			Vec3 topRight(prevCosPitch * cosYaw, prevCosPitch * sinYaw, -prevSinPitch);
			Vec3 topLeft(prevCosPitch * prevCosYaw, prevCosPitch * prevSinYaw, -prevSinPitch);

			bottomLeft *= radius;
			bottomRight *= radius;
			topRight *= radius;
			topLeft *= radius;

			bottomLeft += center;
			bottomRight += center;
			topRight += center;
			topLeft += center;

			AddVertsForLine3D(verts, bottomLeft, bottomRight, wireHalfWidth, color);
			AddVertsForLine3D(verts, bottomRight, topRight, wireHalfWidth, color);
			AddVertsForLine3D(verts, topRight, topLeft, wireHalfWidth, color);
			AddVertsForLine3D(verts, topLeft, bottomLeft, wireHalfWidth, color);

			prevCosPitch = cosPitch;
			prevSinPitch = sinPitch;
		}
		prevCosYaw = cosYaw;
		prevSinYaw = sinYaw;
	}
}


void AddVertsForCylinder3D(std::vector<Vertex_PCU>& verts, Vec3 const& start, Vec3 const& end, float radius, float slices, Rgba8 const& color, AABB2 const& UVs)
{
	Vec3 kBasis = (end - start).GetNormalized();
	Vec3 iBasis, jBasis;
	if (fabsf(DotProduct3D(kBasis, Vec3(1.f, 0.f, 0.f))) < .99f)
	{
		jBasis = CrossProduct3D(kBasis, Vec3(1.f, 0.f, 0.f)).GetNormalized();
		iBasis = CrossProduct3D(jBasis, kBasis).GetNormalized();
	}
	else
	{
		jBasis = CrossProduct3D(kBasis, Vec3(0.f, 1.f, 0.f)).GetNormalized();
		iBasis = CrossProduct3D(jBasis, kBasis).GetNormalized();
	}

	iBasis *= radius;
	jBasis *= radius;

	float degreesPerSlice = 360.f / slices;
	Vec3 firstTopTip = end + iBasis * CosDegrees(0.f) + jBasis * SinDegrees(0.f);
	Vec3 firstBottomTip = start + iBasis * CosDegrees(0.f) + jBasis * SinDegrees(0.f);
	Vec2 centerUV(0.5f * (UVs.m_mins.x + UVs.m_maxs.x), 0.5f * (UVs.m_mins.y + UVs.m_maxs.y));
	float discPrevU = RangeMap(.5f * (CosDegrees(0.f) + 1.f), 0.f, 1.f, UVs.m_mins.x, UVs.m_maxs.x);
	float topPrevV = RangeMap(.5f * (SinDegrees(0.f) + 1.f), 0.f, 1.f, UVs.m_mins.y, UVs.m_maxs.y);
	float bottomPrevV = RangeMap(.5f * (SinDegrees(0.f) + 1.f), 1.f, 0.f, UVs.m_mins.y, UVs.m_maxs.y);
	float sidePrevU = UVs.m_mins.x;

	for (float curDegrees = degreesPerSlice; curDegrees <= 361.f; curDegrees += degreesPerSlice)
	{
		Vec3 secondTopTip = end + iBasis * CosDegrees(curDegrees) + jBasis * SinDegrees(curDegrees);
		Vec3 secondBottomTip = start + iBasis * CosDegrees(curDegrees) + jBasis * SinDegrees(curDegrees);
		float discU = RangeMap(.5f * (CosDegrees(curDegrees) + 1.f), 0.f, 1.f, UVs.m_mins.x, UVs.m_maxs.x);
		float topV = RangeMap(.5f * (SinDegrees(curDegrees) + 1.f), 0.f, 1.f, UVs.m_mins.y, UVs.m_maxs.y);
		float bottomV = RangeMap(.5f * (SinDegrees(curDegrees) + 1.f), 1.f, 0.f, UVs.m_mins.y, UVs.m_maxs.y);
		float sideU = RangeMap(curDegrees, 0.f, 360.f, UVs.m_mins.x, UVs.m_maxs.x);

		//top pie
		verts.emplace_back(end, color, centerUV);
		verts.emplace_back(firstTopTip, color, Vec2(discPrevU, topPrevV));
		verts.emplace_back(secondTopTip, color, Vec2(discU, topV));

		//bottom pie
		verts.emplace_back(start, color, centerUV);
		verts.emplace_back(secondBottomTip, color, Vec2(discU, bottomV));
		verts.emplace_back(firstBottomTip, color, Vec2(discPrevU, bottomPrevV));

		//side panel
		verts.emplace_back(firstBottomTip, color, Vec2(sidePrevU, UVs.m_mins.y));
		verts.emplace_back(secondBottomTip, color, Vec2(sideU, UVs.m_mins.y));
		verts.emplace_back(secondTopTip, color, Vec2(sideU, UVs.m_maxs.y));

		verts.emplace_back(firstBottomTip, color, Vec2(sidePrevU, UVs.m_mins.y));
		verts.emplace_back(secondTopTip, color, Vec2(sideU,  UVs.m_maxs.y));
		verts.emplace_back(firstTopTip, color, Vec2(sidePrevU,  UVs.m_maxs.y));

		firstTopTip = secondTopTip;
		firstBottomTip = secondBottomTip;
		discPrevU = discU;
		topPrevV = topV;
		bottomPrevV = bottomV;
		sidePrevU = sideU;
	}
}


void AddVertsForWireCylinder3D(std::vector<Vertex_PCU>& verts, Vec3 const& start, Vec3 const& end, float radius, float slices, float wireWidth, Rgba8 const& color)
{
	float wireHalfWidth = 0.5f * wireWidth;
	Vec3 kBasis = (end - start).GetNormalized();
	Vec3 iBasis, jBasis;

	if (fabsf(DotProduct3D(kBasis, Vec3(1.f, 0.f, 0.f))) < 1.f)
	{
		jBasis = CrossProduct3D(kBasis, Vec3(1.f, 0.f, 0.f)).GetNormalized();
		iBasis = CrossProduct3D(jBasis, kBasis).GetNormalized();
	}
	else
	{
		jBasis = CrossProduct3D(kBasis, Vec3(0.f, 1.f, 0.f)).GetNormalized();
		iBasis = CrossProduct3D(jBasis, kBasis).GetNormalized();
	}

	iBasis *= radius;
	jBasis *= radius;

	float degreesPerSlice = 360.f / slices;
	Vec3 firstTopTip = end + iBasis * CosDegrees(0.f) + jBasis * SinDegrees(0.f);
	Vec3 firstBottomTip = start + iBasis * CosDegrees(0.f) + jBasis * SinDegrees(0.f);

	for (float curDegrees = degreesPerSlice; curDegrees <= 361.f; curDegrees += degreesPerSlice)
	{
		Vec3 secondTopTip = end + iBasis * CosDegrees(curDegrees) + jBasis * SinDegrees(curDegrees);
		Vec3 secondBottomTip = start + iBasis * CosDegrees(curDegrees) + jBasis * SinDegrees(curDegrees);

		//side panel
		AddVertsForLine3D(verts, firstBottomTip, secondBottomTip, wireHalfWidth, color);
		AddVertsForLine3D(verts, secondBottomTip, secondTopTip, wireHalfWidth, color);
		AddVertsForLine3D(verts, secondTopTip, firstTopTip, wireHalfWidth, color);
		AddVertsForLine3D(verts, firstTopTip, firstBottomTip, wireHalfWidth, color);

		firstTopTip = secondTopTip;
		firstBottomTip = secondBottomTip;
	}
}


void AddVertsForCone3D(std::vector<Vertex_PCU>& verts, Vec3 const& base, Vec3 const& tip, float radius, float slices, Rgba8 const& color, AABB2 const& UVs)
{
	Vec3 kBasis = (tip - base).GetNormalized();
	Vec3 iBasis, jBasis;
	if (fabsf(DotProduct3D(kBasis, Vec3(1.f, 0.f, 0.f))) < .99f)
	{
		jBasis = CrossProduct3D(kBasis, Vec3(1.f, 0.f, 0.f)).GetNormalized();
		iBasis = CrossProduct3D(jBasis, kBasis).GetNormalized();
	}
	else
	{
		jBasis = CrossProduct3D(kBasis, Vec3(0.f, 1.f, 0.f)).GetNormalized();
		iBasis = CrossProduct3D(jBasis, kBasis).GetNormalized();
	}

	iBasis *= radius;
	jBasis *= radius;

	float degreesPerSlice = 360.f / slices;
	Vec3 firstBottomTip = base + iBasis * CosDegrees(0.f) + jBasis * SinDegrees(0.f);
	Vec2 centerUV(0.5f * (UVs.m_mins.x + UVs.m_maxs.x), 0.5f * (UVs.m_mins.y + UVs.m_maxs.y));
	float discPrevU = RangeMap(.5f * (CosDegrees(0.f) + 1.f), 0.f, 1.f, UVs.m_mins.x, UVs.m_maxs.x);
	float bottomPrevV = RangeMap(.5f * (SinDegrees(0.f) + 1.f), 1.f, 0.f, UVs.m_mins.y, UVs.m_maxs.y);
	float sidePrevU = UVs.m_mins.x;

	for (float curDegrees = degreesPerSlice; curDegrees <= 360.1f; curDegrees += degreesPerSlice)
	{
		Vec3 secondBottomTip = base + iBasis * CosDegrees(curDegrees) + jBasis * SinDegrees(curDegrees);
		float discU = RangeMap(.5f * (CosDegrees(curDegrees) + 1.f), 0.f, 1.f, UVs.m_mins.x, UVs.m_maxs.x);
		float bottomV = RangeMap(.5f * (SinDegrees(curDegrees) + 1.f), 1.f, 0.f, UVs.m_mins.y, UVs.m_maxs.y);
		float sideU = RangeMap(curDegrees, 0.f, 360.f, UVs.m_mins.x, UVs.m_maxs.x);

		//bottom pie
		verts.emplace_back(base, color, centerUV);
		verts.emplace_back(secondBottomTip, color, Vec2(discU, bottomV));
		verts.emplace_back(firstBottomTip, color, Vec2(discPrevU, bottomPrevV));

		//side panel
		verts.emplace_back(firstBottomTip, color, Vec2(sidePrevU, UVs.m_mins.y));
		verts.emplace_back(secondBottomTip, color, Vec2(sideU, UVs.m_mins.y));
		verts.emplace_back(tip, color, Vec2(sideU, UVs.m_maxs.y));

		firstBottomTip = secondBottomTip;
		discPrevU = discU;
		bottomPrevV = bottomV;
		sidePrevU = sideU;
	}
}


void AddVertsForWireCone3D(std::vector<Vertex_PCU>& verts, Vec3 const& base, Vec3 const& tip, float radius, float slices, float wireWidth, Rgba8 const& color)
{
	float wireHalfWidth = 0.5f * wireWidth;
	Vec3 kBasis = (tip - base).GetNormalized();
	Vec3 iBasis, jBasis;
	if (fabsf(DotProduct3D(kBasis, Vec3(1.f, 0.f, 0.f))) < .99f)
	{
		jBasis = CrossProduct3D(kBasis, Vec3(1.f, 0.f, 0.f)).GetNormalized();
		iBasis = CrossProduct3D(jBasis, kBasis).GetNormalized();
	}
	else
	{
		jBasis = CrossProduct3D(kBasis, Vec3(0.f, 1.f, 0.f)).GetNormalized();
		iBasis = CrossProduct3D(jBasis, kBasis).GetNormalized();
	}

	iBasis *= radius;
	jBasis *= radius;

	float degreesPerSlice = 360.f / slices;
	Vec3 firstBottomTip = base + iBasis * CosDegrees(0.f) + jBasis * SinDegrees(0.f);

	for (float curDegrees = degreesPerSlice; curDegrees <= 360.1f; curDegrees += degreesPerSlice)
	{
		Vec3 secondBottomTip = base + iBasis * CosDegrees(curDegrees) + jBasis * SinDegrees(curDegrees);

		//side panel
		AddVertsForLine3D(verts, firstBottomTip, secondBottomTip, wireHalfWidth, color);
		AddVertsForLine3D(verts, secondBottomTip, tip, wireHalfWidth, color);
		AddVertsForLine3D(verts, tip, firstBottomTip, wireHalfWidth, color);

		firstBottomTip = secondBottomTip;
	}
}


void AddVertsForArrow3D(std::vector<Vertex_PCU>& verts, Vec3 const& tailPos, Vec3 const& tipPos, float radius, Rgba8 const& color, AABB2 const& UVs)
{
	Vec3 forwardNormal = (tipPos - tailPos).GetNormalized();
	float arrowLength = radius * 4.5f;
	float arrowRadius = radius * 2.f;
	Vec3 arrowBase = tipPos - forwardNormal * arrowLength;
	AddVertsForCone3D(verts, arrowBase, tipPos, arrowRadius, 8.f, color, UVs);
	AddVertsForCylinder3D(verts, tailPos, arrowBase, radius, 8.f, color, UVs);
}


void AddVertsForBasis3D(std::vector<Vertex_PCU>& verts, float radius)
{
	AddVertsForArrow3D(verts, Vec3::ZERO, Vec3(1.f, 0.f, 0.f), radius, Rgba8::RED);
	AddVertsForArrow3D(verts, Vec3::ZERO, Vec3(0.f, 1.f, 0.f), radius, Rgba8::GREEN);
	AddVertsForArrow3D(verts, Vec3::ZERO, Vec3(0.f, 0.f, 1.f), radius, Rgba8::BLUE);
}


void AddVertsForScreenBasis3D(std::vector<Vertex_PCU>& verts, Vec3 const& center, float length, float width)
{
	AddVertsForLine3D(verts, center, center + Vec3(1.f, 0.f, 0.f) * length, width, Rgba8::RED);
	AddVertsForLine3D(verts, center, center + Vec3(0.f, 1.f, 0.f) * length, width, Rgba8::GREEN);
	AddVertsForLine3D(verts, center, center + Vec3(0.f, 0.f, 1.f) * length, width, Rgba8::BLUE);
}


void AddIndexedVertsForQuad3D(std::vector<Vertex_PCU>& verts, std::vector<unsigned int>& indices, Vec3 const& bottomLeft, Vec3 const& bottomRight, Vec3 const& topRight, Vec3 const& topLeft, Rgba8 const& color, AABB2 const& UVs)
{
	int firstIndex = (int)verts.size();

	verts.emplace_back(bottomLeft, color, UVs.m_mins);
	verts.emplace_back(bottomRight, color, Vec2(UVs.m_maxs.x, UVs.m_mins.y));
	verts.emplace_back(topRight, color, UVs.m_maxs);
	verts.emplace_back(topLeft, color, Vec2(UVs.m_mins.x, UVs.m_maxs.y));
	
	indices.push_back(firstIndex);
	indices.push_back(firstIndex + 1);
	indices.push_back(firstIndex + 2);
	indices.push_back(firstIndex);
	indices.push_back(firstIndex + 2);
	indices.push_back(firstIndex + 3);
}


void AddIndexedVertsForAABB3D(std::vector<Vertex_PCU>& verts, std::vector<unsigned int>& indices, AABB3 const& bounds, Rgba8 const& color, AABB2 const& UVs)
{
	Vec3 corners[8] = {};
	bounds.GetCorners(corners);
	Vec3 nearBL = corners[0];
	Vec3 nearBR = corners[1];
	Vec3 nearTR = corners[2];
	Vec3 nearTL = corners[3];
	Vec3 farBL = corners[4];
	Vec3 farBR = corners[5];
	Vec3 farTR = corners[6];
	Vec3 farTL = corners[7];

	// x forward, y left, z up
	AddIndexedVertsForQuad3D(verts, indices, farBR, farBL, farTL, farTR, color, UVs); //back quad +y
	AddIndexedVertsForQuad3D(verts, indices, nearBL, nearBR, nearTR, nearTL, color, UVs); //front quad -y
	AddIndexedVertsForQuad3D(verts, indices, farBL, nearBL, nearTL, farTL, color, UVs); //left quad -x
	AddIndexedVertsForQuad3D(verts, indices, nearBR, farBR, farTR, nearTR, color, UVs); //right quad +x
	AddIndexedVertsForQuad3D(verts, indices, nearTL, nearTR, farTR, farTL, color, UVs); //top quad +z
	AddIndexedVertsForQuad3D(verts, indices, farBL, farBR, nearBR, nearBL, color, UVs); //bottom quad -z
}


void AddVertsForRoundedQuad3D(std::vector<Vertex_PNCU>& verts, Vec3 const& bottomLeft, Vec3 const& bottomRight, Vec3 const& topRight, Vec3 const& topLeft, Rgba8 const& color, AABB2 const& UVs)
{
	Vec3 bottomCenter = 0.5f * (bottomLeft + bottomRight);
	Vec3 topCenter = 0.5f * (topLeft + topRight);
	Vec2 uvBottomCenter(0.5f * (UVs.m_mins.x + UVs.m_maxs.x), UVs.m_mins.y);
	Vec2 uvTopCenter(0.5f * (UVs.m_mins.x + UVs.m_maxs.x), UVs.m_maxs.y);

	Vec3 hortizontalNormal = bottomRight - bottomLeft;
	Vec3 verticalNormal = topLeft - bottomLeft;
	Vec3 outNormal = CrossProduct3D(hortizontalNormal, verticalNormal).GetNormalized();
	hortizontalNormal = hortizontalNormal.GetNormalized();

	verts.emplace_back(bottomLeft, -hortizontalNormal, color, UVs.m_mins);
	verts.emplace_back(bottomCenter, outNormal, color, uvBottomCenter);
	verts.emplace_back(topCenter, outNormal, color, uvTopCenter);

	verts.emplace_back(bottomLeft, -hortizontalNormal, color, UVs.m_mins);
	verts.emplace_back(topCenter, outNormal, color, uvTopCenter);
	verts.emplace_back(topLeft, -hortizontalNormal, color, Vec2(UVs.m_mins.x, UVs.m_maxs.y));

	verts.emplace_back(bottomCenter, outNormal, color, uvBottomCenter);
	verts.emplace_back(bottomRight, hortizontalNormal, color, Vec2(UVs.m_maxs.x, UVs.m_mins.y));
	verts.emplace_back(topRight, hortizontalNormal, color, UVs.m_maxs);

	verts.emplace_back(bottomCenter, outNormal, color, uvBottomCenter);
	verts.emplace_back(topRight, hortizontalNormal, color, UVs.m_maxs);
	verts.emplace_back(topCenter, outNormal, color, uvTopCenter);
}


void AddIndexedVertsForQuad3D(std::vector<Vertex_PNCU>& verts, std::vector<unsigned int>& indices, Vec3 const& bottomLeft, Vec3 const& bottomRight, Vec3 const& topRight, Vec3 const& topLeft, Rgba8 const& color, AABB2 const& UVs)
{
	Vec3 hortizontalNormal = bottomRight - bottomLeft;
	Vec3 verticalNormal = topLeft - bottomLeft;
	Vec3 outNormal = CrossProduct3D(hortizontalNormal, verticalNormal).GetNormalized();

	int firstIndex = (int)verts.size();

	verts.emplace_back(bottomLeft, outNormal, color, UVs.m_mins);
	verts.emplace_back(bottomRight, outNormal, color, Vec2(UVs.m_maxs.x, UVs.m_mins.y));
	verts.emplace_back(topRight, outNormal, color, UVs.m_maxs);
	verts.emplace_back(topLeft, outNormal, color, Vec2(UVs.m_mins.x, UVs.m_maxs.y));

	indices.push_back(firstIndex);
	indices.push_back(firstIndex + 1);
	indices.push_back(firstIndex + 2);
	indices.push_back(firstIndex);
	indices.push_back(firstIndex + 2);
	indices.push_back(firstIndex + 3);
}


void AddVertsForRectBorderUI(std::vector<Vertex_PCU>& verts, AABB2 const& box, float thickness, Rgba8 const& color , AABB2 const& UVs)
{
	Vec2 bottomLeft = box.m_mins;
	Vec2 bottomRight(box.m_maxs.x, box.m_mins.y);
	Vec2 topRight = box.m_maxs;
	Vec2 topLeft(box.m_mins.x, box.m_maxs.y);

	AddVertsForLineSegment2D(verts, LineSegment2(bottomLeft, bottomRight), thickness, color, UVs);
	AddVertsForLineSegment2D(verts, LineSegment2(bottomRight, topRight), thickness, color, UVs);
	AddVertsForLineSegment2D(verts, LineSegment2(topRight, topLeft), thickness, color, UVs);
	AddVertsForLineSegment2D(verts, LineSegment2(topLeft, bottomLeft), thickness, color, UVs);
}


void AddVertsForDoubleRectBorderUI(std::vector<Vertex_PCU>& verts, AABB2 const& box, float thickness, float spacing, Rgba8 const& color, AABB2 const& UVs)
{
	Vec2 bottomLeft = box.m_mins;
	Vec2 bottomRight(box.m_maxs.x, box.m_mins.y);
	Vec2 topRight = box.m_maxs;
	Vec2 topLeft(box.m_mins.x, box.m_maxs.y);

	AddVertsForLineSegment2D(verts, LineSegment2(bottomLeft, bottomRight), thickness, color, UVs);
	AddVertsForLineSegment2D(verts, LineSegment2(bottomRight, topRight), thickness, color, UVs);
	AddVertsForLineSegment2D(verts, LineSegment2(topRight, topLeft), thickness, color, UVs);
	AddVertsForLineSegment2D(verts, LineSegment2(topLeft, bottomLeft), thickness, color, UVs);

	Vec2 outterBottomLeft = bottomLeft + Vec2(-spacing, -spacing);
	Vec2 outterBottomRight = bottomRight + Vec2(spacing, -spacing);
	Vec2 outterTopRight = topRight + Vec2(spacing, spacing);
	Vec2 outterTopLeft = topLeft + Vec2(-spacing, spacing);

	AddVertsForLineSegment2D(verts, LineSegment2(outterBottomLeft, outterBottomRight), thickness, color, UVs);
	AddVertsForLineSegment2D(verts, LineSegment2(outterBottomRight, outterTopRight), thickness, color, UVs);
	AddVertsForLineSegment2D(verts, LineSegment2(outterTopRight, outterTopLeft), thickness, color, UVs);
	AddVertsForLineSegment2D(verts, LineSegment2(outterTopLeft, outterBottomLeft), thickness, color, UVs);
}


void AddVertsForCurveUI(std::vector<Vertex_PCU>& verts, Vec2 const& center, float radius, float thickness, float startingDegrees, float slices, Rgba8 const& color, AABB2 const& UVs)
{
	float pieDegrees = 90.f / slices;
	float halfWdith = 0.5f * thickness;
	for (float curDegress = startingDegrees; curDegress < startingDegrees + 89.f; curDegress += pieDegrees)
	{
		float lesserDegrees = curDegress;
		float moreDegrees = curDegress + pieDegrees;
		Vec3 closerLess = Vec3(center + Vec2((radius - halfWdith) * CosDegrees(lesserDegrees),
			(radius - halfWdith) * SinDegrees(lesserDegrees)));
		Vec3 farLess = Vec3(center + Vec2((radius + halfWdith) * CosDegrees(lesserDegrees),
			(radius + halfWdith) * SinDegrees(lesserDegrees)));
		Vec3 closerMore = Vec3(center + Vec2((radius - halfWdith) * CosDegrees(moreDegrees),
			(radius - halfWdith) * SinDegrees(moreDegrees)));
		Vec3 farMore = Vec3(center + Vec2((radius + halfWdith) * CosDegrees(moreDegrees),
			(radius + halfWdith) * SinDegrees(moreDegrees)));

		verts.emplace_back(closerMore, color, UVs.m_mins);
		verts.emplace_back(closerLess, color, Vec2(UVs.m_maxs.x, UVs.m_mins.y));
		verts.emplace_back(farLess, color, UVs.m_maxs);
		verts.emplace_back(closerMore, color, UVs.m_mins);
		verts.emplace_back(farLess, color, UVs.m_maxs);
		verts.emplace_back(farMore, color, Vec2(UVs.m_mins.x, UVs.m_maxs.y));
	}
}


void AddVertsForRoundedBorderUI(std::vector<Vertex_PCU>& verts, AABB2 const& box, float thickness, float radius, Rgba8 const& color, AABB2 const& UVs)
{
	Vec2 bottomLeft = box.m_mins;
	Vec2 bottomRight(box.m_maxs.x, box.m_mins.y);
	Vec2 topRight = box.m_maxs;
	Vec2 topLeft(box.m_mins.x, box.m_maxs.y);

	AddVertsForLineSegment2D(verts, LineSegment2(bottomLeft + Vec2(radius, 0.f), bottomRight + Vec2(-radius, 0.f)), thickness, color, UVs);
	AddVertsForLineSegment2D(verts, LineSegment2(bottomRight + Vec2(0.f, radius), topRight + Vec2(0.f, -radius)), thickness, color, UVs);
	AddVertsForLineSegment2D(verts, LineSegment2(topRight + Vec2(-radius, 0.f), topLeft + Vec2(radius, 0.f)), thickness, color, UVs);
	AddVertsForLineSegment2D(verts, LineSegment2(topLeft + Vec2(0.f, -radius), bottomLeft + Vec2(0.f, radius)), thickness, color, UVs);

	Vec2 innerBottomLeft = bottomLeft + Vec2(radius, radius);
	Vec2 innerBottomRight = bottomRight + Vec2(-radius, radius);
	Vec2 innerTopRight = topRight + Vec2(-radius, -radius);
	Vec2 innerTopLeft = topLeft + Vec2(radius, - radius);

	AddVertsForCurveUI(verts, innerBottomLeft, radius, thickness, 180.f, 16.f, color, UVs);
	AddVertsForCurveUI(verts, innerBottomRight, radius, thickness, 270.f, 16.f, color, UVs);
	AddVertsForCurveUI(verts, innerTopRight, radius, thickness, 0.f, 16.f, color, UVs);
	AddVertsForCurveUI(verts, innerTopLeft, radius, thickness, 90.f, 16.f, color, UVs);
}


void AddVertsForDoubleRoundedBorderUI(std::vector<Vertex_PCU>& verts, AABB2 const& box, float thickness, float radius, float spacing, Rgba8 const& color, AABB2 const& UVs)
{
	AddVertsForRoundedBorderUI(verts, box, thickness, radius, color, UVs);

	AABB2 outterBox(box.m_mins + Vec2(-spacing, -spacing), box.m_maxs + Vec2(spacing, spacing));
	AddVertsForRoundedBorderUI(verts, outterBox, thickness, radius + spacing, color, UVs);
}


