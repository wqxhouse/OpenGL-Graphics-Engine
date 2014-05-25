#pragma once

#include "Bounds.h"
#include "Vector3.h"

class ObjectViewer
{
public:
	ObjectViewer(void);
	virtual ~ObjectViewer(void);

	void renderBBox(const BBox &bbox, const Vector3 &color); 
	inline void toggleVisible()
	{
		enable_toggle_ = enable_toggle_ ? false : true;
	}

private:
	bool enable_toggle_;
};

