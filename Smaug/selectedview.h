#pragma once
#include "baseview.h"

class CSelectedView : public CBaseView
{
public:
	virtual void Init(bgfx::ViewId viewId, int width, int height, uint32_t clearColor);
	virtual void Draw(float dt);

};
