#pragma once

#include "Shape.h"

class MovingCircle : public Shape
{
public:
	MovingCircle(const Point2F& pos, float radius, float speed, int dir, UINT32 rgb, FLOAT alpha = 1.0F, LPARAM userdata = 0) : Shape(pos, speed, dir, rgb, alpha, userdata), m_fRadius(radius) {}

	bool HitTest(Point2F pos) override {
		float distSq = (pos.x - m_pos.x) * (pos.x - m_pos.x) +
			(pos.y - m_pos.y) * (pos.y - m_pos.y);
		return distSq <= m_fRadius * m_fRadius;
	}

	void Draw(ID2D1HwndRenderTarget* pRenderTarget, const D2DRectScaler* pRS = NULL) override {
		D2D1_ELLIPSE e;
		e.point = GetPos();
		if (pRS)	pRS->Scale(&e.point);

		FLOAT r = m_fRadius;
		if (pRS)	pRS->ScaleNoOffset(&r);
		e.radiusX = e.radiusY = r;

		pRenderTarget->FillEllipse(&e, GetBrush());
		__super::Draw(pRenderTarget, pRS);
	}

	void GetBoundingBox(RectF* p, const Point2F& pos) const {
		p->left = pos.x - m_fRadius;
		p->right = pos.x + m_fRadius;
		p->top = pos.y - m_fRadius;
		p->bottom = pos.y + m_fRadius;
	}

	// Some bounce code for rectangle - sides and corners. Not quite right but close.
	bool WillBounceOffRectSides(const Shape& shape) {
		Point2F pos = shape.GetPos();	// Get where shape is going to be
		shape.MovePos(pos);
		RectF r;
		shape.GetBoundingBox(&r, pos);

		pos = GetPos();	// where we are
		MovePos(pos);	// where we will be

		// Check we're anywhere near it first
		if ((pos.x + m_fRadius < r.left) || (pos.x - m_fRadius > r.right) ||
			(pos.y + m_fRadius < r.top) || (pos.y - m_fRadius > r.bottom)) {
			return false;
		}

		float endlen = sqrt(m_cacheStep.lengthsq());
		float len = 0.0;
		while (len < endlen) {	// Check in radius/2 steps
			len += 1.0;

			pos = GetPos();		// where we are
			MovePos(pos, len);	// where we will be

			// Check left and right bounce zone
			if (WillHitRectX(r, pos)) {
				SetPos(pos);
				BounceX();
				return true;
			}

			// Check top and bottom bounce zone
			if (WillHitRectY(r, pos)) {
				SetPos(pos);
				BounceY();
				return true;
			}
		}

		return false;
	}

	bool WillBounceOffRectCorners(const Shape& shape) {
		Point2F pos = shape.GetPos();
		shape.MovePos(pos);
		RectF r;
		shape.GetBoundingBox(&r, pos);

		pos = GetPos();	// where we are
		MovePos(pos);	// where we will be

		// Check we're anywhere near it first
		if ((pos.x + m_fRadius < r.left) || (pos.x - m_fRadius > r.right) ||
			(pos.y + m_fRadius < r.top) || (pos.y - m_fRadius > r.bottom)) {
			return false;
		}

		double radsq = m_fRadius * m_fRadius;
		std::vector<Point2F> corners;
		r.GetCorners(corners);

		float endlensq = m_cacheStep.lengthsq();
		float lensq = 0.0;
		while (lensq < endlensq) {	// Check in radius/2 steps
			lensq += 1.0;
			MovePos(pos, lensq);	// where we will be

			// Check all 4 corners
			for (auto& corner : corners) {
				if (pos.DistanceToSq(corner) < radsq - 0.1F) {
					SetPos(pos);
					BounceOffPoint(corner);
					return true;
				}
			}
		}

		return false;
	}

protected:
	void BounceOffPoint(const Point2F& pt) {
		// Gradient between pt and direction
		double direction = m_cacheStep.anglerad();
		double touch = pt.angleradTo(m_pos);
		double deflection = M_PI - (direction - touch) * 2;
		m_direction += deflection;
		UpdateCache();
	}

