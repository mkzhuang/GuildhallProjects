#include "Engine/Renderer/Billboard.hpp"

Mat44 GetModelMatrixCameraOpposingXYZ(Vec3 const& position, Camera const& camera)
{
	Mat44 modelMatrix;
	modelMatrix.Append(camera.GetViewMatrix().GetOrthonormalInverse());
	modelMatrix.Append(camera.GetRenderMatrix());
	modelMatrix.SetTranslation3D(position);
	return modelMatrix;
}


