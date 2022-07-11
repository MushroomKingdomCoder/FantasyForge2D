#include "GraphicText.h"
#include <assert.h>

GraphicText::GraphicText(Graphics& gfx, unsigned int layer)
	:
	gfx(gfx),
	paper(layer),
	CharacterWidth(16u),
	CharacterHeight(16u),
	startChar(32u),
	charTableDim({ 16u,6u }),
	cursorLimit({ gfx.GetWidth(paper) / CharacterWidth, gfx.GetHeight(paper) / CharacterHeight }),
	tlMargins({ 0u,0u }),
	brMargins({ 0u,0u }),
	Cursor({ 0u,0u }),
	TextColor(Colors::White),
	TextScale(1u),
	autoCursor(true),
	lineFeed(true),
	lineSpacing(1u),
	TextTexture(),
	isUsingTexture(TextTexture)
{
	Image defaultCharset = Image("charsets\\default.bmp");
	CharBitmaps.resize(charTableDim.x * charTableDim.y);
	for (std::vector<bool>& cb : CharBitmaps)
	{
		cb.resize(CharacterWidth * CharacterHeight);
	}
	for (unsigned int i = 0u; i < CharBitmaps.size(); ++i)
	{
		Image charImage = defaultCharset.Cropped(CharacterWidth, CharacterHeight, (i % charTableDim.x) * CharacterWidth, (i / charTableDim.x) * CharacterHeight);
		for (unsigned int y = 0u; y < CharacterHeight; ++y)
		{
			for (unsigned int x = 0u; x < CharacterWidth; ++x)
			{
				CharBitmaps[i][y * CharacterWidth + x] = (charImage.GetPixel(x, y) != Colors::White);
			}
		}
	} 
}

GraphicText::GraphicText(Graphics& gfx, unsigned int layer, Image charset, uchar2 charTableDim, unsigned char startChar, uint2 cursor_pos, Color txtColor, unsigned int txtScale, bool doubleSpaced, bool auto_cursor, bool line_feed, uint2 tl_margins, uint2 br_margins, std::optional<Image> txtTexture)
	:
	gfx(gfx),
	paper(layer),
	CharacterWidth(charset.GetWidth() / charTableDim.x),
	CharacterHeight(charset.GetHeight() / charTableDim.y),
	startChar(startChar),
	charTableDim(charTableDim),
	cursorLimit({ gfx.GetWidth(paper) / (CharacterWidth * txtScale),gfx.GetHeight(paper) / (CharacterHeight * txtScale) }),
	tlMargins(tl_margins),
	brMargins(br_margins),
	Cursor(cursor_pos),
	TextColor(txtColor),
	TextScale(txtScale),
	autoCursor(auto_cursor),
	lineFeed(line_feed),
	lineSpacing(1u + doubleSpaced),
	TextTexture(txtTexture),
	isUsingTexture(TextTexture)
{
	assert(tl_margins.x + br_margins.x < cursorLimit.x);
	assert(tl_margins.y + br_margins.y < cursorLimit.y / 2u);
	assert(cursor_pos.x >= tlMargins.x);
	assert(cursor_pos.y >= tlMargins.y);
	assert(cursor_pos.x < cursorLimit.x - brMargins.x);
	assert(cursor_pos.y < cursorLimit.y - brMargins.y);
	if (TextTexture)
	{
		assert(TextTexture->GetWidth() == CharacterWidth);
		assert(TextTexture->GetHeight() == CharacterHeight);
	}
	CharBitmaps.resize(charTableDim.x * charTableDim.y);
	for (std::vector<bool>& cb : CharBitmaps)
	{
		cb.resize(CharacterWidth * CharacterHeight);
	}
	for (unsigned int i = 0u; i < CharBitmaps.size(); ++i)
	{
		Image charImage = charset.Cropped(CharacterWidth, CharacterHeight, (i % charTableDim.x) * CharacterWidth, (i / charTableDim.y) * CharacterHeight);
		for (unsigned int y = 0u; y < CharacterHeight; ++y)
		{
			for (unsigned int x = 0u; x < CharacterWidth; ++x)
			{
				CharBitmaps[i][y * CharacterWidth + x] = (charImage.GetPixel(x, y) != Colors::White);
			}
		}
	}
}

void GraphicText::SetTopLeftMargins(const uint2& tl_margins)
{
	assert(tl_margins.x + brMargins.x < cursorLimit.x);
	assert(tl_margins.y + brMargins.y < cursorLimit.y);
	if (Cursor.x < tl_margins.x)
	{
		Cursor.x = tl_margins.x;
	}
	if (Cursor.y < tl_margins.y)
	{
		Cursor.y = tl_margins.y;
	}
	tlMargins = tl_margins;
	ClearText();
}

