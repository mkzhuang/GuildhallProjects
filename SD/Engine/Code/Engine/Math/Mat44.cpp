#include "Engine//Math/Mat44.hpp"
#include "Engine/Math/Vec2.hpp"
#include "Engine/Math/Vec3.hpp"
#include "Engine/Math/Vec4.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Math/EulerAngles.hpp"

#define UNUSED(x) (void)(x);

Mat44::Mat44()
{
	m_values[Ix] = 1.f;
	m_values[Iy] = 0.f;
	m_values[Iz] = 0.f;
	m_values[Iw] = 0.f;

	m_values[Jx] = 0.f;
	m_values[Jy] = 1.f;
	m_values[Jz] = 0.f;
	m_values[Jw] = 0.f;

	m_values[Kx] = 0.f;
	m_values[Ky] = 0.f;
	m_values[Kz] = 1.f;
	m_values[Kw] = 0.f;

	m_values[Tx] = 0.f;
	m_values[Ty] = 0.f;
	m_values[Tz] = 0.f;
	m_values[Tw] = 1.f;
}


Mat44::Mat44(Vec2 const& iBasis2D, Vec2 const& jBasis2D, Vec2 const& translation2D)
{
	m_values[Ix] = iBasis2D.x;
	m_values[Iy] = iBasis2D.y;
	m_values[Iz] = 0.f;
	m_values[Iw] = 0.f;

	m_values[Jx] = jBasis2D.x;
	m_values[Jy] = jBasis2D.y;
	m_values[Jz] = 0.f;
	m_values[Jw] = 0.f;

	m_values[Kx] = 0.f;
	m_values[Ky] = 0.f;
	m_values[Kz] = 1.f;
	m_values[Kw] = 0.f;

	m_values[Tx] = translation2D.x;
	m_values[Ty] = translation2D.y;
	m_values[Tz] = 0.f;
	m_values[Tw] = 1.f;
}


Mat44::Mat44(Vec3 const& iBasis3D, Vec3 const& jBasis3D, Vec3 const& kBasis3D, Vec3 const& translation3D)
{
	m_values[Ix] = iBasis3D.x;
	m_values[Iy] = iBasis3D.y;
	m_values[Iz] = iBasis3D.z;
	m_values[Iw] = 0.f;

	m_values[Jx] = jBasis3D.x;
	m_values[Jy] = jBasis3D.y;
	m_values[Jz] = jBasis3D.z;
	m_values[Jw] = 0.f;

	m_values[Kx] = kBasis3D.x;
	m_values[Ky] = kBasis3D.y;
	m_values[Kz] = kBasis3D.z;
	m_values[Kw] = 0.f;

	m_values[Tx] = translation3D.x;
	m_values[Ty] = translation3D.y;
	m_values[Tz] = translation3D.z;
	m_values[Tw] = 1.f;
}


Mat44::Mat44(Vec4 const& iBasis4D, Vec4 const& jBasis4D, Vec4 const& kBasis4D, Vec4 const& translation4D)
{
	m_values[Ix] = iBasis4D.x;
	m_values[Iy] = iBasis4D.y;
	m_values[Iz] = iBasis4D.z;
	m_values[Iw] = iBasis4D.w;

	m_values[Jx] = jBasis4D.x;
	m_values[Jy] = jBasis4D.y;
	m_values[Jz] = jBasis4D.z;
	m_values[Jw] = jBasis4D.w;

	m_values[Kx] = kBasis4D.x;
	m_values[Ky] = kBasis4D.y;
	m_values[Kz] = kBasis4D.z;
	m_values[Kw] = kBasis4D.w;

	m_values[Tx] = translation4D.x;
	m_values[Ty] = translation4D.y;
	m_values[Tz] = translation4D.z;
	m_values[Tw] = translation4D.w;
}


