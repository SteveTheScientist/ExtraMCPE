#include "SkinsScreen.h"
#include "CustomizationsScreen.h"
#include "../../Minecraft.h"
#include "../../player/LocalPlayer.h"
#include "../../renderer/Tesselator.h"
#include "../../renderer/Textures.h"
#include "../../renderer/entity/EntityRenderDispatcher.h"
#include "../../../util/Mth.h"
#include "../../../world/level/storage/FolderMethods.h"
#include "../../../world/level/storage/MemoryLevelStorage.h"
#include "../../../world/level/Level.h"
#include "../../../world/level/LevelSettings.h"
#include "platform/input/Keyboard.h"
#include "platform/input/Mouse.h"
#include "platform/log.h"
#include <algorithm>
#include <fstream>
#include <sstream>
#include <cctype>
#include <map>
#include <sys/stat.h>

#ifndef WIN32
#include <dirent.h>
#else
#include <windows.h>
#endif

static const bool SHOW_DEBUG_INFO = true;

static std::string intToStr(int val) {
    std::ostringstream oss;
    oss << val;
    return oss.str();
}

static bool fileExists(const std::string& path) {
    if (path.empty()) return false;
    struct stat st;
    return stat(path.c_str(), &st) == 0;
}

static void ensureDefaultSkinExists() {
    createFolderIfNotExists("data");
    createFolderIfNotExists("data/images");
    createFolderIfNotExists("data/images/skins");

    std::string skinChar = "data/images/skins/char.png";
    if (!fileExists(skinChar)) {
        std::string mobChar = "data/images/mob/char.png";
        std::ifstream src(mobChar.c_str(), std::ios::binary);
        if (src.is_open()) {
            std::ofstream dst(skinChar.c_str(), std::ios::binary);
            if (dst.is_open()) {
                dst << src.rdbuf();
                dst.close();
            }
            src.close();
        }
    }
}

static std::string formatSkinName(const std::string& key) {
    std::string name = key;
    for (size_t i = 0; i < name.length(); ++i) {
        if (name[i] == '_' || name[i] == '-') {
            name[i] = ' ';
        }
    }
    bool newWord = true;
    for (size_t i = 0; i < name.length(); ++i) {
        if (name[i] == ' ') {
            newWord = true;
        } else if (newWord) {
            name[i] = (char)toupper((unsigned char)name[i]);
            newWord = false;
        }
    }
    return name;
}

void SkinsScreen::setSkinFromFile(Minecraft* minecraft, Player* player, const std::string& skinFileName) {
    if (!minecraft) return;

    ensureDefaultSkinExists();

    std::string filename = skinFileName;
    if (filename.empty()) filename = "char.png";

    std::string texPath = "skins/" + filename;
    bool isValid = false;

    if (minecraft->textures) {
        TextureId tid = minecraft->textures->loadTexture(texPath, true);
        if (tid != Textures::InvalidId) {
            const TextureData* td = minecraft->textures->getTemporaryTextureData(tid);
            if (td && td->data && td->w == 64 && (td->h == 32 || td->h == 64)) {
                isValid = true;
            }
        }
    }

    if (!isValid && filename != "char.png") {
        LOGW("[Skins] Saved skin file '%s' is missing or invalid, falling back to char.png\n", filename.c_str());
        filename = "char.png";
        texPath = "skins/char.png";
        if (minecraft->textures) {
            minecraft->textures->loadTexture(texPath, true);
        }
    }

    minecraft->options.set(OPTIONS_SKIN_NAME, filename);
    minecraft->options.save();

    if (player) {
        player->setTextureName(texPath);
    }
}

void SkinsScreen::applySavedSkin(Minecraft* minecraft, Player* player) {
    if (!minecraft) return;
    std::string savedSkin = minecraft->options.getStringValue(OPTIONS_SKIN_NAME);
    if (savedSkin.empty()) savedSkin = "char.png";
    setSkinFromFile(minecraft, player, savedSkin);
}

//
// SkinList implementation
//
SkinList::SkinList(SkinsScreen* screen, Minecraft* minecraft, int width, int height)
    : super(minecraft, width, height, 0, width, 34, height - 34, 140),
      selectedItem(0),
      mode(0),
      m_screen(screen)
{
    td.start = td.stop = td.cur = td.dur = 0.0f;
    xo = xoo = (float)(itemWidth - width) * 0.5f;
}

