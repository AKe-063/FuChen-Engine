#include "stdafx.h"
#include "FBloomPP.h"

FBloomPP::FBloomPP(int32_t downUp, std::shared_ptr<FRenderTarget>& sceneColorRT)
	:downUpNum(downUp)
{
// 	float width = (float)sceneColorRT->Width() / 4;
// 	float height = (float)sceneColorRT->Height() / 4;
// 
// 	mBloomSetUpRT = std::make_shared<FRenderTarget>();
// 	mBloomSetUpRT->mWidth = width;
// 	mBloomSetUpRT->mHeight = height;
// 	mBloomSunmergepsRT = std::make_shared<FRenderTarget>();
// 	mBloomSunmergepsRT->mWidth = width;
// 	mBloomSunmergepsRT->mHeight = height;
// 
// 	for (size_t i = 0; i < downUp; i++)
// 	{
// 		std::shared_ptr<FRenderTarget> fRT = std::make_shared<FRenderTarget>();
// 		fRT->mWidth = UINT(width / pow(2, i));
// 		fRT->mHeight = UINT(height / pow(2, i));
// 		mBloomDownRT.push_back(fRT);
// 	}
// 	for (size_t i = 0; i < downUp - 1; i++)
// 	{
// 		std::shared_ptr<FRenderTarget> fRT = std::make_shared<FRenderTarget>();
// 		fRT->mWidth = UINT(mBloomDownRT[mBloomDownRT.size() - 2 - i]->mWidth);
// 		fRT->mHeight = UINT(mBloomDownRT[mBloomDownRT.size() - 2 - i]->mHeight);
// 		mBloomUpRT.push_back(fRT);
// 	}
}
 
FBloomPP::~FBloomPP()
{

}