Mat44::Mat44(float const* sixteenValuesBasisMajor)
{
	m_values[Ix] = sixteenValuesBasisMajor[Ix];
	m_values[Iy] = sixteenValuesBasisMajor[Iy];
	m_values[Iz] = sixteenValuesBasisMajor[Iz];
	m_values[Iw] = sixteenValuesBasisMajor[Iw];

	m_values[Jx] = sixteenValuesBasisMajor[Jx];
	m_values[Jy] = sixteenValuesBasisMajor[Jy];
	m_values[Jz] = sixteenValuesBasisMajor[Jz];
	m_values[Jw] = sixteenValuesBasisMajor[Jw];

	m_values[Kx] = sixteenValuesBasisMajor[Kx];
	m_values[Ky] = sixteenValuesBasisMajor[Ky];
	m_values[Kz] = sixteenValuesBasisMajor[Kz];
	m_values[Kw] = sixteenValuesBasisMajor[Kw];

	m_values[Tx] = sixteenValuesBasisMajor[Tx];
	m_values[Ty] = sixteenValuesBasisMajor[Ty];
	m_values[Tz] = sixteenValuesBasisMajor[Tz];
	m_values[Tw] = sixteenValuesBasisMajor[Tw];
}


Mat44 const Mat44::CreateTranslation2D(Vec2 const& translationXY)
{
	return Mat44(Vec2(1.f, 0.f), Vec2(0.f, 1.f), translationXY);
}


Mat44 const Mat44::CreateTranslation3D(Vec3 const& translationXYZ)
{
	return Mat44(Vec3(1.f, 0.f, 0.f), Vec3(0.f, 1.f, 0.f), Vec3(0.f, 0.f, 1.f), translationXYZ);
}


Mat44 const Mat44::CreateUniformScale2D(float uniformScaleXY)
{
	return Mat44(Vec2(uniformScaleXY, 0.f), Vec2(0.f, uniformScaleXY), Vec2::ZERO);
}


Mat44 const Mat44::CreateUniformScale3D(float uniformScaleXYZ)
{
	return Mat44(Vec3(uniformScaleXYZ, 0.f, 0.f), Vec3(0.f, uniformScaleXYZ, 0.f), Vec3(0.f, 0.f, uniformScaleXYZ), Vec3::ZERO);
}

Mat44 const Mat44::CreateNonUniformScale2D(Vec2 const& nonUniformScaleXY)
{
	return Mat44(Vec2(nonUniformScaleXY.x, 0.f), Vec2(0.f, nonUniformScaleXY.y), Vec2::ZERO);
}


Mat44 const Mat44::CreateNonUniformScale3D(Vec3 const& nonUniformScaleXYZ)
{
	return Mat44(Vec3(nonUniformScaleXYZ.x, 0.f, 0.f), Vec3(0.f, nonUniformScaleXYZ.y, 0.f), Vec3(0.f, 0.f, nonUniformScaleXYZ.z), Vec3::ZERO);
}


Mat44 const Mat44::CreateZRotationDegrees(float rotationDegreesAboutZ)
{
	float cosTheta = CosDegrees(rotationDegreesAboutZ);
	float sinTheta = SinDegrees(rotationDegreesAboutZ);
	return Mat44(Vec3(cosTheta, sinTheta, 0.f), Vec3(-sinTheta, cosTheta, 0.f), Vec3(0.f, 0.f, 1.f), Vec3::ZERO);
}


Mat44 const Mat44::CreateYRotationDegrees(float rotationDegreesAboutY)
{
	float cosTheta = CosDegrees(rotationDegreesAboutY);
	float sinTheta = SinDegrees(rotationDegreesAboutY);
	return Mat44(Vec3(cosTheta, 0.f, -sinTheta), Vec3(0.f, 1.f, 0.f), Vec3(sinTheta, 0.f, cosTheta), Vec3::ZERO);
}


Mat44 const Mat44::CreateXRotationDegrees(float rotationDegreesAboutX)
{
	float cosTheta = CosDegrees(rotationDegreesAboutX);
	float sinTheta = SinDegrees(rotationDegreesAboutX);
	return Mat44(Vec3(1.f, 0.f, 0.f), Vec3(0.f, cosTheta, sinTheta), Vec3(0.f, -sinTheta, cosTheta), Vec3::ZERO);
}


