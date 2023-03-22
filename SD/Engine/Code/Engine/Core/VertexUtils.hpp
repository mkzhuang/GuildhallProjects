#pragma once
#include "Engine/Core/Vertex_PCU.hpp"
#include "Engine/Core/Vertex_PNCU.hpp"
#include "Engine/Math/Vec2.hpp"
#include "Engine/Math/AABB2.hpp"
#include "Engine/Math/AABB3.hpp"
#include "Engine/Math/OBB2.hpp"
#include "Engine/Math/ConvexPoly2D.hpp"
#include "Engine/Math/OBB3.hpp"
#include "Engine/Math/Capsule2.hpp"
#include "Engine/Math/Capsule3.hpp"
#include "Engine/Math/LineSegment2.hpp"
#include "Engine/Core/Rgba8.hpp"

#include <vector>

void TransformVertexArrayXY3D(int numberVerts, Vertex_PCU* verts, float uniformScaleXY, float rotationDegreesAboutZ, Vec2 const& translationXY);
void TransformVertexArrayXYZ3D(int numberVerts, Vertex_PCU* verts, Vec3 const& iBasis, Vec3 const& jBasis, Vec3 const& kBasis, Vec3 const& translationXYZ);

void AddVertsForCapsule2D(std::vector<Vertex_PCU>& verts, Capsule2 const& capsule, Rgba8 const& color = Rgba8::WHITE, AABB2 const& UVs = AABB2::ZERO_TO_ONE);
void AddVertsForDiscs2D(std::vector<Vertex_PCU>& verts, Vec2 const& center, float radius, Rgba8 const& color = Rgba8::WHITE, AABB2 const& UVs = AABB2::ZERO_TO_ONE);
void AddVertsForAABB2D(std::vector<Vertex_PCU>& verts, AABB2 const& bounds, Rgba8 const& color = Rgba8::WHITE, Vec2 const& uvMins = Vec2::ZERO, Vec2 const& uvMaxs = Vec2::ONE);
void AddVertsForHollowAABB2D(std::vector<Vertex_PCU>& verts, AABB2 const& bounds, float thickness, Rgba8 const& color = Rgba8::WHITE);
void AddVertsForAABB2D(std::vector<Vertex_PCU>& verts, AABB2 const& bounds, Rgba8 const& color, AABB2 const& UVs);
void AddVertsForOBB2D(std::vector<Vertex_PCU>& verts, OBB2 const& box, Rgba8 const& color = Rgba8::WHITE, Vec2 const& uvMins = Vec2::ZERO, Vec2 const& uvMaxs = Vec2::ONE);
//void AddVertsForOBB2D(std::vector<Vertex_PCU>& verts, OBB2 const& box, Rgba8 const& color = Rgba8::WHITE, AABB2 const& UVs = AABB2::ZERO_TO_ONE);
void AddVertsForConvex2D(std::vector<Vertex_PCU>& verts, ConvexPoly2D const& convex, Rgba8 const& color = Rgba8::WHITE, AABB2 const& UVs = AABB2::ZERO_TO_ONE);
void AddVertsForConvexOutline2D(std::vector<Vertex_PCU>& verts, ConvexPoly2D const& convex, float thickness, Rgba8 const& color = Rgba8::WHITE, AABB2 const& UVs = AABB2::ZERO_TO_ONE);
void AddVertsForLineSegment2D(std::vector<Vertex_PCU>& verts, LineSegment2 const& lineSegment, float thickness, Rgba8 const& color = Rgba8::WHITE, AABB2 const& UVs = AABB2::ZERO_TO_ONE);
void AddVertsForRing2D(std::vector<Vertex_PCU>& verts, Vec2 const& center, float radius, float thickness, float slices, Rgba8 const& color = Rgba8::WHITE, AABB2 const& UVs = AABB2::ZERO_TO_ONE);
void AddVertsForOrientedSector2D(std::vector<Vertex_PCU>& verts, Vec2 const& sectorTip, float sectorForwardDegrees, float sectorApertureDegrees, float sectorRadius, Rgba8 const& color = Rgba8::WHITE, AABB2 const& UVs = AABB2::ZERO_TO_ONE);
void AddVertsForArrow2D(std::vector<Vertex_PCU>& verts, Vec2 const& tailPos, Vec2 const& tipPos, float arrowSize, float lineThickness, Rgba8 const& color = Rgba8::WHITE);
void AddVertsForQuad3D(std::vector<Vertex_PCU>& verts, Vec3 const& bottomLeft, Vec3 const& bottomRight, Vec3 const& topRight, Vec3 const& topLeft, Rgba8 const& color = Rgba8::WHITE, AABB2 const& UVs = AABB2::ZERO_TO_ONE);
void AddVertsForAABB3D(std::vector<Vertex_PCU>& verts, AABB3 const& bounds, Rgba8 const& color = Rgba8::WHITE, AABB2 const& UVs = AABB2::ZERO_TO_ONE);
void AddVertsForInvertedAABB3D(std::vector<Vertex_PCU>& verts, AABB3 const& bounds, Rgba8 const& color = Rgba8::WHITE, AABB2 const& UVs = AABB2::ZERO_TO_ONE);
void AddVertsForOBB3D(std::vector<Vertex_PCU>& verts, OBB3 const& bounds, Rgba8 const& color = Rgba8::WHITE, AABB2 const& UVs = AABB2::ZERO_TO_ONE);
void AddVertsForCapsule3D(std::vector<Vertex_PCU>& verts, Capsule3 const& capsule, float longitudeSlices, float latitudeSlices, Rgba8 const& color = Rgba8::WHITE, AABB2 const& UVs = AABB2::ZERO_TO_ONE);
void AddVertsForWireAABB3D(std::vector<Vertex_PCU>& verts, AABB3 const& bounds, float wireWidth, Rgba8 const& color = Rgba8::WHITE, AABB2 const& UVs = AABB2::ZERO_TO_ONE);
void AddVertsForLine3D(std::vector<Vertex_PCU>& verts, Vec3 const& start, Vec3 const& end, float width, Rgba8 const& color = Rgba8::WHITE, AABB2 const& UVs = AABB2::ZERO_TO_ONE);
void AddVertsForSphere3D(std::vector<Vertex_PCU>& verts, Vec3 const& center, float radius, float longitudeSlices, float latitudeSlices, Rgba8 const& color = Rgba8::WHITE, AABB2 const& UVs = AABB2::ZERO_TO_ONE);
void AddVertsForWireSphere3D(std::vector<Vertex_PCU>& verts, Vec3 const& center, float radius, float longitudeSlices, float latitudeSlices, float wireWidth, Rgba8 const& color = Rgba8::WHITE);
void AddVertsForCylinder3D(std::vector<Vertex_PCU>& verts, Vec3 const& start, Vec3 const& end, float radius, float slices, Rgba8 const& color = Rgba8::WHITE,  AABB2 const& UVs = AABB2::ZERO_TO_ONE);
void AddVertsForWireCylinder3D(std::vector<Vertex_PCU>& verts, Vec3 const& start, Vec3 const& end, float radius, float slices, float wireWidth, Rgba8 const& color = Rgba8::WHITE);
void AddVertsForCone3D(std::vector<Vertex_PCU>& verts, Vec3 const& base, Vec3 const& tip, float radius, float slices, Rgba8 const& color = Rgba8::WHITE,  AABB2 const& UVs = AABB2::ZERO_TO_ONE);
void AddVertsForWireCone3D(std::vector<Vertex_PCU>& verts, Vec3 const& base, Vec3 const& tip, float radius, float slices, float wireWidth, Rgba8 const& color = Rgba8::WHITE);
void AddVertsForArrow3D(std::vector<Vertex_PCU>& verts, Vec3 const& tailPos, Vec3 const& tipPos, float radius, Rgba8 const& color = Rgba8::WHITE,  AABB2 const& UVs = AABB2::ZERO_TO_ONE);
void AddVertsForBasis3D(std::vector<Vertex_PCU>& verts, float radius);
void AddVertsForScreenBasis3D(std::vector<Vertex_PCU>& verts, Vec3 const& center, float length, float wdith);

