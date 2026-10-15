#include "Font.h"

//#include "SharedConstants.h"
#include "../Options.h"
#include "../renderer/Textures.h"
#include "../renderer/Tesselator.h"
#include "../../util/Mth.h"
#include <cstring>

Font::Font( Options* options, const std::string& name, Textures* textures )
:	options(options),
	fontTexture(0),
	fontName(name),
	index(0),
	count(0),
	_textures(textures),
	_x(0), _y(0),
	_cols(16), _rows(16),
	_charOffset(0),
	lineHeight(DefaultLineHeight)
{
	init(options);
}


//Font::Font( Options* options, const std::string& name, Textures* textures, int imgW, int imgH, int x, int y, int cols, int rows, unsigned char charOffset )
//:	options(options),
//	fontTexture(0),
//	fontName(name),
//	index(0),
//	count(0),
//	_textures(textures),
//	_x(x), _y(y),
//	_cols(cols), _rows(rows),
//	_charOffset(charOffset)
//{
//	init(options);
//}

void Font::onGraphicsReset()
{
	init(options);
}

void Font::init( Options* options )
{
	TextureId fontTexture = _textures->loadTexture(fontName);
	const TextureData* tex = _textures->getTemporaryTextureData(fontTexture);

	if (!tex || !tex->data)
		return;

	unsigned char* rawPixels = tex->data;
	int imgW = tex->w;
	int imgH = tex->h;

	bool usesAlpha = false;
	int totalPixels = imgW * imgH;
	for (int p = 0; p < totalPixels; p++) {
		if (rawPixels[(p << 2) + 3] < 250) {
			usesAlpha = true;
			break;
		}
	}

	int cellW = imgW / _cols;
	int cellH = imgH / _rows;
	if (cellW <= 0) cellW = 8;
	if (cellH <= 0) cellH = 8;

	const int numChars = _rows * _cols;
	for (int i = 0; i < numChars && i < 256; i++) {
		int xt = i % _cols;
		int yt = i / _cols;

		if (i == ' ') {
			charWidths[i] = 4;
			fcharWidths[i] = 4.0f;
			continue;
		}

		int x = cellW - 1;
		for (; x >= 0; x--) {
			int px = _x + xt * cellW + x;
			bool emptyColumn = true;

			for (int y = 0; y < cellH && emptyColumn; y++) {
				int py = _y + yt * cellH + y;
				if (px >= imgW || py >= imgH) continue;

				int idx = (px + py * imgW) << 2;
				unsigned char r = rawPixels[idx + 0];
				unsigned char g = rawPixels[idx + 1];
				unsigned char b = rawPixels[idx + 2];
				unsigned char a = rawPixels[idx + 3];

				if (usesAlpha) {
					if (a > 32) emptyColumn = false;
				} else {
					if (r > 32 || g > 32 || b > 32) emptyColumn = false;
				}
			}

			if (!emptyColumn) {
				break;
			}
		}

		if (x < 0) {
			charWidths[i] = 0;
		} else {
			int w = x + 1;
			if (cellW != 8 && cellW > 0) {
				w = (w * 8) / cellW;
			}
			// Special-case narrow characters like 'f', 'i', 'j', 'l', 't', 'I', '!', '.', ',', etc.
			// Standard Minecraft font widths: 'i', 'l', '!', '.', ',', ';', ':', '\'', '|' are narrower.
			if (i == 'i' || i == 'l' || i == '!' || i == '.' || i == ',' || i == ';' || i == ':' || i == '\'' || i == '|') {
				charWidths[i] = w;
			} else {
				charWidths[i] = w + 1;
			}
		}
		fcharWidths[i] = (float) charWidths[i];
	}

#ifdef USE_VBO
	return; // this <1
#endif

#ifndef USE_VBO
	listPos = glGenLists(256 + 32);

	Tesselator& t = Tesselator::instance;
	for (int i = 0; i < 256; i++) {
		glNewList(listPos + i, GL_COMPILE);
		// @attn @huge @note: This is some dangerous code right here / Aron, added ^1
		t.begin();
		buildChar(i);
		t.end(false, -1);

		glTranslatef2((GLfloat)charWidths[i], 0.0f, 0.0f);
		glEndList();
	}

	for (int i = 0; i < 32; i++) {
		int br = ((i >> 3) & 1) * 0x55;
		int r = ((i >> 2) & 1) * 0xaa + br;
		int g = ((i >> 1) & 1) * 0xaa + br;
		int b = ((i >> 0) & 1) * 0xaa + br;
		if (i == 6) {
			r += 0x55;
		}
		bool darken = i >= 16;

		if (options->anaglyph3d) {
			int cr = (r * 30 + g * 59 + b * 11) / 100;
			int cg = (r * 30 + g * 70) / (100);
			int cb = (r * 30 + b * 70) / (100);

			r = cr;
			g = cg;
			b = cb;
		}

		// color = r << 16 | g << 8 | b;
		if (darken) {
			r /= 4;
			g /= 4;
			b /= 4;
		}

		glNewList(listPos + 256 + i, GL_COMPILE);
		glColor3f(r / 255.0f, g / 255.0f, b / 255.0f);
		glEndList();
	}
#endif
}

void Font::drawShadow( const std::string& str, float x, float y, int color )
{
	draw(str, x + 1, y + 1, color, true);
	draw(str, x, y, color);
}
void Font::drawShadow( const char* str, float x, float y, int color )
{
	draw(str, x + 1, y + 1, color, true);
	draw(str, x, y, color);
}

void Font::draw( const std::string& str, float x, float y, int color )
{
	draw(str, x, y, color, false);
}

void Font::draw( const char* str, float x, float y, int color )
{
	draw(str, x, y, color, false);
}