Mat44 const Mat44::CreateOrthoProjection(float left, float right, float bottom, float top, float near, float far)
{
	float values[16] = {};
	float iNormalRange = 1.f / (right - left);
	float jNormalRange = 1.f / (top - bottom);
	float kNormalRange = 1.f / (far - near);
	values[Ix] = 2.f * iNormalRange;
	values[Jy] = 2.f * jNormalRange;
	values[Kz] = kNormalRange;
	values[Tx] = (right + left) * -iNormalRange;
	values[Ty] = (top + bottom) * -jNormalRange;
	values[Tz] = (near) * -kNormalRange;
	values[Tw] = 1.f;
	
	return Mat44(values);
}


Mat44 const Mat44::CreatePerspectiveProjection(float fovYDegrees, float aspect, float zNear, float zFar)
{
	float values[16] = {};
	float scale = CosDegrees(.5f * fovYDegrees) / SinDegrees(.5f * fovYDegrees);
	float zNormalRange = 1.f / (zFar - zNear);
	values[Ix] = scale / aspect;
	values[Jy] = scale;
	values[Kz] = zFar * zNormalRange;
	values[Kw] = 1.f;
	values[Tz] = (zNear * zFar) * -zNormalRange;

	return Mat44(values);
}


Mat44 const Mat44::CreateRotationFromQuaternions(Vec4 const& q)
{
	float sqx	= q.x * q.x;
	float sqy	= q.y * q.y;
	float sqz	= q.z * q.z;
	float xy	= q.x * q.y;
	float xz	= q.x * q.z;
	float yz	= q.y * q.z;
	float xw	= q.x * q.w;
	float yw	= q.y * q.w;
	float zw	= q.z * q.w;

	float values[16] = {};
	values[Ix] = 1.f - 2.f * (sqy + sqz);
	values[Iy] = 2.f * (xy - zw);
	values[Iz] = 2.f * (xz + yw);

	values[Jx] = 2.f * (xy + zw);
	values[Jy] = 1.f - 2.f * (sqx + sqz);
	values[Jz] = 2.f * (yz - xw);

	values[Kx] = 2.f * (xz - yw);
	values[Ky] = 2.f * (yz + xw);
	values[Kz] = 1.f - 2.f * (sqx + sqy);

	values[Tz] = 1.f;

	Mat44 matrix(values);
	matrix.Orthonormalize_XFwd_YLeft_ZUp();
	return matrix;
}


Mat44 const Mat44::CreateRotationFromAngleAxis(Vec3 const& a)
{
	Vec3 normal = a.GetNormalized();
	float rotation = a.GetLength();

	float c = cosf(rotation);
	float s = sinf(rotation);
	float cn = 1.f - c;

	float xx = normal.x * normal.x;
	float yy = normal.y * normal.y;
	float zz = normal.z * normal.z;

	float xy = normal.x * normal.y;
	float yz = normal.y * normal.z;
	float xz = normal.x * normal.z;

	float xs = normal.x * s;
	float ys = normal.y * s;
	float zs = normal.z * s;

	Vec3 iBasis(xx * cn + c, xy * cn + zs, xz * cn - ys);
	Vec3 jBasis(xy * cn - zs, yy * cn + c, yz * cn + xs);
	Vec3 kBasis(xz * cn + ys, yz * cn - xs, zz * cn + c);

	return Mat44(iBasis, jBasis, kBasis, Vec3::ZERO);
}


