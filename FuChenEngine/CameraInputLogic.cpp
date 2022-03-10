#include "stdafx.h"
#include "CameraInputLogic.h"
#include "Engine.h"
#include "FWin32Input.h"

CameraInputLogic::CameraInputLogic()
{

}

CameraInputLogic::~CameraInputLogic()
{

}

void CameraInputLogic::OnKeyboardInput()
{
	const float dt = Engine::GetInstance().GetTimer()->DeltaTime();
	const float speed = 700.0f;

	if (FWin32Input::GetInstance().IsKeyDown('W'))
		Engine::GetInstance().GetFScene()->GetCamera()->GetControlCamera()->Walk(speed * dt);

	if (FWin32Input::GetInstance().IsKeyDown('S'))
		Engine::GetInstance().GetFScene()->GetCamera()->GetControlCamera()->Walk(speed * -dt);

	if (FWin32Input::GetInstance().IsKeyDown('A'))
		Engine::GetInstance().GetFScene()->GetCamera()->GetControlCamera()->Strafe(speed * -dt);

	if (FWin32Input::GetInstance().IsKeyDown('D'))
		Engine::GetInstance().GetFScene()->GetCamera()->GetControlCamera()->Strafe(speed * dt);

	if (FWin32Input::GetInstance().IsKeyDown('Q'))
		Engine::GetInstance().GetFScene()->GetCamera()->GetControlCamera()->Roll(0.05f);

	if (FWin32Input::GetInstance().IsKeyDown('E'))
		Engine::GetInstance().GetFScene()->GetCamera()->GetControlCamera()->Roll(-0.05f);

	Engine::GetInstance().GetFScene()->GetCamera()->GetControlCamera()->UpdateViewMatrix();
}

void CameraInputLogic::OnMouseInput()
{
	bool* flag = FWin32Input::GetInstance().GetMouseInputFlag();
	WPARAM* wParam = FWin32Input::GetInstance().GetMouseWparam();
	LPARAM* lParam = FWin32Input::GetInstance().GetMouselParam();
	if (flag[0])
	{
		OnMouseUp(wParam[0], GET_X_LPARAM(lParam[0]), GET_Y_LPARAM(lParam[0]));
	}
	if (flag[1])
	{
		OnMouseDown(wParam[1], GET_X_LPARAM(lParam[1]), GET_Y_LPARAM(lParam[1]));
	}
	if (flag[2])
	{
		OnMouseMove(wParam[2], GET_X_LPARAM(lParam[2]), GET_Y_LPARAM(lParam[2]));
	}
}

void CameraInputLogic::OnMouseDown(WPARAM btnState, int x, int y)
{
	mLastMousePos.x = x;
	mLastMousePos.y = y;

	SetCapture(GetActiveWindow());
}

void CameraInputLogic::OnMouseUp(WPARAM btnState, int x, int y)
{
	ReleaseCapture();
}

void CameraInputLogic::OnMouseMove(WPARAM btnState, int x, int y)
{
	if ((btnState & MK_LBUTTON) != 0)
	{
		// Make each pixel correspond to a quarter of a degree.
		float dx = glm::radians(0.25f * static_cast<float>(x - mLastMousePos.x));
		float dy = glm::radians(0.25f * static_cast<float>(y - mLastMousePos.y));

		// Update angles based on input to orbit camera around box.
		Engine::GetInstance().GetFScene()->GetCamera()->Pitch(dy);
		Engine::GetInstance().GetFScene()->GetCamera()->Yaw(dx);

		// Restrict the angle mPhi.
		/*mPhi = MathHelper::Clamp(mPhi, 0.1f, MathHelper::Pi - 0.1f);*/

	}
	mLastMousePos.x = x;
	mLastMousePos.y = y;
}
