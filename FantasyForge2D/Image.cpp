#include "Image.h"
#include <fstream>
#include "BaseException.h"
#include <wingdi.h>
#include <assert.h>

Image::Image(const Image& image)
	:
	width(image.width),
	height(image.height),
	pImage(std::make_unique<Color[]>(width * height))
{
	const unsigned int nImageBytes = width * height * sizeof(Color);
	memcpy(pImage.get(), image.pImage.get(), nImageBytes);
}

Image& Image::operator=(const Image& image)
{
	width = image.width;
	height = image.height;
	const unsigned int nPixels = width * height;
	pImage = std::make_unique<Color[]>(nPixels);
	const unsigned int nImageBytes = nPixels * sizeof(Color);
	memcpy(pImage.get(), image.pImage.get(), nImageBytes);
	return *this;
}

Image::Image(unsigned int width, unsigned int height)
	:
	width(width),
	height(height),
	pImage(std::make_unique<Color[]>(width * height))
{}

Image::Image(const char* filename)
{
	std::ifstream bitmapIN{ filename, std::ios::binary };
	if (bitmapIN.fail())
	{
		throw EXCPT_NOTE("Bitmap file not found! Check directory and/or file name spelling and retry.");
	}
	BITMAPFILEHEADER fileHead = {};
	bitmapIN.read(reinterpret_cast<char*>(&fileHead), sizeof(fileHead));
	if (fileHead.bfType != ('B' + ('M' << 8)))
	{
		throw EXCPT_NOTE("Only .bmp image files supported! Check image file extension and retry.");
	}
	BITMAPINFOHEADER infoHead = {};
	bitmapIN.read(reinterpret_cast<char*>(&infoHead), sizeof(infoHead));
	if (infoHead.biCompression != BI_RGB)
	{
		throw EXCPT_NOTE("Only uncompressed bitmaps supported! Decompress image and retry.");
	}
	if (infoHead.biBitCount != 24 && infoHead.biBitCount != 32)
	{
		throw EXCPT_NOTE("Only bitmaps of either 24bpp or 32bpp allowed. Reset color depth of image and retry.");
	}
	bitmapIN.seekg(fileHead.bfOffBits, std::ios::beg);
	width = infoHead.biWidth;
	height = std::abs(infoHead.biHeight);
	const unsigned int nPixels = width * height;
	const unsigned int nImageBytes = nPixels * sizeof(Color);
	pImage = std::make_unique<Color[]>(nPixels);
	if (infoHead.biBitCount == 32 && infoHead.biHeight < 0)
	{
		bitmapIN.read(reinterpret_cast<char*>(pImage.get()), nImageBytes);
	}
	else
	{
		unsigned int padding = 0u;
		if (infoHead.biBitCount == 24)
		{
			padding = (4u - ((width * 3u) % 4u)) % 4u;
		}
		int startY = 0u;
		int deltaY = 1u;
		int endY = height;
		if (infoHead.biHeight > 0)
		{
			startY = height - 1;
			deltaY = -1;
			endY = -1;
		}
		for (int y = startY; y != endY; y += deltaY)
		{
			for (unsigned int x = 0; x < width; ++x)
			{
				const unsigned int pxl = y * width + x;
				if (infoHead.biBitCount == 24)
				{
					bitmapIN.read(reinterpret_cast<char*>(&pImage[pxl]), sizeof(Color) - 1u);
					pImage[pxl].SetA(255u);
				}
				else
				{
					bitmapIN.read(reinterpret_cast<char*>(&pImage[pxl]), sizeof(Color));
				}
			}
			bitmapIN.seekg(padding, std::ios::cur);
		}
	}
	if (bitmapIN.fail())
	{
		throw EXCPT_NOTE("Critical error in reading bitmap file! Please retry.");
	}
	bitmapIN.close();
}

Image::Image(const std::vector<Color>& image, unsigned int image_width)
	:
	width(image_width),
	height((unsigned int)image.size() / width),
	pImage(std::make_unique<Color[]>(image.size()))
{
	const unsigned int nImageBytes = (unsigned int)image.size() * sizeof(Color);
	memcpy(pImage.get(), image.data(), nImageBytes);
}

unsigned int Image::GetWidth() const
{
	return width;
}