Mat44 const Mat44::GetInverse(Mat44 const& mat)
{
	float inv[16];
	float det;
	float m[16];
	uint32_t i;
	for (i = 0; i < 16; ++i) {
		m[i] = mat.m_values[i];
	}

	inv[0] = m[5] * m[10] * m[15] -
		m[5] * m[11] * m[14] -
		m[9] * m[6] * m[15] +
		m[9] * m[7] * m[14] +
		m[13] * m[6] * m[11] -
		m[13] * m[7] * m[10];

	inv[4] = -m[4] * m[10] * m[15] +
		m[4] * m[11] * m[14] +
		m[8] * m[6] * m[15] -
		m[8] * m[7] * m[14] -
		m[12] * m[6] * m[11] +
		m[12] * m[7] * m[10];

	inv[8] = m[4] * m[9] * m[15] -
		m[4] * m[11] * m[13] -
		m[8] * m[5] * m[15] +
		m[8] * m[7] * m[13] +
		m[12] * m[5] * m[11] -
		m[12] * m[7] * m[9];

	inv[12] = -m[4] * m[9] * m[14] +
		m[4] * m[10] * m[13] +
		m[8] * m[5] * m[14] -
		m[8] * m[6] * m[13] -
		m[12] * m[5] * m[10] +
		m[12] * m[6] * m[9];

	inv[1] = -m[1] * m[10] * m[15] +
		m[1] * m[11] * m[14] +
		m[9] * m[2] * m[15] -
		m[9] * m[3] * m[14] -
		m[13] * m[2] * m[11] +
		m[13] * m[3] * m[10];

	inv[5] = m[0] * m[10] * m[15] -
		m[0] * m[11] * m[14] -
		m[8] * m[2] * m[15] +
		m[8] * m[3] * m[14] +
		m[12] * m[2] * m[11] -
		m[12] * m[3] * m[10];

	inv[9] = -m[0] * m[9] * m[15] +
		m[0] * m[11] * m[13] +
		m[8] * m[1] * m[15] -
		m[8] * m[3] * m[13] -
		m[12] * m[1] * m[11] +
		m[12] * m[3] * m[9];

	inv[13] = m[0] * m[9] * m[14] -
		m[0] * m[10] * m[13] -
		m[8] * m[1] * m[14] +
		m[8] * m[2] * m[13] +
		m[12] * m[1] * m[10] -
		m[12] * m[2] * m[9];

	inv[2] = m[1] * m[6] * m[15] -
		m[1] * m[7] * m[14] -
		m[5] * m[2] * m[15] +
		m[5] * m[3] * m[14] +
		m[13] * m[2] * m[7] -
		m[13] * m[3] * m[6];

	inv[6] = -m[0] * m[6] * m[15] +
		m[0] * m[7] * m[14] +
		m[4] * m[2] * m[15] -
		m[4] * m[3] * m[14] -
		m[12] * m[2] * m[7] +
		m[12] * m[3] * m[6];

	inv[10] = m[0] * m[5] * m[15] -
		m[0] * m[7] * m[13] -
		m[4] * m[1] * m[15] +
		m[4] * m[3] * m[13] +
		m[12] * m[1] * m[7] -
		m[12] * m[3] * m[5];

	inv[14] = -m[0] * m[5] * m[14] +
		m[0] * m[6] * m[13] +
		m[4] * m[1] * m[14] -
		m[4] * m[2] * m[13] -
		m[12] * m[1] * m[6] +
		m[12] * m[2] * m[5];

	inv[3] = -m[1] * m[6] * m[11] +
		m[1] * m[7] * m[10] +
		m[5] * m[2] * m[11] -
		m[5] * m[3] * m[10] -
		m[9] * m[2] * m[7] +
		m[9] * m[3] * m[6];

	inv[7] = m[0] * m[6] * m[11] -
		m[0] * m[7] * m[10] -
		m[4] * m[2] * m[11] +
		m[4] * m[3] * m[10] +
		m[8] * m[2] * m[7] -
		m[8] * m[3] * m[6];

	inv[11] = -m[0] * m[5] * m[11] +
		m[0] * m[7] * m[9] +
		m[4] * m[1] * m[11] -
		m[4] * m[3] * m[9] -
		m[8] * m[1] * m[7] +
		m[8] * m[3] * m[5];

	inv[15] = m[0] * m[5] * m[10] -
		m[0] * m[6] * m[9] -
		m[4] * m[1] * m[10] +
		m[4] * m[2] * m[9] +
		m[8] * m[1] * m[6] -
		m[8] * m[2] * m[5];

	det = m[0] * inv[0] + m[1] * inv[4] + m[2] * inv[8] + m[3] * inv[12];
	det = 1.0f / det;

	Mat44 ret;
	for (i = 0; i < 16; i++) {
		ret.m_values[i] = (inv[i] * det);
	}

	return ret;
}


Vec2 const Mat44::TransformVectorQuantity2D(Vec2 const& vectorQuantityXY) const
{
	float x = DotProduct2D(Vec2(m_values[Ix], m_values[Jx]), vectorQuantityXY);
	float y = DotProduct2D(Vec2(m_values[Iy], m_values[Jy]), vectorQuantityXY);
	return Vec2(x, y);
}