int SkinList::getNumberOfItems() {
    if (!m_screen) return 0;
    return (int)m_screen->m_skinFiles.size();
}

void SkinList::selectItem(int item, bool doubleClick) {
    if (selectedItem < 0 || selectedItem != item) {
        selectedItem = item;
    }
}

bool SkinList::isSelectedItem(int item) {
    return item == selectedItem;
}

void SkinList::renderItem(int i, int x, int y, int h, Tesselator& t) {
    if (!m_screen || !minecraft || !minecraft->font) return;
    int centerx = x + h / 2;

    float a0 = std::max(0.2f, 1.1f - std::abs(width / 2 - centerx) * 0.0055f);
    if (a0 > 1.0f) a0 = 1.0f;
    int textColor = (int)(255.0f * a0) * 0x010101;

    // Skin display name above player preview
    if (i >= 0 && i < (int)m_screen->m_skinNames.size()) {
        const std::string& name = m_screen->m_skinNames[i];
        int nameW = minecraft->font->width(name);
        drawString(minecraft->font, name.c_str(), centerx - nameW / 2, y + 10, textColor);
    }

    // Render player preview
    float playerY = (float)height * 0.72f;
    if (i >= 0 && i < (int)m_screen->m_skinFiles.size()) {
        if (m_screen->m_previewPlayer) {
            m_screen->renderPlayer((float)centerx, playerY, "skins/" + m_screen->m_skinFiles[i]);
        } else {
            int textW = minecraft->font->width("Preview unavailable");
            drawString(minecraft->font, "Preview unavailable", centerx - textW / 2, (int)playerY, 0xffa0a0a0);
        }
    }
}

void SkinList::setSelectedItemAndCenter(int index) {
    selectedItem = index;
    xo = xoo = (float)(index * itemWidth - (width - itemWidth) * 0.5f);
}

void SkinList::stepLeft() {
    if (selectedItem > 0) {
        td.start = xo;
        td.stop = xo - itemWidth;
        td.cur = 0;
        td.dur = 8;
        mode = 1;
        tweenInited();
    }
}

void SkinList::stepRight() {
    if (selectedItem >= 0 && selectedItem < getNumberOfItems() - 1) {
        td.start = xo;
        td.stop = xo + itemWidth;
        td.cur = 0;
        td.dur = 8;
        mode = 1;
        tweenInited();
    }
}

static float quadraticInOut(float t, float dur, float start, float stop) {
    const float delta = stop - start;
    const float T = (t / dur) * 2.0f;
    if (T < 1.0f) return 0.5f * delta * T * T + start;
    return -0.5f * delta * ((T - 1.0f) * (T - 3.0f) - 1.0f) + start;
}

void SkinList::tweenInited() {
    float x0 = quadraticInOut(td.cur, td.dur, td.start, td.stop);
    float x1 = quadraticInOut(td.cur + 1.0f, td.dur, td.start, td.stop);
    xInertia = x0 - x1;
}

float SkinList::getPos(float alpha) {
    if (mode != 1) return super::getPos(alpha);
    float x0 = quadraticInOut(td.cur, td.dur, td.start, td.stop);
    float x1 = quadraticInOut(td.cur + 1.0f, td.dur, td.start, td.stop);
    return x0 + (x1 - x0) * alpha;
}

bool SkinList::capXPosition() {
    bool capped = super::capXPosition();
    if (capped) mode = 0;
    return capped;
}

void SkinList::tick() {
    super::tick();

    if (Mouse::isButtonDown(MouseAction::ACTION_LEFT) || dragState == 0)
        return;

    if (mode == 1) {
        if (++td.cur == td.dur) {
            mode = 0;
            xInertia = 0;
            xoo = xo = td.stop;
            selectedItem = getItemAtPosition(width / 2, height / 2);
        } else {
            tweenInited();
        }
        return;
    }

    float speed = Mth::abs(xInertia);
    bool slowEnoughToBeBothered = speed < 5.0f;
    if (!slowEnoughToBeBothered) {
        xInertia *= 0.9f;
        return;
    }

    xInertia *= 0.8f;

    if (speed < 1.0f && dragState < 0) {
        const int offsetx = (width - itemWidth) / 2;
        const float pxo = xo + offsetx;
        int index = getItemAtXPositionRaw((int)(pxo - 10.0f * xInertia));
        int indexPos = index * itemWidth;

        float diff = (float)indexPos - pxo;
        if (diff < -itemWidth / 2) {
            diff += itemWidth;
            index++;
        }
        if (Mth::abs(diff) < 1.0f && speed < 0.1f) {
            selectedItem = getItemAtPosition(width / 2, height / 2);
            return;
        }

        td.start = xo;
        td.stop = xo + diff;
        td.cur = 0;
        td.dur = (float)Mth::Min(7, 1 + (int)(Mth::abs(diff) * 0.25f));
        mode = 1;
        tweenInited();
    }
}

