#include "stdafx.h"
#include "CommonInputLogic.h"
#include "FWin32Input.h"
#include "Engine.h"

CommonInputLogic::CommonInputLogic()
{
}

CommonInputLogic::~CommonInputLogic()
{
}

bool CommonInputLogic::OnChoosePostProcessKeyDown()
{
	if (FWin32Input::GetInstance().IsKeyDown('B'))
	{
		Engine::GetInstance().GetRenderer()->ChosePostProcess(CHOSE_POST_PROCESS_MODEL::BLOOM);
		return true;
	}
	if (FWin32Input::GetInstance().IsKeyDown('C'))
	{
		Engine::GetInstance().GetRenderer()->ChosePostProcess(CHOSE_POST_PROCESS_MODEL::CYBERPUNK);
		return true;
	}
	if (FWin32Input::GetInstance().IsKeyDown('N'))
	{
		Engine::GetInstance().GetRenderer()->ChosePostProcess(CHOSE_POST_PROCESS_MODEL::NONE);
		return true;
	}

	return false;
}
