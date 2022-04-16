#include "ClientSize.h"

int ClientSize::Width = 0;
int ClientSize::Height = 0;

float ClientSize::AspectRatio()
{
	return static_cast<float>(ClientSize::Width) / ClientSize::Height;
}