//
// SkinsScreen implementation
//
SkinsScreen::SkinsScreen()
    : btnReset(NULL),
      btnSelect(NULL),
      btnDone(NULL),
      btnLeft(NULL),
      btnRight(NULL),
      m_skinList(NULL),
      m_previewPlayer(NULL),
      m_dummyLevel(NULL),
      m_dummyStorage(NULL),
      m_ownsPreviewPlayer(false),
      m_debugDiskCount(0),
      m_debugAssetCount(0)
{
}

SkinsScreen::~SkinsScreen() {
    delete btnReset;
    delete btnSelect;
    delete btnDone;
    delete btnLeft;
    delete btnRight;
    delete m_skinList;

    if (m_ownsPreviewPlayer) {
        delete m_previewPlayer;
        delete m_dummyLevel;
        m_dummyLevel = NULL;
        m_previewPlayer = NULL;
        m_dummyStorage = NULL;
    }
}

void SkinsScreen::loadSkins() {
    m_skinFiles.clear();
    m_skinNames.clear();

    if (!minecraft) return;

    ensureDefaultSkinExists();

    createFolderIfNotExists("data/images/skins/texts");

    std::string scannedDiskPath;
    std::vector<std::string> diskFiles;
    std::vector<std::string> assetFiles;

    std::vector<std::string> candidateFiles = listDataFiles(
        minecraft->platform(),
        "images/skins",
        ".png",
        &scannedDiskPath,
        &diskFiles,
        &assetFiles
    );

    m_debugDiskPath = scannedDiskPath;
    m_debugDiskCount = (int)diskFiles.size();
    m_debugAssetCount = (int)assetFiles.size();

    LOGI("[Skins] Scanned disk path: '%s' (%d files)\n", scannedDiskPath.c_str(), (int)diskFiles.size());
    for (size_t i = 0; i < diskFiles.size(); ++i) {
        LOGI("[Skins]   Disk file: %s\n", diskFiles[i].c_str());
    }
    LOGI("[Skins] Scanned asset path: 'images/skins' (%d files)\n", (int)assetFiles.size());
    for (size_t i = 0; i < assetFiles.size(); ++i) {
        LOGI("[Skins]   Asset file: %s\n", assetFiles[i].c_str());
    }

    if (minecraft->textures) {
        for (size_t i = 0; i < candidateFiles.size(); ++i) {
            const std::string& fn = candidateFiles[i];
            if (fn.empty()) continue;

            TextureId texId = minecraft->textures->loadTexture("skins/" + fn, true);
            if (texId != Textures::InvalidId) {
                const TextureData* texData = minecraft->textures->getTemporaryTextureData(texId);
                if (texData && texData->data && texData->w == 64 && (texData->h == 32 || texData->h == 64)) {
                    m_skinFiles.push_back(fn);
                } else {
                    LOGW("[Skins] Skipping invalid skin file '%s' (data=%p, w=%d, h=%d)\n",
                         fn.c_str(), texData ? (void*)texData->data : NULL,
                         texData ? texData->w : 0, texData ? texData->h : 0);
                }
            } else {
                LOGW("[Skins] Failed to load skin texture '%s'\n", fn.c_str());
            }
        }
    }

    std::sort(m_skinFiles.begin(), m_skinFiles.end());

    std::string langPath = "data/images/skins/texts/en_us.lang";
    std::string assetLangPath = "images/skins/texts/en_us.lang";

    if (!fileExists(langPath) && minecraft->platform()) {
        BinaryBlob langBlob = minecraft->platform()->readAssetFile(assetLangPath);
        if (langBlob.data && langBlob.size > 0) {
            std::ofstream out(langPath.c_str(), std::ios::binary);
            if (out.is_open()) {
                out.write((const char*)langBlob.data, langBlob.size);
                out.close();
            }
            delete[] langBlob.data;
        }
    }

    std::map<std::string, std::string> langMap;

    std::ifstream langIn(langPath.c_str());
    if (langIn.is_open()) {
        std::string line;
        while (std::getline(langIn, line)) {
            while (!line.empty() && (line.at(line.length() - 1) == '\r' || line.at(line.length() - 1) == '\n')) {
                line.erase(line.length() - 1);
            }
            size_t eqPos = line.find('=');
            if (eqPos != std::string::npos) {
                std::string k = line.substr(0, eqPos);
                std::string v = line.substr(eqPos + 1);
                langMap[k] = v;
            }
        }
        langIn.close();
    }

    std::vector<std::string> newEntriesToAppend;

    for (size_t i = 0; i < m_skinFiles.size(); ++i) {
        std::string fn = m_skinFiles[i];
        if (fn.length() < 5) continue;
        std::string key = fn.substr(0, fn.length() - 4);

        std::map<std::string, std::string>::iterator it = langMap.find(key);
        if (it != langMap.end()) {
            m_skinNames.push_back(it->second);
        } else {
            std::string genName = formatSkinName(key);
            langMap[key] = genName;
            m_skinNames.push_back(genName);
            newEntriesToAppend.push_back(key + "=" + genName);
        }
    }

    if (!newEntriesToAppend.empty()) {
        std::ofstream langOut(langPath.c_str(), std::ios::out | std::ios::app);
        if (langOut.is_open()) {
            for (size_t i = 0; i < newEntriesToAppend.size(); ++i) {
                langOut << newEntriesToAppend[i] << "\n";
            }
            langOut.close();
        }
    }

    LOGI("[Skins] Total valid skins ready (%d):\n", (int)m_skinFiles.size());
    for (size_t i = 0; i < m_skinFiles.size(); ++i) {
        LOGI("[Skins]   Skin %d: %s ('%s')\n", (int)i, m_skinFiles[i].c_str(),
             (i < m_skinNames.size() ? m_skinNames[i].c_str() : ""));
    }
}