void GraphicText::SetBottomRightMargins(const uint2& br_margins)
{
	assert(tlMargins.x + br_margins.x < cursorLimit.x);
	assert(tlMargins.y + br_margins.y < cursorLimit.y);
	if (Cursor.x >= cursorLimit.x - br_margins.x)
	{
		Cursor.x = cursorLimit.x - br_margins.x - 1u;
	}
	if (Cursor.y >= cursorLimit.y - br_margins.y)
	{
		Cursor.y = cursorLimit.y - br_margins.y - 1u;
	}
	brMargins = br_margins;
	ClearText();
}

const uint2& GraphicText::GetTopLeftMargins() const
{
	return tlMargins;
}

const uint2& GraphicText::GetBottomRightMargins() const
{
	return brMargins;
}

void GraphicText::CursorUp()
{
	for (unsigned int i = 0u; i < lineSpacing; ++i)
	{
		if (Cursor.y == tlMargins.y)
		{
			if (!lineFeed)
			{
				Cursor.y = cursorLimit.y - brMargins.y - 1u;
			}
			else
			{
				LineFeedDown();
			}
		}
		else
		{
			--Cursor.y;
		}
	}
}

void GraphicText::CursorRight()
{
	if (Cursor.x == cursorLimit.x - brMargins.x - 1u)
	{
		Cursor.x = tlMargins.x;
		CursorDown();
	}
	else
	{
		++Cursor.x;
	}
}

void GraphicText::CursorDown()
{
	for (unsigned int i = 0u; i < lineSpacing; ++i)
	{
		if (Cursor.y == cursorLimit.y - brMargins.y - 1u)
		{
			if (!lineFeed)
			{
				Cursor.y = tlMargins.y;
			}
			else
			{
				LineFeedUp();
			}
		}
		else
		{
			++Cursor.y;
		}
	}
}

void GraphicText::CursorLeft()
{
	if (Cursor.x == tlMargins.x)
	{
		Cursor.x = cursorLimit.x - brMargins.x - 1u;
		CursorUp();
	}
	else
	{
		--Cursor.x;
	}
}

void GraphicText::SetCursorPosition(const uint2& new_pos)
{
	assert(new_pos.x >= tlMargins.x);
	assert(new_pos.y >= tlMargins.y);
	assert(new_pos.x < cursorLimit.x - brMargins.x);
	assert(new_pos.y < cursorLimit.y - brMargins.y);
	Cursor = new_pos;
}

const uint2& GraphicText::GetCursorPosition() const
{
	return Cursor;
}

void GraphicText::SetTextColor(const Color& color)
{
	TextColor = color;
	isUsingTexture = false;
}

const Color& GraphicText::GetTextColor() const
{
	return TextColor;
}

void GraphicText::SetTextScale(unsigned int scale)
{
	assert(CharacterWidth * scale < (gfx.GetWidth(paper) - ((tlMargins.x + brMargins.x) * scale)));
	assert(CharacterHeight * scale < (gfx.GetHeight(paper) - ((tlMargins.y + brMargins.y) * scale)));
	TextScale = scale;
	cursorLimit = { gfx.GetWidth(paper) / (CharacterWidth * TextScale),gfx.GetHeight(paper) / (CharacterHeight * TextScale) };
}

const unsigned int& GraphicText::GetTextScale() const
{
	return TextScale;
}

void GraphicText::SetManualCursor()
{
	autoCursor = false;
}

void GraphicText::SetAutoCursor()
{
	autoCursor = true;
}

const bool& GraphicText::AutoCursorIsOn() const
{
	return autoCursor;
}

void GraphicText::SetTextWrapping()
{
	lineFeed = false;
}

void GraphicText::SetLineFeed()
{
	lineFeed = true;
}

const bool& GraphicText::LineFeedIsEnabled() const
{
	return lineFeed;
}

void GraphicText::SetDoubleSpacing()
{
	lineSpacing = 2u;
}

void GraphicText::SetSingleSpacing()
{
	lineSpacing = 1u;
}

bool GraphicText::IsDoubleSpaced() const
{
	return lineSpacing == 2u;
}

void GraphicText::SetTextTexture(const Image& image)
{
	assert(image.GetWidth() == CharacterWidth);
	assert(image.GetHeight() == CharacterHeight);
	TextTexture = image;
	isUsingTexture = true;
}

const Image& GraphicText::GetTextTexture() const
{
	assert(TextTexture);
	return *TextTexture;
}

void GraphicText::UseTextTexture()
{
	assert(TextTexture);
	isUsingTexture = true;
}

bool GraphicText::isUsingTextTexture() const
{
	return isUsingTexture;
}

void GraphicText::UseTextColor()
{
	isUsingTexture = false;
}

bool GraphicText::isUsingTextColor() const
{
	return !isUsingTexture;
}

