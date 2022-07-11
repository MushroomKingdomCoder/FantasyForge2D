#pragma once
#include "Image.h"

class Animation
{
private:
	std::vector<Image> frames;
	unsigned int currentFrame;
	const unsigned int frameWidth;
	const unsigned int frameHeight;
	const unsigned int nFrames;
	float currentFrameTime;
	float secsPerFrame;
public:
	Animation() = delete;
	Animation(Image sprite_sheet, uint2 sprite_size, uint2 sheet_dim, unsigned int fps);
	const unsigned int& GetFrameWidth() const;
	const unsigned int& GetFrameHeight() const;
	vec2u GetFrameSize() const;
	const unsigned int& GetCurrentFrameIndex() const;
	void SetCurrentFrameIndex(unsigned int frame);
	const float& GetCurrentFrameTime() const;
	void SetCurrentFrameTime(float time);
	float GetFPS() const;
	void SetFPS(unsigned int fps);
	const Image& GetCurrentFrame() const;
	const Image& Play(float time_ellapsed);
	bool PlayAndCheck(float time_ellapsed);
	void Draw(Graphics& gfx, int x, int y, unsigned int layer = 0u) const;
	void DrawWithTransparency(Graphics& gfx, int x, int y, unsigned int layer = 0u) const;
};