void SkinsScreen::init() {
    super::init();

    btnReset  = new Button(0, 0, 0, 84, 20, "Reset");
    btnSelect = new Button(1, 0, 0, 84, 20, "Select");
    btnDone   = new Button(2, 0, 0, 84, 20, "Done");
    btnLeft   = new Button(3, 0, 0, 20, 20, "<");
    btnRight  = new Button(4, 0, 0, 20, 20, ">");

    buttons.push_back(btnReset);
    buttons.push_back(btnSelect);
    buttons.push_back(btnDone);
    buttons.push_back(btnLeft);
    buttons.push_back(btnRight);

    tabButtons.push_back(btnReset);
    tabButtons.push_back(btnSelect);
    tabButtons.push_back(btnDone);

    if (minecraft && minecraft->player) {
        m_previewPlayer = minecraft->player;
        m_ownsPreviewPlayer = false;
    } else {
        m_dummyStorage = new MemoryLevelStorage();
        m_dummyLevel = new Level(m_dummyStorage, "SkinPreview", LevelSettings::None(), 0);
        m_previewPlayer = new Player(m_dummyLevel, false);
        m_ownsPreviewPlayer = true;
    }

    loadSkins();

    delete m_skinList;
    m_skinList = new SkinList(this, minecraft, width, height);

    // Position carousel at the currently applied skin
    std::string currentApplied = minecraft ? minecraft->options.getStringValue(OPTIONS_SKIN_NAME) : "char.png";
    if (currentApplied.empty()) currentApplied = "char.png";

    int foundIdx = -1;
    for (size_t i = 0; i < m_skinFiles.size(); ++i) {
        if (m_skinFiles[i] == currentApplied) {
            foundIdx = (int)i;
            break;
        }
    }
    if (foundIdx >= 0) {
        m_skinList->setSelectedItemAndCenter(foundIdx);
    } else {
        applySavedSkin(minecraft, minecraft ? minecraft->player : NULL);
        for (size_t i = 0; i < m_skinFiles.size(); ++i) {
            if (m_skinFiles[i] == "char.png") {
                m_skinList->setSelectedItemAndCenter((int)i);
                break;
            }
        }
    }
}