unsigned int Image::GetHeight() const
{
	return height;
}

const Color* Image::GetPtrToImage() const
{
	return pImage.get();
}

void Image::SetPixel(unsigned int x, unsigned int y, const Color& color)
{
	assert(x < width && y < height);
	const unsigned int pxl = y * width + x;
	pImage[pxl] = color;
}

const Color& Image::GetPixel(unsigned int x, unsigned int y) const
{
	assert(x < width && y < height);
	const unsigned int pxl = y * width + x;
	return pImage[pxl];
}

Image Image::Cropped(unsigned int new_width, unsigned int new_height, unsigned int x_off, unsigned int y_off) const
{
	assert(new_width + x_off <= width && new_width > 0u);
	assert(new_height + y_off <= height && new_height > 0u);
	Image image{ new_width, new_height };
	const unsigned int pitch = new_width * sizeof(Color);
	for (unsigned int y = 0; y < new_height; ++y)
	{
		const unsigned int dst_pxl = y * new_width;
		const unsigned int src_pxl = (y + y_off) * width + (x_off);
		memcpy(&image.pImage[dst_pxl], &pImage[src_pxl], pitch);
	}
	return image;
}

Image& Image::Crop(unsigned int new_width, unsigned int new_height, unsigned int x_off, unsigned int y_off)
{
	return *this = this->Cropped(new_width, new_height, x_off, y_off);
}

Image Image::FlippedV() const
{
	Image flippedV{ width,height };
	for (unsigned int y = 0; y < height; ++y)
	{
		for (unsigned int x = 0; x < width; ++x)
		{
			const unsigned int dst_pxl = y * width + x;
			const unsigned int src_pxl = (height - y - 1) * width + x;
			flippedV.pImage[dst_pxl] = pImage[src_pxl];
		}
	}
	return flippedV;
}

Image& Image::FlipV()
{
	return *this = this->FlippedV();
}

Image Image::FlippedH() const
{
	Image flippedH{ width,height };
	for (unsigned int y = 0; y < height; ++y)
	{
		for (unsigned int x = 0; x < width; ++x)
		{
			const unsigned int dst_pxl = y * width + x;
			const unsigned int src_pxl = y * width + (width - x - 1);
			flippedH.pImage[dst_pxl] = pImage[src_pxl];
		}
	}
	return flippedH;
}

Image& Image::FlipH()
{
	return *this = this->FlippedH();
}

Image Image::Rotated90() const
{
	Image rotated{ height,width };
	for (unsigned int y = 0; y < height; ++y)
	{
		for (unsigned int x = 0; x < width; ++x)
		{
			const unsigned int src_pxl = y * width + x;
			const unsigned int dst_pxl = (width - x - 1) * height + y;
			rotated.pImage[dst_pxl] = pImage[src_pxl];
		}
	}
	return rotated;
}

Image& Image::Rotate90()
{
	return *this = this->Rotated90();
}

Image Image::Rotated180() const
{
	Image rotated{ width,height };
	for (unsigned int y = 0; y < height; ++y)
	{
		for (unsigned int x = 0; x < width; ++x)
		{
			const unsigned int src_pxl = y * width + x;
			const unsigned int dst_pxl = (height - y - 1) * width + (width - x - 1);
			rotated.pImage[dst_pxl] = pImage[src_pxl];
		}
	}
	return rotated;
}

Image& Image::Rotate180()
{
	return *this = this->Rotated180();
}

Image Image::Rotated270() const
{
	Image rotated{ height,width };
	for (unsigned int y = 0; y < height; ++y)
	{
		for (unsigned int x = 0; x < width; ++x)
		{
			const unsigned int src_pxl = y * width + x;
			const unsigned int dst_pxl = x * height + (height - y - 1);
			rotated.pImage[dst_pxl] = pImage[src_pxl];
		}
	}
	return rotated;
}

Image& Image::Rotate270()
{
	return *this = this->Rotated270();
}

