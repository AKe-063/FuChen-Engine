#include "stdafx.h"

D3D12HelloWindow::D3D12HelloWindow(UINT width, UINT height, std::wstring name) :
	DXSample(width,height,name),
	m_frameIndex(0),
	m_rtvDescriptorSize(0)
{
}