Vec3 const Mat44::TransformVectorQuantity3D(Vec3 const& vectorQuantityXYZ) const
{
	float x = DotProduct3D(Vec3(m_values[Ix], m_values[Jx], m_values[Kx]), vectorQuantityXYZ);
	float y = DotProduct3D(Vec3(m_values[Iy], m_values[Jy], m_values[Ky]), vectorQuantityXYZ);
	float z = DotProduct3D(Vec3(m_values[Iz], m_values[Jz], m_values[Kz]), vectorQuantityXYZ);
	return Vec3(x, y, z);
}


Vec2 const Mat44::TransformPosition2D(Vec2 const& positionXY) const
{
	float x = DotProduct2D(Vec2(m_values[Ix], m_values[Jx]), positionXY) + m_values[Tx];
	float y = DotProduct2D(Vec2(m_values[Iy], m_values[Jy]), positionXY) + m_values[Ty];
	return Vec2(x, y);
}


Vec3 const Mat44::TransformPosition3D(Vec3 const& position3D) const
{
	float x = DotProduct3D(Vec3(m_values[Ix], m_values[Jx], m_values[Kx]), position3D) + m_values[Tx];
	float y = DotProduct3D(Vec3(m_values[Iy], m_values[Jy], m_values[Ky]), position3D) + m_values[Ty];
	float z = DotProduct3D(Vec3(m_values[Iz], m_values[Jz], m_values[Kz]), position3D) + m_values[Tz];
	return Vec3(x, y, z);
}


Vec4 const Mat44::TransformHomogeneous3D(Vec4 const& homogeneousPoint3D) const
{
	float x = DotProduct4D(Vec4(m_values[Ix], m_values[Jx], m_values[Kx], m_values[Tx]), Vec4(homogeneousPoint3D));
	float y = DotProduct4D(Vec4(m_values[Iy], m_values[Jy], m_values[Ky], m_values[Ty]), Vec4(homogeneousPoint3D));
	float z = DotProduct4D(Vec4(m_values[Iz], m_values[Jz], m_values[Kz], m_values[Tz]), Vec4(homogeneousPoint3D));
	float w = DotProduct4D(Vec4(m_values[Iw], m_values[Jw], m_values[Kw], m_values[Tw]), Vec4(homogeneousPoint3D));
	return Vec4(x, y, z, w);
}


float* Mat44::GetAsFloatArray()
{
	return m_values;
}


float const* Mat44::GetAsFloatArray() const
{
	return m_values;
}


Vec2 const Mat44::GetIBasis2D() const
{
	return Vec2(m_values[Ix], m_values[Iy]);
}


Vec2 const Mat44::GetJBasis2D() const
{
	return Vec2(m_values[Jx], m_values[Jy]);
}


Vec2 const Mat44::GetTranslation2D() const
{
	return Vec2(m_values[Tx], m_values[Ty]);
}


Vec3 const Mat44::GetIBasis3D() const
{
	return Vec3(m_values[Ix], m_values[Iy], m_values[Iz]);
}


Vec3 const Mat44::GetJBasis3D() const
{
	return Vec3(m_values[Jx], m_values[Jy], m_values[Jz]);
}


Vec3 const Mat44::GetKBasis3D() const
{
	return Vec3(m_values[Kx], m_values[Ky], m_values[Kz]);
}


Vec3 const Mat44::GetTranslation3D() const
{
	return Vec3(m_values[Tx], m_values[Ty], m_values[Tz]);
}


Vec4 const Mat44::GetIBasis4D() const
{
	return Vec4(m_values[Ix], m_values[Iy], m_values[Iz], m_values[Iw]);
}


Vec4 const Mat44::GetJBasis4D() const
{
	return Vec4(m_values[Jx], m_values[Jy], m_values[Jz], m_values[Jw]);
}


Vec4 const Mat44::GetKBasis4D() const
{
	return Vec4(m_values[Kx], m_values[Ky], m_values[Kz], m_values[Kw]);
}


Vec4 const Mat44::GetTranslation4D() const
{
	return Vec4(m_values[Tx], m_values[Ty], m_values[Tz], m_values[Tw]);
}


