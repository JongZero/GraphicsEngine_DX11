#pragma once
#include <memory>
#include "Shared_RenderingData.h"

class ResourceManager;

/// <summary>
/// Picking 담당
/// 2022. 01. 17 정종영
/// </summary>
class PickChecker
{
public:
	PickChecker(std::shared_ptr<ResourceManager> rm);
	~PickChecker();

private:
	std::shared_ptr<ResourceManager> m_ResourceManager;

public:
	void PickCheck(int x, int y, Shared_RenderingData* dataForRender);

private:
	float PickCheck(Shared_ObjectData* objDataForRender, const EMath::Vector4& rayOrigin, const EMath::Vector4& rayDir, const EMath::Matrix& invView);
};