void SkinsScreen::setupPositions() {
    int yBase = height - 28;
    int btnW = 84;

    if (btnReset) {
        btnReset->width = btnW; btnReset->height = 20; btnReset->y = yBase;
        btnReset->x = width / 2 - 4 - btnW - btnW / 2;
    }
    if (btnSelect) {
        btnSelect->width = btnW; btnSelect->height = 20; btnSelect->y = yBase;
        btnSelect->x = width / 2 - btnW / 2;
    }
    if (btnDone) {
        btnDone->width = btnW; btnDone->height = 20; btnDone->y = yBase;
        btnDone->x = width / 2 + 4 + btnW - btnW / 2;
    }

    if (btnLeft) {
        btnLeft->width = 20; btnLeft->height = 20;
        btnLeft->x = width / 2 - 110;
        btnLeft->y = height / 2 - 10;
    }
    if (btnRight) {
        btnRight->width = 20; btnRight->height = 20;
        btnRight->x = width / 2 + 90;
        btnRight->y = height / 2 - 10;
    }
}

void SkinsScreen::tick() {
    super::tick();
    if (m_skinList) {
        m_skinList->tick();
        if (m_skinFiles.empty()) {
            if (btnLeft) { btnLeft->active = false; btnLeft->visible = false; }
            if (btnRight) { btnRight->active = false; btnRight->visible = false; }
            if (btnSelect) btnSelect->active = false;
            if (btnReset) btnReset->active = false;
        } else {
            if (btnLeft) btnLeft->visible = true;
            if (btnRight) btnRight->visible = true;

            int idx = m_skinList->selectedItem;
            int num = m_skinList->getNumberOfItems();
            if (btnLeft) btnLeft->active = (idx > 0);
            if (btnRight) btnRight->active = (idx >= 0 && idx < num - 1);

            std::string previewedSkin = (idx >= 0 && idx < (int)m_skinFiles.size()) ? m_skinFiles[idx] : "";
            std::string currentApplied = minecraft ? minecraft->options.getStringValue(OPTIONS_SKIN_NAME) : "char.png";
            if (currentApplied.empty()) currentApplied = "char.png";

            if (btnSelect) {
                btnSelect->active = (!previewedSkin.empty() && previewedSkin != currentApplied);
            }
            if (btnReset) {
                btnReset->active = (currentApplied != "char.png");
            }
        }
    }
}

void SkinsScreen::render(int xm, int ym, float a) {
    if (minecraft && minecraft->isLevelGenerated()) {
        renderBackground();
    } else {
        renderDirtBackground(0);
    }

    if (m_skinFiles.empty()) {
        std::string msg = "No skins found in\ndata/images/skins/";
        std::stringstream ss(msg);
        std::string line;
        std::vector<std::string> lines;
        while (std::getline(ss, line)) {
            lines.push_back(line);
        }
        int totalH = (int)lines.size() * 12;
        int startY = height / 2 - totalH / 2;
        for (size_t i = 0; i < lines.size(); ++i) {
            if (minecraft && minecraft->font) {
                drawCenteredString(minecraft->font, lines[i], width / 2, startY + (int)i * 12, 0xffffffff);
            }
        }
    } else if (m_skinList) {
        m_skinList->render(xm, ym, a);
    }

    super::render(xm, ym, a);

    if (minecraft && minecraft->font) {
        drawCenteredString(minecraft->font, "Skins", width / 2, 10, 0xffffffff);

        if (SHOW_DEBUG_INFO) {
            std::string debugStr = "disk: " + m_debugDiskPath + " (" + intToStr(m_debugDiskCount) + ") | assets (" + intToStr(m_debugAssetCount) + ")";
            int textW = minecraft->font->width(debugStr);
            int drawX = width / 2 - textW / 2;
            if (drawX < 2) drawX = 2;
            drawString(minecraft->font, debugStr.c_str(), drawX, height - 10, 0xffa0a0a0);
        }
    }
}

void SkinsScreen::buttonClicked(Button* button) {
    if (!button || !minecraft) return;

    if (button == btnDone) {
        minecraft->setScreen(new CustomizationsScreen());
        return;
    }
    if (button == btnLeft && m_skinList) {
        m_skinList->stepLeft();
        return;
    }
    if (button == btnRight && m_skinList) {
        m_skinList->stepRight();
        return;
    }
    if (button == btnSelect && m_skinList) {
        int idx = m_skinList->selectedItem;
        if (idx >= 0 && idx < (int)m_skinFiles.size()) {
            std::string chosen = m_skinFiles[idx];
            setSkinFromFile(minecraft, minecraft->player, chosen);
            if (m_ownsPreviewPlayer && m_previewPlayer) {
                m_previewPlayer->setTextureName("skins/" + chosen);
            }
        }
        return;
    }
    if (button == btnReset) {
        setSkinFromFile(minecraft, minecraft->player, "char.png");
        if (m_ownsPreviewPlayer && m_previewPlayer) {
            std::string charTex = fileExists("data/images/skins/char.png") ? "skins/char.png" : "mob/char.png";
            m_previewPlayer->setTextureName(charTex);
        }
        for (size_t i = 0; i < m_skinFiles.size(); ++i) {
            if (m_skinFiles[i] == "char.png") {
                m_skinList->setSelectedItemAndCenter((int)i);
                break;
            }
        }
        return;
    }
}

