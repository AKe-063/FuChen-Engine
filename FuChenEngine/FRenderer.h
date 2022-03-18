#pragma once

class FRenderer
{
public:
	FRenderer();
	virtual ~FRenderer();

	virtual void Init() = 0;
	virtual void Destroy() = 0;
	virtual void Render() = 0;
	virtual void BuildPrimitives() = 0;
	virtual void Draw() = 0;
	virtual void BuildInitialMap() = 0;

private:

};