Image Image::AdjustedSize(float x_adjust, float y_adjust) const
{
	float newWidth = (float)width * x_adjust;
	float newHeight = (float)height * y_adjust;
	Image adjusted{ unsigned int(newWidth), unsigned int(newHeight) };
	const float xPxlsPerPxl = (float)width / newWidth;
	const float yPxlsPerPxl = (float)height / newHeight;
	for (unsigned int y = 0u; y < adjusted.height; ++y)
	{
		for (unsigned int x = 0u; x < adjusted.height; ++x)
		{
			adjusted.pImage[y * adjusted.width + x] = pImage[unsigned int((float)y * yPxlsPerPxl) * width + unsigned int((float)x * xPxlsPerPxl)];
		}
	}
	return adjusted;
}

Image& Image::AdjustSize(float x_adjust, float y_adjust)
{
	return *this = this->AdjustedSize(x_adjust, y_adjust);
}

Image Image::WithAddedTransparencyFromChroma(const Color& chroma) const
{
	Image transparentCopy{ width,height };
	for (unsigned int y = 0u; y < height; ++y)
	{
		for (unsigned int x = 0u; x < width; ++x)
		{
			const unsigned int pxl = y * width + x;
			if (pImage[pxl] == chroma)
			{
				transparentCopy.pImage[pxl] = Colors::Transparent;
			}
			else
			{
				transparentCopy.pImage[pxl] = pImage[pxl];
			}
		}
	}
	return transparentCopy;
}

Image& Image::AddTransparencyFromChroma(const Color& chroma)
{
	return *this = this->WithAddedTransparencyFromChroma(chroma);
}

Image Image::WithInvertedColors() const
{
	Image inverted{ width,height };
	for (unsigned int i = 0u; i < width * height; ++i)
	{
		if (pImage[i].GetA())
		{
			inverted.pImage[i] = pImage[i].Inverted();
		}
		else
		{
			inverted.pImage[i] = Colors::Transparent;
		}
	}
	return inverted;
}

Image& Image::InvertColors()
{
	return *this = this->WithInvertedColors();
}

Image Image::WithSubstitutedColors(std::vector<Color> targets, std::vector<Color> replacements) const
{
	assert(targets.size() <= replacements.size());
	Image substituted{ width,height };
	for (unsigned int i = 0u; i < width * height; ++i)
	{
		for (unsigned int j = 0u; j < targets.size(); ++j)
		{
			if (pImage[i] == targets[j])
			{
				substituted.pImage[i] = replacements[j];
			}
			else
			{
				substituted.pImage[i] = pImage[i];
			}
		}
	}
	return substituted;
}

Image& Image::SubstituteColors(std::vector<Color> targets, std::vector<Color> replacements)
{
	return *this = this->WithSubstitutedColors(targets, replacements);
}

Image Image::Monochromatic(const Color& color) const
{
	Image monochromatic{ width,height };
	for (unsigned int i = 0u; i < width * height; ++i)
	{
		if (pImage[i].GetA())
		{
			monochromatic.pImage[i] = color;
		}
		else
		{
			monochromatic.pImage[i] = Colors::Transparent;
		}
	}
	return monochromatic;
}

Image& Image::MakeMonochromatic(const Color& color)
{
	return *this = this->Monochromatic(color);
}

Image Image::ColorScaled(const Color& scale) const
{
	Image scaled{ width,height };
	for (unsigned int i = 0u; i < width * height; ++i)
	{
		if (pImage[i].GetA())
		{
			const float pxl_avg = (pImage[i].GetRn() + pImage[i].GetGn() + pImage[i].GetBn()) / 3.0f;
			scaled.pImage[i] = Color(pxl_avg * scale.GetRn(), pxl_avg * scale.GetGn(), pxl_avg * scale.GetBn());
		}
		else
		{
			scaled.pImage[i] = Colors::Transparent;
		}
	}
	return scaled;
}

Image& Image::ColorScale(const Color& scale)
{
	return *this = this->ColorScaled(scale);
}

Image Image::Filtered(const Color& filter) const
{
	Image filtered{ width,height };
	vec4 vFilter = filter.GetVector();
	for (unsigned int i = 0u; i < width * height; ++i)
	{
		filtered.pImage[i] = pImage[i] * vFilter;
	}
	return filtered;
}

Image& Image::Filter(const Color& filter)
{
	return *this = this->Filtered(filter);
}

