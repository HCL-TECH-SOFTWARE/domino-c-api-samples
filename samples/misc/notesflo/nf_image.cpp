/*
* Copyright HCL Technologies 1996, 2025.
*
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
* http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*/
//
//		nf_image.cpp
//
//	Implementation of pattern-drawing classes:
//		CDrawEnvironment
//		CImagePattern
//

#include <windows.h>

#include "notesflo.h"
#include "nf_image.hpp"
#include "nf_store.hpp"

//
//		Class CDrawEnvironment
//

	// Constructor
CDrawEnvironment::CDrawEnvironment (
	HDC			hDc,
	RECT FAR *	lpClientRect,
	RECT FAR *	lpInvalidRect,
	BOOL		fIsMetafile
)
	: m_hDc (hDc), m_fIsMetafile (fIsMetafile),
	m_hImageBrush ((HBRUSH) NULL), m_hImagePen ((HPEN) NULL),
	m_hOldBrush ((HBRUSH) NULL), m_hOldPen ((HPEN) NULL)
{
		// Copy the rectangles
	m_ClientRect = *lpClientRect;
	m_InvalidRect = *lpInvalidRect;
}

	// Destructor
CDrawEnvironment::~CDrawEnvironment (
	void
) {
		// If we created drawing tools, don't forget to delete them!
	if (((HBRUSH) NULL) != m_hImageBrush)
	{
		::DeleteObject (m_hImageBrush);
		m_hImageBrush = (HBRUSH) NULL;
	}

	if (((HPEN) NULL) != m_hImagePen)
	{
		::DeleteObject (m_hImagePen);
		m_hImagePen = (HPEN) NULL;
	}
}

	// Set the drawing color
void CDrawEnvironment::SetColors (
	COLORREF	newBgColor,
	COLORREF	newFgColor
) {
	LOGBRUSH	LogBrush;

		// Create a pen with the background color
	m_hImagePen = ::CreatePen (PS_INSIDEFRAME, 0, newBgColor);
        m_hOldPen = (HPEN)::SelectObject (m_hDc, m_hImagePen);

		// Create a solid brush with the specified foreground color
	LogBrush.lbStyle = BS_SOLID;
	LogBrush.lbColor = newFgColor;
	LogBrush.lbHatch = 0;

	m_hImageBrush = ::CreateBrushIndirect (&LogBrush);
        m_hOldBrush = (HBRUSH)::SelectObject (m_hDc, m_hImageBrush);
}

	// Restore the original drawing color
void CDrawEnvironment::RestoreColors (
	void
) {
		// If this is a metafile, there's no "original brush" to restore
	if (!m_fIsMetafile)
	{
		::SelectObject (m_hDc, m_hOldBrush);
		::SelectObject (m_hDc, m_hOldPen);
	}

	if (((HBRUSH) NULL) != m_hImageBrush)
	{
		::DeleteObject (m_hImageBrush);
		m_hImageBrush = (HBRUSH) NULL;
	}

	if (((HPEN) NULL) != m_hImagePen)
	{
		::DeleteObject (m_hImagePen);
		m_hImagePen = (HPEN) NULL;
	}
}

	// Draw the desired rectangle
void CDrawEnvironment::FillArea (
	RECT FAR *	lpDrawRect
) {
		// Make sure the handles are valid
	if (((HDC) NULL) != m_hDc)
	{
		::Rectangle (
			m_hDc,
			lpDrawRect->left - 1,
			lpDrawRect->top - 1,
			lpDrawRect->right + 1,
			lpDrawRect->bottom + 1);
	}
}

//
//		Class CImagePattern
//

	// Constructor
CImagePattern::CImagePattern (
	void
) {
		// Set the default colors
	m_ForegroundColor = DEFAULT_FOREGROUND_COLOR;
	m_BackgroundColor = DEFAULT_BACKGROUND_COLOR;
}

	// Destructor
CImagePattern::~CImagePattern (
	void
) {
	;
}

	// Rectangle normalization - this algorithm works best if the
	// boundaries are set to a power of 3.  This routine finds the
	// largest dimension of the bounding rectangle, computes the
	// next-largest power of 3, and sets the width and height of the
	// bounding rectangle to that value.
void CImagePattern::NormalizeRectangle (
	RECT FAR *	lpBounds
) const {
	LONG		max;
	LONG		size;

		// Take the largest dimension of the area . . .
	max = (lpBounds->bottom > lpBounds->right) ? lpBounds->bottom : lpBounds->right;

		// . . . find the next-biggest power of 3 . . .
	for (size = 1; size < max; size *= 3)
		;

		// . . . and inflate the rectangle.
	lpBounds->top = 0;
	lpBounds->bottom = size;
	lpBounds->left = 0;
	lpBounds->right = size;
}

	// Draw the Sierpinski carpet pattern
	//
	// This is a 2-dimensional representation of a set defined by the
	// mathemetician Sierpinski.  Divide a square into a 3x3 grid, and
	// cut out the center square.  For each of the 8 squares surrounding
	// the cut-out center, repeat the process.
	//
	// In theory, the process continues until you're left with a surface
	// having an infinite perimeter and zero area.  However, this program
	// only continues until we get down to 1 pixel!
void CImagePattern::DrawSierpinskiCarpet (
	CDrawEnvironment &	drawEnv,
	RECT FAR *			lpDrawRect
) {
	LONG		width;
	LONG		height;
	LONG		size;
	int			i, j;
	RECT		newArea;

	width = (lpDrawRect->right - lpDrawRect->left) / 3;
	height = (lpDrawRect->bottom - lpDrawRect->top) / 3;
	size = (height < width) ? height : width;

	if (size > 1)		// Quit when we run out of pixels
	{
		for (i = 0; i < 3; i++)
		{
			for (j = 0; j < 3; j++)
			{
				RECT	invalidArea;	// Needed by IntersectRect

				newArea.top = lpDrawRect->top + (i * size);
				newArea.bottom = newArea.top + size;
				newArea.left = lpDrawRect->left + (j * size);
				newArea.right = newArea.left + size;

					// Don't bother if we're out of the invalid rectangle
				if (::IntersectRect (&invalidArea, drawEnv.GetInvalidRectPtr (), &newArea))
					if ((1 == i) && (1 == j))
						drawEnv.FillArea (&invalidArea);
					else
						this->DrawSierpinskiCarpet (drawEnv, &newArea);
							// Oh, no!!!  Recursion!!!
			}
		}
	}
	else
	{
			// We're down to a 3x3 area - just draw the center pixel
		newArea.top = lpDrawRect->top + 1;
		newArea.bottom = newArea.top + 1;
		newArea.left = lpDrawRect->left + 1;
		newArea.right = newArea.left + 1;

		drawEnv.FillArea (&newArea);
	}
}
