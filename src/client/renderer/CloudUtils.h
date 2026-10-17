#ifndef NET_MINECRAFT_CLIENT_RENDERER__CloudUtils_H__
#define NET_MINECRAFT_CLIENT_RENDERER__CloudUtils_H__

#include <algorithm>

inline float cloudAlpha(float ny, float rel, float baseAlpha, float opacity, float fadeStrength) {
    // ny: 0 at bottom of cloud, 1 at top
    // rel: layerBaseY minus cameraY in blocks (positive = clouds above camera)
    float relClamped = rel / 2.5f;
    if (relClamped < -1.0f) relClamped = -1.0f;
    if (relClamped > 1.0f) relClamped = 1.0f;

    float m = (relClamped + 1.0f) / 2.0f;
    float fade = (1.0f - ny) * (1.0f - m) + ny * m;

    float alpha_faded = baseAlpha * (opacity - fade);
    if (alpha_faded < 0.0f) alpha_faded = 0.0f;

    float alpha_unfaded = baseAlpha * opacity;
    if (alpha_unfaded < 0.0f) alpha_unfaded = 0.0f;

    float alpha = alpha_unfaded + (alpha_faded - alpha_unfaded) * fadeStrength;
    if (alpha < 0.0f) alpha = 0.0f;
    return alpha;
}

struct CloudDrawnFaces {
    bool drawTop;
    bool drawBottom;
    bool drawSides;
};

inline CloudDrawnFaces getCloudFacesDrawn(float cameraY, float cloudBaseY, float cloudThickness) {
    CloudDrawnFaces faces;
    faces.drawSides = true;
    faces.drawTop = (cameraY > cloudBaseY);
    faces.drawBottom = (cameraY < cloudBaseY + cloudThickness);
    return faces;
}

#endif /* NET_MINECRAFT_CLIENT_RENDERER__CloudUtils_H__ */
