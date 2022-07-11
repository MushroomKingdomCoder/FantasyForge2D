#pragma once
#include "Graphics.h"
#include <vector>
#include <memory>
#include <functional>

class Image
{
private:
	unsigned int width = 0u;
	unsigned int height = 0u;
	std::unique_ptr<Color[]> pImage = nullptr;
public:
	Image() = default;
	Image(const Image& image);
	Image& operator =(const Image& image);
	Image(unsigned int width, unsigned int height);
	Image(const char* filename);
	Image(const std::vector<Color>& image, unsigned int image_width);
	unsigned int GetWidth() const;
	unsigned int GetHeight() const;
	const Color* GetPtrToImage() const;
	void SetPixel(unsigned int x, unsigned int y, const Color& color);
	const Color& GetPixel(unsigned int x, unsigned int y) const;
	Image Cropped(unsigned int new_width, unsigned int new_height, unsigned int x_off, unsigned int y_off) const;
	Image& Crop(unsigned int new_width, unsigned int new_height, unsigned int x_off, unsigned int y_off);
	Image FlippedV() const;
	Image& FlipV();
	Image FlippedH() const;
	Image& FlipH();
	Image Rotated90() const;
	Image& Rotate90();
	Image Rotated180() const;
	Image& Rotate180();
	Image Rotated270() const;
	Image& Rotate270();
	Image AdjustedSize(float x_adjust, float y_adjust) const;
	Image& AdjustSize(float x_adjust, float y_adjust);
	Image WithAddedTransparencyFromChroma(const Color& chroma) const;
	Image& AddTransparencyFromChroma(const Color& chroma);
	Image WithInvertedColors() const;
	Image& InvertColors();
	Image WithSubstitutedColors(std::vector<Color> targets, std::vector<Color> replacements) const;
	Image& SubstituteColors(std::vector<Color> targets, std::vector<Color> replacements);
	Image Monochromatic(const Color& color) const;
	Image& MakeMonochromatic(const Color& color);
	Image ColorScaled(const Color& scale) const;
	Image& ColorScale(const Color& scale);
	Image Filtered(const Color& filter) const;
	Image& Filter(const Color& filter);
	Image WithMosaicEffect(uint2 img_divs) const;
	Image& MakeMosaic(uint2 img_divs);
	Image Silhouetted(const Color& background, const Color& silhouette) const;
	Image& Silhouette(const Color& background, const Color& silhouette);
	void Load(const char* filename);
	void Save(const char* filename) const;
	void Import(const std::vector<Color>& image, unsigned int image_width);
	std::vector<Color> Export() const;
	void Draw(Graphics& gfx, int X, int Y, unsigned int layer = 0u) const;
	void Draw(Graphics& gfx, int X, int Y, unsigned int width, unsigned int height, unsigned int layer = 0u) const;
	void Draw(Graphics& gfx, int X, int Y, std::function<Color(const Image&, unsigned int, unsigned int, unsigned int)> color_func, unsigned int layer = 0u) const;
	void Draw(Graphics& gfx, int X, int Y, unsigned int width, unsigned int height, std::function<Color(const Image&, unsigned int, unsigned int, unsigned int)> color_func, unsigned int layer = 0u) const;
	void DrawWithTransparency(Graphics& gfx, int X, int Y, unsigned int layer = 0u) const;
	void DrawWithTransparency(Graphics& gfx, int X, int Y, unsigned int width, unsigned int height, unsigned int layer = 0u) const;
	void DrawWithTransparency(Graphics& gfx, int X, int Y, std::function<Color(const Image&, unsigned int, unsigned int, unsigned int)> color_func, unsigned int layer = 0u) const;
	void DrawWithTransparency(Graphics& gfx, int X, int Y, unsigned int width, unsigned int height, std::function<Color(const Image&, unsigned int, unsigned int, unsigned int)> color_func, unsigned int layer = 0u) const;
};

namespace ImageEffects
{
	Color InvertColors(const Image& image, unsigned int img_x, unsigned int img_y, unsigned int img_pxl);
	Color GreyScale(const Image& image, unsigned int img_x, unsigned int img_y, unsigned int img_pxl);
	Color White(const Image& image, unsigned int img_x, unsigned int img_y, unsigned int img_pxl);
	Color Black(const Image& image, unsigned int img_x, unsigned int img_y, unsigned int img_pxl);
	Color FlipH(const Image& image, unsigned int img_x, unsigned int img_y, unsigned int img_pxl);
	Color FlipV(const Image& image, unsigned int img_x, unsigned int img_y, unsigned int img_pxl);
	Color FlipHV(const Image& image, unsigned int img_x, unsigned int img_y, unsigned int img_pxl);
}