void AddIndexedVertsForQuad3D(std::vector<Vertex_PCU>& verts, std::vector<unsigned int>& indices, Vec3 const& bottomLeft, Vec3 const& bottomRight, Vec3 const& topRight, Vec3 const& topLeft, Rgba8 const& color = Rgba8::WHITE, AABB2 const& UVs = AABB2::ZERO_TO_ONE);
void AddIndexedVertsForAABB3D(std::vector<Vertex_PCU>& verts, std::vector<unsigned int>& indices, AABB3 const& bounds, Rgba8 const& color = Rgba8::WHITE, AABB2 const& UVs = AABB2::ZERO_TO_ONE);

void AddVertsForRoundedQuad3D(std::vector<Vertex_PNCU>& verts, Vec3 const& bottomLeft, Vec3 const& bottomRight, Vec3 const& topRight, Vec3 const& topLeft, Rgba8 const& color = Rgba8::WHITE, AABB2 const& UVs = AABB2::ZERO_TO_ONE);
void AddIndexedVertsForQuad3D(std::vector<Vertex_PNCU>& verts, std::vector<unsigned int>& indices, Vec3 const& bottomLeft, Vec3 const& bottomRight, Vec3 const& topRight, Vec3 const& topLeft, Rgba8 const& color = Rgba8::WHITE, AABB2 const& UVs = AABB2::ZERO_TO_ONE);

void AddVertsForRectBorderUI(std::vector<Vertex_PCU>& verts, AABB2 const& box, float thickness, Rgba8 const& color = Rgba8::WHITE, AABB2 const& UVs = AABB2::ZERO_TO_ONE);
void AddVertsForDoubleRectBorderUI(std::vector<Vertex_PCU>& verts, AABB2 const& box, float thickness, float spacing = 1.f, Rgba8 const& color = Rgba8::WHITE, AABB2 const& UVs = AABB2::ZERO_TO_ONE);
void AddVertsForCurveUI(std::vector<Vertex_PCU>& verts, Vec2 const& center, float radius, float thickness, float startingDegrees, float slices = 16.f, Rgba8 const& color = Rgba8::WHITE, AABB2 const& UVs = AABB2::ZERO_TO_ONE);
void AddVertsForRoundedBorderUI(std::vector<Vertex_PCU>& verts, AABB2 const& box, float thickness, float radius, Rgba8 const& color = Rgba8::WHITE, AABB2 const& UVs = AABB2::ZERO_TO_ONE);
void AddVertsForDoubleRoundedBorderUI(std::vector<Vertex_PCU>& verts, AABB2 const& box, float thickness, float radius, float spacing = 1.f, Rgba8 const& color = Rgba8::WHITE, AABB2 const& UVs = AABB2::ZERO_TO_ONE);