	bool WillHitRectX(const RectF& r, const Point2F& pos) {
		if ((pos.y >= r.top) && (pos.y <= r.bottom)) {
			if ((pos.x >= r.right) && (pos.x - m_fRadius <= r.right)) {
				return true;
			}
			if ((pos.x <= r.left) && (pos.x + m_fRadius >= r.left)) {
				return true;
			}
		}
		return false;
	}

	bool WillHitRectY(const RectF& r, const Point2F& pos) {
		if ((pos.x >= r.left) && (pos.x <= r.right)) {
			if ((pos.y >= r.bottom) && (pos.y - m_fRadius <= r.bottom)) {
				return true;
			}
			if ((pos.y <= r.top) && (pos.y + m_fRadius >= r.top)) {
				return true;
			}
		}
		return false;
	}

protected:
	FLOAT m_fRadius;
};

class MovingRectangle : public Shape
{
public:
	MovingRectangle(const Point2F& pos, float width, float height, float speed, int dir, UINT32 rgb, FLOAT alpha = 1.0F, LPARAM userdata = 0) :
		Shape(pos, speed, dir, rgb, alpha, userdata), m_fWidth(width), m_fHeight(height)
	{
	}

	void SetSize(FLOAT width, FLOAT height) {
		m_fWidth = width;
		m_fHeight = height;
	}

	FLOAT GetWidth() { return m_fWidth; }
	FLOAT GetHeight() { return m_fHeight; }

	void SetWidth(FLOAT f) { m_fWidth = f; }
	void SetHeight(FLOAT f) { m_fHeight = f; }

	void Draw(ID2D1HwndRenderTarget* pRenderTarget, Point2F pos, const D2DRectScaler* pRS = NULL) override {
		FLOAT fWidth = m_fWidth;
		FLOAT fHeight = m_fHeight;
		if (pRS) {
			pRS->Scale(&pos);
			pRS->ScaleNoOffset(&fWidth);
			pRS->ScaleNoOffset(&fHeight);
		}

		D2D1_RECT_F r;
		r.left = pos.x;
		r.right = pos.x + fWidth;
		r.top = pos.y;
		r.bottom = pos.y + fHeight;
		pRenderTarget->FillRectangle(&r, GetBrush());
		__super::Draw(pRenderTarget, pRS);
	}

	void Draw(ID2D1HwndRenderTarget* pRenderTarget, const D2DRectScaler* pRS = NULL) override {
		Draw(pRenderTarget, GetPos(), pRS);
	}

	void GetBoundingBox(RectF* p, const Point2F& pos) const {
		p->left = pos.x;
		p->right = pos.x + m_fWidth - 1;
		p->top = pos.y;
		p->bottom = pos.y + m_fHeight - 1;
	}

protected:
	FLOAT m_fWidth;
	FLOAT m_fHeight;
};

class MovingBitmap : public Shape
{
public:
	MovingBitmap(d2dBitmap* bitmap, const Point2F& pos, float width, float height, float speed, int dir, UINT32 rgb, FLOAT alpha = 1.0F, LPARAM userdata = 0) :
		m_bitmap(bitmap), Shape(pos, speed, dir, rgb, alpha, userdata), m_fWidth(width), m_fHeight(height) {
	}

	void SetBitmap(d2dBitmap* bitmap) {
		m_bitmap = bitmap;
	}

	void Draw(ID2D1HwndRenderTarget* pRenderTarget, const D2DRectScaler* pRS = NULL) override {
		Point2F pos = GetPos();
		FLOAT fWidth = m_fWidth;
		FLOAT fHeight = m_fHeight;
		if (pRS) {
			pRS->Scale(&pos);
			pRS->ScaleNoOffset(&fWidth);
			pRS->ScaleNoOffset(&fHeight);
		}

		D2D1_RECT_F r;
		r.left = pos.x;
		r.right = pos.x + fWidth;
		r.top = pos.y;
		r.bottom = pos.y + fHeight;
		m_bitmap->Render(pRenderTarget, r);
		__super::Draw(pRenderTarget, pRS);
	}

