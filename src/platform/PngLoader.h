#ifndef PNGLOADER_H__
#define PNGLOADER_H__

#include "../client/renderer/TextureData.h"

#include <cstddef>

#if !defined(ANDROID) && !defined(__APPLE__) && !defined(TARGET_OS_IPHONE)
/// Decode a PNG (from memory) into a TextureData.
/// Returns an empty TextureData on failure.
TextureData loadPngFromMemory(const unsigned char* data, size_t size);
#endif

#endif // PNGLOADER_H__
