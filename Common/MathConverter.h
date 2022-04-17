#pragma once

#include <string>

#include "EMath.h"
#include "SimpleMath.h"
using namespace DirectX;

/// <summary>
/// EMath의 Vector, Matrix들을 SimpleMath의 Vector, Matrix로 변환하여 반환해준다.
/// 2021. 10. 28 정종영
/// </summary>
class MathConverter
{
public:
	static SimpleMath::Vector2 EMathVector2_To_SimpleMathVector2(EMath::Vector2 e);
	static SimpleMath::Vector3 EMathVector3_To_SimpleMathVector3(EMath::Vector3 e);
	static SimpleMath::Vector4 EMathVector4_To_SimpleMathVector4(EMath::Vector4 e);
	static SimpleMath::Matrix EMathMatrix_To_SimpleMathMatrix(EMath::Matrix e);

	static EMath::Vector2 SimpleMathVector2_To_EMathVector2(SimpleMath::Vector2 e);
	static EMath::Vector3 SimpleMathVector3_To_EMathVector3(SimpleMath::Vector3 e);
	static EMath::Vector4 SimpleMathVector4_To_EMathVector4(SimpleMath::Vector4 e);
	static EMath::Matrix SimpleMathMatrix_To_EMathMatrix(SimpleMath::Matrix e);
};