	void GetBoundingBox(RectF* p, const Point2F& pos) const {
		p->left = pos.x;
		p->right = pos.x + m_fWidth;
		p->top = pos.y;
		p->bottom = pos.y + m_fHeight;
	}

protected:
	FLOAT m_fWidth;
	FLOAT m_fHeight;
	d2dBitmap* m_bitmap;
};

class MovingText : public Shape {
public:
	MovingText(LPCWSTR wsz, const Point2F& pos, float width, float height, float speed, int dir, DWRITE_TEXT_ALIGNMENT ta, UINT32 rgb, FLOAT alpha = 1.0F, LPARAM userdata = 0) :
		m_text(wsz),
		Shape(pos, speed, dir, rgb, alpha, userdata),
		m_fWidth(width), m_fHeight(height),
		m_pWTF(NULL),
		m_ta(ta)
	{
	}

	void D2DOnCreateResources(IDWriteFactory* pDWriteFactory, ID2D1HwndRenderTarget* pRenderTarget, IWICImagingFactory* pIWICFactory, const D2DRectScaler* pRS) override {
		FLOAT fHeight = m_fHeight;
		pRS->ScaleNoOffset(&fHeight);
		pDWriteFactory->CreateTextFormat(
			L"Arial",
			NULL,
			DWRITE_FONT_WEIGHT_REGULAR,
			DWRITE_FONT_STYLE_NORMAL,
			DWRITE_FONT_STRETCH_NORMAL,
			fHeight,
			L"en-us",
			&m_pWTF
		);

		m_pWTF->SetTextAlignment(m_ta);

		__super::D2DOnCreateResources(pDWriteFactory, pRenderTarget, pIWICFactory, pRS);
	}

	void D2DDiscardResources() override {
		SafeRelease(&m_pWTF);
		__super::D2DDiscardResources();
	}

	void SetText(LPCWSTR wsz) {
		m_text = wsz;
	}

	void Draw(ID2D1HwndRenderTarget* pRenderTarget, const D2DRectScaler* pRS = NULL) override {
		Point2F pos = GetPos();
		FLOAT fWidth = m_fWidth;
		FLOAT fHeight = m_fHeight;
		if (pRS) {
			pRS->Scale(&pos);
			pRS->ScaleNoOffset(&fWidth);
			pRS->ScaleNoOffset(&fHeight);
		}

		D2D1_RECT_F r;
		r.left = pos.x;
		r.right = pos.x + fWidth;
		r.top = pos.y;
		r.bottom = pos.y + fHeight;
		pRenderTarget->DrawTextW(m_text.c_str(), (UINT32)m_text.length(), m_pWTF, r, m_pBrush);
		__super::Draw(pRenderTarget, pRS);
	}

	virtual void GetBoundingBox(RectF* pRect, const Point2F& pos) const {
		pRect->left = pos.x;
		pRect->top = pos.y;
		pRect->right = pos.x + m_fWidth;
		pRect->bottom = pos.y + m_fHeight;
	}

protected:
	FLOAT m_fWidth;
	FLOAT m_fHeight;
	IDWriteTextFormat* m_pWTF;
	std::wstring m_text;
	DWRITE_TEXT_ALIGNMENT m_ta;
};

class MovingGroup : public MovingRectangle {
public:
	// Set width/height to !0 to debug where the shape is
	MovingGroup(const Point2F& pos, float speed, int dir, LPARAM userdata = 0) :
		MovingRectangle(pos, 0.0f, 0.0f, speed, dir, RGB(255, 0, 255), 1.0F, userdata) {
	}
};