void Font::draw( const char* str, float x, float y, int color, bool darken )
{
	drawSlow(str, x, y, color, darken);
}

void Font::draw( const std::string& str, float x, float y, int color, bool darken )
{
	drawSlow(str.c_str(), x, y, color, darken);
}

int Font::width( const std::string& str )
{
	int maxLen = 0;
	int len = 0;

	for (unsigned int i = 0; i < str.length(); i++) {
		if (str[i] == '\xa7') {
			i++;
		} else {
			//int ch = SharedConstants.acceptableLetters.indexOf(str.charAt(i));
			//if (ch >= 0) {
			//    len += charWidths[ch + 32];
			//}
			if (str[i] == '\n') {
				if (len > maxLen) maxLen = len;
				len = 0;
			}
			else {
				int charWidth = charWidths[ (unsigned char) str[i] ];
				len += charWidth;
			}
		}
	}
	return maxLen>len? maxLen : len;
}

int Font::height( const std::string& str ) {
	int h = 0;
	bool hasLine = false;
	for (unsigned int i = 0; i < str.length(); ++i) {
		if (str[i] == '\n') hasLine = true;
		else {
			if (hasLine) h += lineHeight;
			hasLine = false;
		}
	}
	return h;
}

std::string Font::sanitize( const std::string& str )
{
	std::string sanitized(str.length() + 1, 0);
	int j = 0;

	for (unsigned int i = 0; i < str.length(); i++) {
		if (str[i] == '\xa7') {
			i++;
			//} else if (SharedConstants.acceptableLetters.indexOf(str.charAt(i)) >= 0) {
		} else {
			sanitized[j++] = str[i];
		}
	}
	return sanitized.erase(j);
}

void Font::drawWordWrap( const std::string& str, float x, float y, float w, int col )
{
	char* cstr = new char[str.length() + 1];
	strncpy(cstr, str.c_str(), str.length());
	cstr[str.length()] = 0;

	const char* lims = " \n\t\r";
	char* ptok = strtok(cstr, lims);

	std::vector<std::string> words;
	while (ptok != NULL) {
		words.push_back( ptok );
		ptok = strtok(NULL, lims);
	}

	delete[] cstr;

	int pos = 0;
	while (pos < (int)words.size()) {
		std::string line = words[pos++] + " ";
		while (pos < (int)words.size() && width(line + words[pos]) < w) {
			line += words[pos++] + " ";
		}
		drawShadow(line, x, y, col);
		y += lineHeight;
	}
}

void Font::drawSlow( const std::string& str, float x, float y, int color, bool darken /*= false*/ ) {
	drawSlow(str.c_str(), x, y, color, darken);
}
void Font::drawSlow( const char* str, float x, float y, int color, bool darken /*= false*/ )
{
	if (!str) return;

	if (darken) {
		int oldAlpha = color & 0xff000000;
		color = (color & 0xfcfcfc) >> 2;
		color += oldAlpha;
	}

	_textures->loadAndBindTexture(fontName);

	Tesselator& t = Tesselator::instance;
	t.begin();
	int alpha = (0xff000000 & color) >> 24;
	if (!alpha) alpha = 0xff;

	static const std::string hex("0123456789abcdef");
	static const int colorCodes[16] = {
		0x000000, 0x0000AA, 0x00AA00, 0x00AAAA,
		0xAA0000, 0xAA00AA, 0xFFAA00, 0xAAAAAA,
		0x555555, 0x5555FF, 0x55FF55, 0x55FFFF,
		0xFF5555, 0xFF55FF, 0xFFFF55, 0xFFFFFF
	};

	int curColor = color;
	int curR = (curColor >> 16) & 0xff;
	int curG = (curColor >> 8) & 0xff;
	int curB = curColor & 0xff;
	t.color(curR, curG, curB, alpha);

	t.addOffset((float)x, (float)y, 0);
	float xOffset = 0;
	float yOffset = 0;

	for (int i = 0; str[i] != '\0'; i++) {
		if (str[i] == '\xa7' && str[i + 1] != '\0') {
			char codeChar = tolower(str[i + 1]);
			int codeIdx = hex.find(codeChar);
			if (codeIdx >= 0 && codeIdx < 16) {
				int cCode = colorCodes[codeIdx];
				if (darken) {
					cCode = (cCode & 0xfcfcfc) >> 2;
				}
				curR = (cCode >> 16) & 0xff;
				curG = (cCode >> 8) & 0xff;
				curB = cCode & 0xff;
				t.color(curR, curG, curB, alpha);
			}
			i++;
			continue;
		}

		unsigned char ch = (unsigned char)str[i];
		if (ch == '\n') {
			xOffset = 0;
			yOffset += lineHeight;
		} else {
			buildChar(ch, xOffset, yOffset);
			xOffset += fcharWidths[ch];
		}
	}
	t.draw();
	t.addOffset(-(float)x, -(float)y, 0);
}

void Font::buildChar( unsigned char i, float x /*= 0*/, float y /*=0*/ )
{
	Tesselator& t = Tesselator::instance;

	float col = (float)(i % _cols);
	float row = (float)(i / _cols);

	float u0 = (col * 8.0f) / 128.0f;
	float v0 = (row * 8.0f) / 128.0f;
	float u1 = u0 + (7.99f / 128.0f);
	float v1 = v0 + (7.99f / 128.0f);

	float sw = 8.0f;
	float sh = 8.0f;

	t.vertexUV(x, y + sh, 0, u0, v1);
	t.vertexUV(x + sw, y + sh, 0, u1, v1);
	t.vertexUV(x + sw, y, 0, u1, v0);
	t.vertexUV(x, y, 0, u0, v0);
}