Image Image::WithMosaicEffect(uint2 img_divs) const
{
	assert(height % img_divs.y == 0u);
	assert(width % img_divs.x == 0u);
	Image mosaic{ width,height };
	const unsigned int pxlsPerDivY = height / img_divs.y;
	const unsigned int pxlsPerDivX = width / img_divs.x;
	for (unsigned int y = 0u; y < height; ++y)
	{
		const unsigned int row = y * width;
		const unsigned int mos_y = y / pxlsPerDivY * pxlsPerDivY;
		const unsigned int mos_row = mos_y * width;
		for (unsigned int x = 0u; x < width; ++x)
		{
			const unsigned int mos_x = x / pxlsPerDivX * pxlsPerDivX;
			mosaic.pImage[row + x] = pImage[mos_row + mos_x];
		}
	}
	return mosaic;
}

Image& Image::MakeMosaic(uint2 img_divs)
{
	return *this = this->WithMosaicEffect(img_divs);
}

Image Image::Silhouetted(const Color& background, const Color& silhouette) const
{
	Image silhouetted{ width,height };
	for (unsigned int i = 0u; i < width * height; ++i)
	{
		if (pImage[i].GetA())
		{
			if (pImage[i] != background)
			{
				silhouetted.pImage[i] = silhouette;
			}
			else
			{
				silhouetted.pImage[i] = background;
			}
		}
		else
		{
			silhouetted.pImage[i] = Colors::Transparent;
		}
	}
	return silhouetted;
}

Image& Image::Silhouette(const Color& background, const Color& silhouette)
{
	return *this = this->Silhouetted(background, silhouette);
}

void Image::Load(const char* filename)
{
	*this = Image(filename);
}

void Image::Save(const char* filename) const
{
	const unsigned int nPixels = width * height;
	const unsigned int nImageBytes = nPixels * sizeof(Color);
	const unsigned int headerSectionSize = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);
	std::ofstream bitmapOUT{ filename, std::ios::binary };
	if (bitmapOUT.fail())
	{
		throw EXCPT_NOTE("Cannot write to specified file! Check directory and/or file name spelling and retry.");
	}
	BITMAPFILEHEADER fileHead;
	fileHead.bfType = 'B' + ('M' << 8);
	fileHead.bfSize = headerSectionSize + nImageBytes;
	fileHead.bfReserved1 = 0;
	fileHead.bfReserved2 = 0;
	fileHead.bfOffBits = headerSectionSize;
	BITMAPINFOHEADER infoHead;
	infoHead.biSize = sizeof(BITMAPINFOHEADER);
	infoHead.biWidth = width;
	infoHead.biHeight = -int(height);	// Top-down DIB
	infoHead.biPlanes = 1;				// Always set to 1
	infoHead.biBitCount = 32;
	infoHead.biCompression = BI_RGB; 
	infoHead.biSizeImage = nImageBytes;
	infoHead.biXPelsPerMeter = 0;		// No target device
	infoHead.biYPelsPerMeter = 0;		// No target device
	infoHead.biClrUsed = 0;				// Use all possible colors
	infoHead.biClrImportant = 0;		// All colors are required
	bitmapOUT.write(reinterpret_cast<char*>(&fileHead), sizeof(BITMAPFILEHEADER));
	bitmapOUT.write(reinterpret_cast<char*>(&infoHead), sizeof(BITMAPINFOHEADER));
	bitmapOUT.write(reinterpret_cast<char*>(pImage.get()), nImageBytes);
	if (bitmapOUT.fail())
	{
		throw EXCPT_NOTE("Critical error writing bitmap file! Please retry.");
	}
	bitmapOUT.close();
}

void Image::Import(const std::vector<Color>& image, unsigned int image_width)
{
	*this = Image(image, image_width);
}

std::vector<Color> Image::Export() const
{
	std::vector<Color> image;
	const unsigned int nPixels = width * height;
	image.resize(nPixels);
	const unsigned int nImageBytes = nPixels * sizeof(Color);
	memcpy(image.data(), pImage.get(), nImageBytes);
	return image;
}