Mat44 const Mat44::GetOrthonormalInverse() const
{
	Mat44 translation = CreateTranslation3D(-1.f * GetTranslation3D());

	Mat44 rotation = *this;
	rotation.m_values[Tx] = 0.f;
	rotation.m_values[Ty] = 0.f;
	rotation.m_values[Tz] = 0.f;
	rotation.Transpose();

	rotation.Append(translation);
	return rotation;
}


EulerAngles const Mat44::GetEulerAngles_XFwd_YLeft_ZUp() const
{
	float y = atan2f(m_values[Iy], m_values[Ix]);
	float p = atan2f(-m_values[Iz], sqrtf(m_values[Jz] * m_values[Jz] + m_values[Kz] * m_values[Kz]));
	float r = atan2f(m_values[Jz], m_values[Kz]);

	float yawDegrees = ConvertRadiansToDegrees(y);
	float pitchDegrees = ConvertRadiansToDegrees(p);
	float rollDegrees = ConvertRadiansToDegrees(r);

	return EulerAngles(yawDegrees, pitchDegrees, rollDegrees);
}


Vec3 const Mat44::GetAngleAxis_XFwd_YLeft_ZUp() const
{
	float rotation = acosf(0.5f * (m_values[Ix] + m_values[Jy] + m_values[Kz] - 1.f));
	float a1 = m_values[Jz] - m_values[Ky];
	float a2 = m_values[Kx] - m_values[Iz];
	float a3 = m_values[Iy] - m_values[Jx];
	float s = sqrtf((a1 * a1) + (a2 * a2) + (a3 * a3));
	float x = a1 / s;
	float y = a2 / s;
	float z = a3 / s;
	
	return Vec3(x, y, z) * rotation;
}


void Mat44::SetTranslation2D(Vec2 const& translationXY)
{
	m_values[Tx] = translationXY.x;
	m_values[Ty] = translationXY.y;
	m_values[Tz] = 0.f;
	m_values[Tw] = 1.f;
}


void Mat44::SetTranslation3D(Vec3 const& translationXYZ)
{
	m_values[Tx] = translationXYZ.x;
	m_values[Ty] = translationXYZ.y;
	m_values[Tz] = translationXYZ.z;
	m_values[Tw] = 1.f;
}


void Mat44::SetIJ2D(Vec2 const& iBasis2D, Vec2 const& jBasis2D)
{
	m_values[Ix] = iBasis2D.x;
	m_values[Iy] = iBasis2D.y;
	m_values[Iz] = 0.f;
	m_values[Iw] = 0.f;
	m_values[Jx] = jBasis2D.x;
	m_values[Jy] = jBasis2D.y;
	m_values[Jz] = 0.f;
	m_values[Jw] = 0.f;
}


void Mat44::SetIJT2D(Vec2 const& iBasis2D, Vec2 const& jBasis2D, Vec2 const& translationXY)
{
	SetIJ2D(iBasis2D, jBasis2D);
	SetTranslation2D(translationXY);
}


void Mat44::SetIJK3D(Vec3 const& iBasis3D, Vec3 const& jBasis3D, Vec3 const& kBasis3D)
{
	m_values[Ix] = iBasis3D.x;
	m_values[Iy] = iBasis3D.y;
	m_values[Iz] = iBasis3D.z;
	m_values[Iw] = 0.f;
	m_values[Jx] = jBasis3D.x;
	m_values[Jy] = jBasis3D.y;
	m_values[Jz] = jBasis3D.z;
	m_values[Jw] = 0.f;
	m_values[Kx] = kBasis3D.x;
	m_values[Ky] = kBasis3D.y;
	m_values[Kz] = kBasis3D.z;
	m_values[Kw] = 0.f;
}


void Mat44::SetIJKT3D(Vec3 const& iBasis3D, Vec3 const& jBasis3D, Vec3 const& kBasis3D, Vec3 const& translationXYZ)
{
	SetIJK3D(iBasis3D, jBasis3D, kBasis3D);
	SetTranslation3D(translationXYZ);
}


