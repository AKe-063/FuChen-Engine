#pragma once

class FInputBase
{
public:
	FInputBase();
	virtual ~FInputBase();

	virtual bool Init();
	virtual void Update(const GameTimer& gt);
protected:


private:

};