void Image::Draw(Graphics& gfx, int X, int Y, unsigned int layer) const
{
	const unsigned int& xRes = gfx.GetWidth(layer);
	const unsigned int& yRes = gfx.GetHeight(layer);
	assert(X < (int)xRes && X + width > 0);
	assert(Y < (int)yRes && Y + height > 0);
	const unsigned int startX =
		(0u) * (X >= 0) +
		(-X) * (X < 0);
	const unsigned int startY =
		(0u) * (Y >= 0) +
		(-Y) * (Y < 0);
	const unsigned int slicePitch =
		((xRes - X - startX) * sizeof(Color)) * (width + X > xRes) +
		((width - startX) * sizeof(Color)) * (width + X <= xRes);
	const unsigned int endY =
		(yRes - Y) * (height + Y > yRes) +
		(height) * (height + Y <= yRes);
	for (unsigned int y = startY; y < endY; ++y)
	{
		const unsigned int dst_pxl = (Y + y) * xRes + X + startX;
		const unsigned int src_pxl = y * width + startX;
		memcpy(&gfx.GetPixelMap(layer)[dst_pxl], &pImage[src_pxl], slicePitch);
	}
}

void Image::Draw(Graphics& gfx, int X, int Y, unsigned int width, unsigned int height, unsigned int layer) const
{
	const unsigned int& xRes = gfx.GetWidth(layer);
	const unsigned int& yRes = gfx.GetHeight(layer);
	assert(width != 0u && height != 0u);
	assert(X < (int)xRes && X + width > 0);
	assert(Y < (int)yRes && Y + height > 0);
	const float xPxlsPerPxl = (float)this->width / (float)width;
	const float yPxlsPerPxl = (float)this->height / (float)height;
	const unsigned int startX =
		(0u) * (X >= 0) +
		(-X) * (X < 0);
	const unsigned int startY =
		(0u) * (Y >= 0) +
		(-Y) * (Y < 0);
	const unsigned int endX =
		(xRes - X) * (width + X > xRes) +
		(width) * (width + X <= xRes);
	const unsigned int endY =
		(yRes - Y) * (height + Y > yRes) +
		(height) * (height + Y <= yRes);
	for (unsigned int y = startY; y < endY; ++y)
	{
		for (unsigned int x = startX; x < endX; ++x)
		{
			const unsigned int dest_pxl = (Y + y) * xRes + X + x;
			const unsigned int src_pxl = unsigned int((float)y * yPxlsPerPxl) * this->width + unsigned int((float)x * xPxlsPerPxl);
			gfx.GetPixelMap(layer)[dest_pxl] = pImage[src_pxl];
		}
	}
}

void Image::Draw(Graphics& gfx, int X, int Y, std::function<Color(const Image&, unsigned int, unsigned int, unsigned int)> color_func, unsigned int layer) const
{
	const unsigned int& xRes = gfx.GetWidth(layer);
	const unsigned int& yRes = gfx.GetHeight(layer);
	assert(X < (int)xRes && X + width > 0);
	assert(Y < (int)yRes && Y + height > 0);
	const unsigned int startX =
		(0u) * (X >= 0) +
		(-X) * (X < 0);
	const unsigned int startY =
		(0u) * (Y >= 0) +
		(-Y) * (Y < 0);
	const unsigned int endX =
		(xRes - X) * (width + X > xRes) +
		(width) * (height + X <= xRes);
	const unsigned int endY =
		(yRes - Y) * (height + Y > yRes) +
		(height) * (height + Y <= yRes);
	for (unsigned int y = startY; y < endY; ++y)
	{
		for (unsigned int x = startX; x < endX; ++x)
		{
			const unsigned int src_pxl = y * width + x;
			const unsigned int dest_pxl = (Y + y) * xRes + X + x;
			gfx.GetPixelMap(layer)[dest_pxl] = color_func(*this, x, y, src_pxl);
		}
	}
}

