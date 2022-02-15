#pragma once

using Microsoft::WRL::ComPtr;

class D3D12HelloWindow : public DXSample
{
public:
	D3D12HelloWindow(UINT width, UINT height, std::wstring name);

	virtual void OnInit();
	virtual void OnUpdate();
	virtual void OnRender();
	virtual void OnDestroy();

private:
	static const UINT FrameCount = 2;

	UINT m_rtvDescriptorSize;		//Render Target View Descriptor Size

	UINT m_frameIndex;
};