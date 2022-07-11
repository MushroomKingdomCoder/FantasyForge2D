#pragma once
#include "Image.h"
#include "Vector.h"
#include <vector>
#include <optional>

class GraphicText
{
private:
	Graphics& gfx;
	unsigned int paper;
	const unsigned char CharacterWidth;
	const unsigned char CharacterHeight;
	std::vector<std::vector<bool>> CharBitmaps;
	const unsigned char startChar;
	const uchar2 charTableDim;
	uint2 cursorLimit;
	uint2 tlMargins;
	uint2 brMargins;
	uint2 Cursor;
	Color TextColor;
	unsigned int TextScale;
	bool autoCursor;
	bool lineFeed;
	unsigned int lineSpacing;
	std::optional<Image> TextTexture;
	bool isUsingTexture;
public:
	GraphicText() = delete;
	GraphicText(Graphics& gfx, unsigned int layer = 0u);
	GraphicText(Graphics& gfx, unsigned int layer, Image charset, uchar2 charTableDim, unsigned char startChar, uint2 cursor_pos = { 0u,0u }, Color txtColor = Colors::White, unsigned int txtScale = 1u, bool double_spaced = false, bool autoCursor = true, bool lineFeed = true, uint2 tl_margins = { 0u,0u }, uint2 br_margins = { 0u,0u }, std::optional<Image> txtTexture = std::optional<Image>());
	void SetTopLeftMargins(const uint2& tl_margins);
	void SetBottomRightMargins(const uint2& br_margins);
	const uint2& GetTopLeftMargins() const;
	const uint2& GetBottomRightMargins() const;
	void CursorUp();
	void CursorRight();
	void CursorDown();
	void CursorLeft();
	void SetCursorPosition(const uint2& new_pos);
	const uint2& GetCursorPosition() const;
	void SetTextColor(const Color& color);
	const Color& GetTextColor() const;
	void SetTextScale(unsigned int scale);
	const unsigned int& GetTextScale() const;
	void SetManualCursor();
	void SetAutoCursor();
	const bool& AutoCursorIsOn() const;
	void SetTextWrapping();
	void SetLineFeed();
	const bool& LineFeedIsEnabled() const;
	void SetDoubleSpacing();
	void SetSingleSpacing();
	bool IsDoubleSpaced() const;
	void SetTextTexture(const Image& image);
	const Image& GetTextTexture() const;
	void UseTextTexture();
	bool isUsingTextTexture() const;
	void UseTextColor();
	bool isUsingTextColor() const;
	void Write(std::string text);
	void PutChar(const char& chr, uint2 pos);
	void LineFeedUp();
	void LineFeedDown();
	void ClearText();
};