void GraphicText::Write(std::string text)
{
	unsigned int Y;
	unsigned int X;
	for (unsigned int i = 0u; i < text.length(); ++i)
	{
		bool isBackspace = false;
		bool isDelete = false;
		if (text[i] == '\b')
		{
			CursorLeft();
			text[i] = ' ';
			isBackspace = true;
		}
		else if (text[i] == 0x7F)
		{
			text[i] = ' ';
			isDelete = true;
		}
		else if (text[i] == '\r')
		{
			CursorDown();
			Cursor.x = tlMargins.x;
			continue;
		}
		assert(text[i] >= startChar);
		assert(text[i] < startChar + charTableDim.x * charTableDim.y);
		text[i] -= startChar;
		Y = Cursor.y * (CharacterHeight * TextScale);
		X = Cursor.x * (CharacterWidth * TextScale);
		for (unsigned int y = 0u; y < CharacterHeight * TextScale; ++y)
		{
			const unsigned int yPxl = (Y + y) * gfx.GetWidth(paper);
			const unsigned int yBit = (y / TextScale) * CharacterWidth;
			for (unsigned int x = 0u; x < CharacterWidth * TextScale; ++x)
			{
				if (isUsingTexture)
				{
					const unsigned int xBit = x / TextScale;
					gfx.GetPixelMap(paper)[yPxl + (X + x)] = (TextTexture->GetPtrToImage()[yBit + xBit] * (float)CharBitmaps[text[i]][yBit + xBit]);
				}
				else
				{
					gfx.GetPixelMap(paper)[yPxl + (X + x)] = (TextColor * (float)CharBitmaps[text[i]][yBit + (x / TextScale)]);
				}
			}
		}
		if (isBackspace)
		{
			continue;
		}
		else
		{
			if (autoCursor && !isDelete)
			{
				CursorRight();
			}
		}
	}
}

void GraphicText::PutChar(const char& chr, uint2 pos)
{
	assert(pos.x >= tlMargins.x);
	assert(pos.y >= tlMargins.y);
	assert(pos.x < cursorLimit.x - brMargins.x);
	assert(pos.y < cursorLimit.y - brMargins.y);
	uint2 cursorStore = Cursor;
	Cursor = pos;
	Write(&chr);
	Cursor = cursorStore;
}

void GraphicText::LineFeedUp()
{
	const unsigned int lineHeight = (CharacterHeight * TextScale);
	unsigned int startY = (tlMargins.y + 1u) * lineHeight;
	const unsigned int startX = tlMargins.x * lineHeight;
	const unsigned int endY = (cursorLimit.y - brMargins.y) * lineHeight;
	const unsigned int endX = (cursorLimit.x - brMargins.x) * lineHeight;
	const unsigned int pitchBytes = (endX - startX) * sizeof(Color);
	for (unsigned int y = startY; y < endY; ++y)
	{
		memcpy(&gfx.GetPixelMap(paper)[(y - lineHeight) * gfx.GetWidth(paper) + startX], &gfx.GetPixelMap(paper)[y * gfx.GetWidth(paper) + startX], pitchBytes);
	}
	startY = endY - lineHeight;
	for (unsigned int y = startY; y < endY; ++y)
	{
		memset(&gfx.GetPixelMap(paper)[y * gfx.GetWidth(paper) + startX], 0u, pitchBytes);
	}
}

void GraphicText::LineFeedDown()
{
	const unsigned int lineHeight = (CharacterHeight * TextScale);
	unsigned int startY = (cursorLimit.y - brMargins.y - 1u) * lineHeight;
	const unsigned int startX = tlMargins.x * lineHeight;
	const unsigned int endY = tlMargins.y * lineHeight;
	const unsigned int endX = (cursorLimit.x - brMargins.x) * lineHeight;
	const unsigned int pitchBytes = (endX - startX) * sizeof(Color);
	for (unsigned int y = startY - 1u; y >= endY; --y)
	{
		memcpy(&gfx.GetPixelMap(paper)[(y + lineHeight) * gfx.GetWidth(paper) + startX], &gfx.GetPixelMap(paper)[y * gfx.GetWidth(paper) + startX], pitchBytes);
	}
	startY = endY + lineHeight;
	for (unsigned int y = startY - 1u; y >= endY; --y)
	{
		memset(&gfx.GetPixelMap(paper)[y * gfx.GetWidth(paper) + startX], 0u, pitchBytes);
	}
}

void GraphicText::ClearText()
{
	const unsigned int lineHeight = (CharacterHeight * TextScale);
	const unsigned int startY = tlMargins.y * lineHeight;
	const unsigned int startX = tlMargins.x * lineHeight;
	const unsigned int endY = (cursorLimit.y - brMargins.y) * lineHeight;
	const unsigned int endX = (cursorLimit.x - brMargins.x) * lineHeight;
	const unsigned int pitchBytes = (endX - startX) * sizeof(Color);
	for (unsigned int y = startY; y < endY; ++y)
	{
		memset(&gfx.GetPixelMap(paper)[y * gfx.GetWidth(paper) + startX], 0u, pitchBytes);
	}
	Cursor = { tlMargins.x,tlMargins.y };
}