void Mat44::SetIJKT4D(Vec4 const& iBasis4D, Vec4 const& jBasis4D, Vec4 const& kBasis4D, Vec4 const& translation4D)
{
	m_values[Ix] = iBasis4D.x;
	m_values[Iy] = iBasis4D.y;
	m_values[Iz] = iBasis4D.z;
	m_values[Iw] = iBasis4D.w;
	m_values[Jx] = jBasis4D.x;
	m_values[Jy] = jBasis4D.y;
	m_values[Jz] = jBasis4D.z;
	m_values[Jw] = jBasis4D.w;
	m_values[Kx] = kBasis4D.x;
	m_values[Ky] = kBasis4D.y;
	m_values[Kz] = kBasis4D.z;
	m_values[Kw] = kBasis4D.w;
	m_values[Tx] = translation4D.x;
	m_values[Ty] = translation4D.y;
	m_values[Tz] = translation4D.z;
	m_values[Tw] = translation4D.w;
}


void Mat44::Transpose()
{
	Mat44 copy = *this;
	//m_values[Ix] = copy.m_values[Ix];
	m_values[Iy] = copy.m_values[Jx];
	m_values[Iz] = copy.m_values[Kx];
	m_values[Iw] = copy.m_values[Tx];
	m_values[Jx] = copy.m_values[Iy];
	//m_values[Jy] = copy.m_values[Jy];
	m_values[Jz] = copy.m_values[Ky];
	m_values[Jw] = copy.m_values[Ty];
	m_values[Kx] = copy.m_values[Iz];
	m_values[Ky] = copy.m_values[Jz];
	//m_values[Kz] = copy.m_values[Kz];
	m_values[Kw] = copy.m_values[Tz];
	m_values[Tx] = copy.m_values[Iw];
	m_values[Ty] = copy.m_values[Jw];
	m_values[Tz] = copy.m_values[Kw];
	//m_values[Tw] = copy.m_values[Tw];
}


void Mat44::Orthonormalize_XFwd_YLeft_ZUp()
{
	Vec3 iBasis = Vec3(m_values[Ix], m_values[Iy], m_values[Iz]);
	Vec3 jBasis = Vec3(m_values[Jx], m_values[Jy], m_values[Jz]);
	Vec3 kBasis = Vec3(m_values[Kx], m_values[Ky], m_values[Kz]);

	iBasis = iBasis.GetNormalized();
	kBasis -= GetProjectedOnto3D(kBasis, iBasis);
	kBasis = kBasis.GetNormalized();
	jBasis -= GetProjectedOnto3D(jBasis, kBasis);
	jBasis -= GetProjectedOnto3D(jBasis, iBasis);
	jBasis = jBasis.GetNormalized();

	m_values[Ix] = iBasis.x;
	m_values[Iy] = iBasis.y;
	m_values[Iz] = iBasis.z;
	m_values[Jx] = jBasis.x;
	m_values[Jy] = jBasis.y;
	m_values[Jz] = jBasis.z;
	m_values[Kx] = kBasis.x;
	m_values[Ky] = kBasis.y;
	m_values[Kz] = kBasis.z;
}