void Image::Draw(Graphics& gfx, int X, int Y, unsigned int width, unsigned int height, std::function<Color(const Image&, unsigned int, unsigned int, unsigned int)> color_func, unsigned int layer) const
{
	const unsigned int& xRes = gfx.GetWidth(layer);
	const unsigned int& yRes = gfx.GetHeight(layer);
	assert(width != 0u && height != 0u);
	assert(X < (int)xRes && X + width > 0);
	assert(Y < (int)yRes && Y + height > 0);
	const float xPxlsPerPxl = (float)this->width / (float)width;
	const float yPxlsPerPxl = (float)this->height / (float)height;
	const unsigned int startX =
		(0u) * (X >= 0) +
		(-X) * (X < 0);
	const unsigned int startY =
		(0u) * (Y >= 0) +
		(-Y) * (Y < 0);
	const unsigned int endX =
		(xRes - X) * (width + X > xRes) +
		(width) * (width + X <= xRes);
	const unsigned int endY =
		(yRes - Y) * (height + Y > yRes) +
		(height) * (height + Y <= yRes);
	for (unsigned int y = startY; y < endY; ++y)
	{
		for (unsigned int x = startX; x < endX; ++x)
		{
			const unsigned int dest_pxl = (Y + y) * xRes + X + x;
			const unsigned int src_x = unsigned int((float)x * xPxlsPerPxl);
			const unsigned int src_y = unsigned int((float)y * yPxlsPerPxl);
			const unsigned int src_pxl = src_y * this->width + src_x;
			gfx.GetPixelMap(layer)[dest_pxl] = color_func(*this, src_x, src_y, src_pxl);
		}
	}
}

void Image::DrawWithTransparency(Graphics& gfx, int X, int Y, unsigned int layer) const
{
	const unsigned int& xRes = gfx.GetWidth(layer);
	const unsigned int& yRes = gfx.GetHeight(layer);
	assert(X < (int)xRes && X + width > 0);
	assert(Y < (int)yRes && Y + height > 0);
	const unsigned int startX =
		(0u) * (X >= 0) +
		(-X) * (X < 0);
	const unsigned int startY =
		(0u) * (Y >= 0) +
		(-Y) * (Y < 0);
	const unsigned int endX =
		(xRes - X) * (width + X > xRes) +
		(width) * (height + X <= xRes);
	const unsigned int endY =
		(yRes - Y) * (height + Y > yRes) +
		(height) * (height + Y <= yRes);
	for (unsigned int y = startY; y < endY; ++y)
	{
		for (unsigned int x = startX; x < endX; ++x)
		{
			const unsigned int src_pxl = y * width + x;
			if (pImage[src_pxl].GetA())
			{
				const unsigned int dest_pxl = (Y + y) * xRes + X + x;
				gfx.GetPixelMap(layer)[dest_pxl] = pImage[src_pxl];
			}
		}
	}
}

void Image::DrawWithTransparency(Graphics& gfx, int X, int Y, unsigned int width, unsigned int height, unsigned int layer) const
{
	const unsigned int& xRes = gfx.GetWidth(layer);
	const unsigned int& yRes = gfx.GetHeight(layer);
	assert(width != 0u && height != 0u);
	assert(X < (int)xRes && X + width > 0);
	assert(Y < (int)yRes && Y + height > 0);
	const float xPxlsPerPxl = (float)this->width / (float)width;
	const float yPxlsPerPxl = (float)this->height / (float)height;
	const unsigned int startX =
		(0u) * (X >= 0) +
		(-X) * (X < 0);
	const unsigned int startY =
		(0u) * (Y >= 0) +
		(-Y) * (Y < 0);
	const unsigned int endX =
		(xRes - X) * (width + X > xRes) +
		(width) * (width + X <= xRes);
	const unsigned int endY =
		(yRes - Y) * (height + Y > yRes) +
		(height) * (height + Y <= yRes);
	for (unsigned int y = startY; y < endY; ++y)
	{
		for (unsigned int x = startX; x < endX; ++x)
		{
			const unsigned int dest_pxl = (Y + y) * xRes + X + x;
			const unsigned int src_pxl = unsigned int((float)y * yPxlsPerPxl) * this->width + unsigned int((float)x * xPxlsPerPxl);
			if (pImage[src_pxl].GetA())
			{
				gfx.GetPixelMap(layer)[dest_pxl] = pImage[src_pxl];
			}
		}
	}
}