void SkinsScreen::mouseClicked(int x, int y, int buttonNum) {
    super::mouseClicked(x, y, buttonNum);
}

void SkinsScreen::mouseReleased(int x, int y, int buttonNum) {
    super::mouseReleased(x, y, buttonNum);
}

void SkinsScreen::mouseWheel(int dx, int dy, int xm, int ym) {
    if (m_skinList && dy != 0) {
        if (dy > 0) {
            m_skinList->stepLeft();
        } else {
            m_skinList->stepRight();
        }
    }
    super::mouseWheel(dx, dy, xm, ym);
}

void SkinsScreen::keyPressed(int eventKey) {
    if (eventKey == Keyboard::KEY_ESCAPE) {
        if (minecraft) minecraft->setScreen(new CustomizationsScreen());
        return;
    }
    if (m_skinList && minecraft) {
        if (eventKey == minecraft->options.getIntValue(OPTIONS_KEY_LEFT)) {
            m_skinList->stepLeft();
            return;
        }
        if (eventKey == minecraft->options.getIntValue(OPTIONS_KEY_RIGHT)) {
            m_skinList->stepRight();
            return;
        }
    }
    super::keyPressed(eventKey);
}

void SkinsScreen::renderPlayer(float xo, float yo, const std::string& skinTexturePath) {
    if (!m_previewPlayer || !minecraft || !minecraft->textures) return;

    EntityRenderDispatcher* rd = EntityRenderDispatcher::getInstance();
    if (!rd) return;

    rd->setMinecraft(minecraft);
    rd->textures = minecraft->textures;
    rd->cameraEntity = m_previewPlayer;

    glPushMatrix();

    glTranslatef(xo, yo, -200.0f);
    float ss = 45.0f;
    glScalef(-ss, ss, ss);

    glRotatef(180.0f, 0, 0, 1);

    float oybr = m_previewPlayer->yBodyRot;
    float oyr = m_previewPlayer->yRot;
    float oxr = m_previewPlayer->xRot;

    float t = getTimeS();

    float xd = 10.0f * Mth::sin(t);
    float yd = 10.0f * Mth::cos(t * 0.05f);

    glRotatef(45.0f + 90.0f, 0, 1, 0);
    glRotatef(-45.0f - 90.0f, 0, 1, 0);

    const float xtan = Mth::atan(xd / 40.0f) * +20.0f;
    const float ytan = Mth::atan(yd / 40.0f) * -20.0f;

    glRotatef(ytan, 1, 0, 0);

    m_previewPlayer->yBodyRot = xtan;
    m_previewPlayer->yRot = xtan + xtan;
    m_previewPlayer->xRot = ytan;
    glTranslatef(0, m_previewPlayer->heightOffset, 0);

    float oldWAP = m_previewPlayer->walkAnimPos;
    float oldWAS = m_previewPlayer->walkAnimSpeed;
    float oldWASO = m_previewPlayer->walkAnimSpeedO;

    m_previewPlayer->walkAnimSpeedO = m_previewPlayer->walkAnimSpeed = 0.25f;
    m_previewPlayer->walkAnimPos = getTimeS() * m_previewPlayer->walkAnimSpeed * SharedConstants::TicksPerSecond;

    std::string oldTex = m_previewPlayer->textureName;
    m_previewPlayer->textureName = skinTexturePath;

    rd->playerRotY = 180.0f;
    rd->render(m_previewPlayer, 0, 0, 0, 0, 1.0f);

    m_previewPlayer->textureName = oldTex;

    m_previewPlayer->walkAnimPos = oldWAP;
    m_previewPlayer->walkAnimSpeed = oldWAS;
    m_previewPlayer->walkAnimSpeedO = oldWASO;

    m_previewPlayer->yBodyRot = oybr;
    m_previewPlayer->yRot = oyr;
    m_previewPlayer->xRot = oxr;

    glPopMatrix();
}