void Mat44::Append(Mat44 const& appendThis)
{
	float const* append_values = appendThis.m_values;
	float newValues[16] = {};

	newValues[Ix] = (m_values[Ix] * append_values[Ix]) + (m_values[Jx] * append_values[Iy]) + (m_values[Kx] * append_values[Iz]) + (m_values[Tx] * append_values[Iw]);
	newValues[Iy] = (m_values[Iy] * append_values[Ix]) + (m_values[Jy] * append_values[Iy]) + (m_values[Ky] * append_values[Iz]) + (m_values[Ty] * append_values[Iw]);
	newValues[Iz] = (m_values[Iz] * append_values[Ix]) + (m_values[Jz] * append_values[Iy]) + (m_values[Kz] * append_values[Iz]) + (m_values[Tz] * append_values[Iw]);
	newValues[Iw] = (m_values[Iw] * append_values[Ix]) + (m_values[Jw] * append_values[Iy]) + (m_values[Kw] * append_values[Iz]) + (m_values[Tw] * append_values[Iw]);
	
	newValues[Jx] = (m_values[Ix] * append_values[Jx]) + (m_values[Jx] * append_values[Jy]) + (m_values[Kx] * append_values[Jz]) + (m_values[Tx] * append_values[Jw]);
	newValues[Jy] = (m_values[Iy] * append_values[Jx]) + (m_values[Jy] * append_values[Jy]) + (m_values[Ky] * append_values[Jz]) + (m_values[Ty] * append_values[Jw]);
	newValues[Jz] = (m_values[Iz] * append_values[Jx]) + (m_values[Jz] * append_values[Jy]) + (m_values[Kz] * append_values[Jz]) + (m_values[Tz] * append_values[Jw]);
	newValues[Jw] = (m_values[Iw] * append_values[Jx]) + (m_values[Jw] * append_values[Jy]) + (m_values[Kw] * append_values[Jz]) + (m_values[Tw] * append_values[Jw]);
	
	newValues[Kx] = (m_values[Ix] * append_values[Kx]) + (m_values[Jx] * append_values[Ky]) + (m_values[Kx] * append_values[Kz]) + (m_values[Tx] * append_values[Kw]);
	newValues[Ky] = (m_values[Iy] * append_values[Kx]) + (m_values[Jy] * append_values[Ky]) + (m_values[Ky] * append_values[Kz]) + (m_values[Ty] * append_values[Kw]);
	newValues[Kz] = (m_values[Iz] * append_values[Kx]) + (m_values[Jz] * append_values[Ky]) + (m_values[Kz] * append_values[Kz]) + (m_values[Tz] * append_values[Kw]);
	newValues[Kw] = (m_values[Iw] * append_values[Kx]) + (m_values[Jw] * append_values[Ky]) + (m_values[Kw] * append_values[Kz]) + (m_values[Tw] * append_values[Kw]);
	
	newValues[Tx] = (m_values[Ix] * append_values[Tx]) + (m_values[Jx] * append_values[Ty]) + (m_values[Kx] * append_values[Tz]) + (m_values[Tx] * append_values[Tw]);
	newValues[Ty] = (m_values[Iy] * append_values[Tx]) + (m_values[Jy] * append_values[Ty]) + (m_values[Ky] * append_values[Tz]) + (m_values[Ty] * append_values[Tw]);
	newValues[Tz] = (m_values[Iz] * append_values[Tx]) + (m_values[Jz] * append_values[Ty]) + (m_values[Kz] * append_values[Tz]) + (m_values[Tz] * append_values[Tw]);
	newValues[Tw] = (m_values[Iw] * append_values[Tx]) + (m_values[Jw] * append_values[Ty]) + (m_values[Kw] * append_values[Tz]) + (m_values[Tw] * append_values[Tw]);

	for (int i = 0; i < 16; i++) 
	{
		m_values[i] = newValues[i];
	}
}


void Mat44::AppendZRotation(float degreesRotationAboutZ)
{
	Append(CreateZRotationDegrees(degreesRotationAboutZ));
}


void Mat44::AppendYRotation(float degreesRotationAboutY)
{
	Append(CreateYRotationDegrees(degreesRotationAboutY));
}


void Mat44::AppendXRotation(float degreesRotationAboutX)
{
	Append(CreateXRotationDegrees(degreesRotationAboutX));
}


void Mat44::AppendTranslation2D(Vec2 const& translationXY)
{
	Append(CreateTranslation2D(translationXY));
}


void Mat44::AppendTranslation3D(Vec3 const& translationXYZ)
{
	Append(CreateTranslation3D(translationXYZ));
}


void Mat44::AppendScaleUniform2D(float uniformScaleXY)
{
	Append(CreateUniformScale2D(uniformScaleXY));
}


void Mat44::AppendScaleUniform3D(float uniformScaleXYZ)
{
	Append(CreateUniformScale3D(uniformScaleXYZ));
}


void Mat44::AppendScaleNonUniform2D(Vec2 const& nonUniformScaleXY)
{
	Append(CreateNonUniformScale2D(nonUniformScaleXY));
}


void Mat44::AppendScaleNonUniform3D(Vec3 const& nonUniformScaleXYZ)
{
	Append(CreateNonUniformScale3D(nonUniformScaleXYZ));
}