void Image::DrawWithTransparency(Graphics& gfx, int X, int Y, std::function<Color(const Image&, unsigned int, unsigned int, unsigned int)> color_func, unsigned int layer) const
{
	const unsigned int& xRes = gfx.GetWidth(layer);
	const unsigned int& yRes = gfx.GetHeight(layer);
	assert(X < (int)xRes && X + width > 0);
	assert(Y < (int)yRes && Y + height > 0);
	const unsigned int startX =
		(0u) * (X >= 0) +
		(-X) * (X < 0);
	const unsigned int startY =
		(0u) * (Y >= 0) +
		(-Y) * (Y < 0);
	const unsigned int endX =
		(xRes - X) * (width + X > xRes) +
		(width) * (height + X <= xRes);
	const unsigned int endY =
		(yRes - Y) * (height + Y > yRes) +
		(height) * (height + Y <= yRes);
	for (unsigned int y = startY; y < endY; ++y)
	{
		for (unsigned int x = startX; x < endX; ++x)
		{
			const unsigned int src_pxl = y * width + x;
			if (pImage[src_pxl].GetA())
			{
				const unsigned int dest_pxl = (Y + y) * xRes + X + x;
				gfx.GetPixelMap(layer)[dest_pxl] = color_func(*this, x, y, src_pxl);
			}
		}
	}
}

void Image::DrawWithTransparency(Graphics& gfx, int X, int Y, unsigned int width, unsigned int height, std::function<Color(const Image&, unsigned int, unsigned int, unsigned int)> color_func, unsigned int layer) const
{
	const unsigned int& xRes = gfx.GetWidth(layer);
	const unsigned int& yRes = gfx.GetHeight(layer);
	assert(width != 0u && height != 0u);
	assert(X < (int)xRes && X + width > 0);
	assert(Y < (int)yRes && Y + height > 0);
	const float xPxlsPerPxl = (float)this->width / (float)width;
	const float yPxlsPerPxl = (float)this->height / (float)height;
	const unsigned int startX =
		(0u) * (X >= 0) +
		(-X) * (X < 0);
	const unsigned int startY =
		(0u) * (Y >= 0) +
		(-Y) * (Y < 0);
	const unsigned int endX =
		(xRes - X) * (width + X > xRes) +
		(width) * (width + X <= xRes);
	const unsigned int endY =
		(yRes - Y) * (height + Y > yRes) +
		(height) * (height + Y <= yRes);
	for (unsigned int y = startY; y < endY; ++y)
	{
		for (unsigned int x = startX; x < endX; ++x)
		{
			const unsigned int dest_pxl = (Y + y) * xRes + X + x;
			const unsigned int src_x = unsigned int((float)x * xPxlsPerPxl);
			const unsigned int src_y = unsigned int((float)y * yPxlsPerPxl);
			const unsigned int src_pxl = src_y * this->width + src_x;
			if (pImage[src_pxl].GetA())
			{
				gfx.GetPixelMap(layer)[dest_pxl] = color_func(*this, src_x, src_y, src_pxl);
			}
		}
	}
}

Color ImageEffects::InvertColors(const Image& image, unsigned int img_x, unsigned int img_y, unsigned int img_pxl)
{
	return image.GetPtrToImage()[img_pxl].Inverted();
}

Color ImageEffects::GreyScale(const Image& image, unsigned int img_x, unsigned int img_y, unsigned int img_pxl)
{
	const Color& pxl = image.GetPtrToImage()[img_pxl];
	const unsigned char scale = unsigned char(((unsigned int)pxl.GetR() + (unsigned int)pxl.GetG() + (unsigned int)pxl.GetB()) / 3u);
	return Color(scale, scale, scale, pxl.GetA());
}

Color ImageEffects::White(const Image& image, unsigned int img_x, unsigned int img_y, unsigned int img_pxl)
{
	return Colors::White;
}

Color ImageEffects::Black(const Image& image, unsigned int img_x, unsigned int img_y, unsigned int img_pxl)
{
	return Colors::Black;
}

Color ImageEffects::FlipH(const Image& image, unsigned int img_x, unsigned int img_y, unsigned int img_pxl)
{
	return image.GetPixel(image.GetWidth() - img_x - 1u, img_y);
}

Color ImageEffects::FlipV(const Image& image, unsigned int img_x, unsigned int img_y, unsigned int img_pxl)
{
	return image.GetPixel(img_x, image.GetHeight() - img_y - 1u);
}

Color ImageEffects::FlipHV(const Image& image, unsigned int img_x, unsigned int img_y, unsigned int img_pxl)
{
	return image.GetPixel(image.GetWidth() - img_x - 1u, image.GetHeight() - img_y - 1u);
}