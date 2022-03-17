#pragma once

class RHI
{
public:
	virtual ~RHI();
	virtual void Init() = 0;
	virtual void Destroy() = 0;
	virtual void Draw() = 0;

	virtual void BuildInitialMap() = 0;
};