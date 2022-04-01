#pragma once
#include "FLightDesc.h"

class FLight
{
public:
	FLight();
	FLight(FLight&) = default;
	~FLight();

	FLightDesc* GetFlightDesc();
	void UpdateLightTrans();

private:
	FLightDesc fLightDesc;
	int rota = 0;
	bool flag = true;
};