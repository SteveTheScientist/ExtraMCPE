#include "TextEditScreen.h"
#include "../../../world/level/tile/entity/SignTileEntity.h"
#include "../../../AppPlatform.h"
#include "../../Minecraft.h"
#include "../../renderer/tileentity/TileEntityRenderDispatcher.h"
#include "../../renderer/Tesselator.h"
#include "../../renderer/Textures.h"
#include "../../renderer/GameRenderer.h"
#include "../components/Button.h"
#include "../../../network/Packet.h"
#include "../../../network/RakNetInstance.h"
#include "../../../platform/input/Keyboard.h"
#include "../../../platform/input/Mouse.h"
#include "../../../platform/time.h"
#include "util/StringUtils.h"

TextEditScreen::TextEditScreen( SignTileEntity* signEntity )
 : sign(signEntity), isShowingKeyboard(false), frame(0), line(0), btnClose(1, ""),
   m_isPressingSign(false), m_pressStartTime(0), m_showPopup(false),
   m_popupX(0), m_popupY(0), m_popupW(180), m_popupH(20) {

}

TextEditScreen::~TextEditScreen() {

}

void TextEditScreen::init() {
	super::init();
	minecraft->platform()->showKeyboard();
	isShowingKeyboard = true;
	ImageDef def;
	def.name = "gui/spritesheet.png";
	def.x = 0;
	def.y = 1;
	def.width = def.height = 18;
	def.setSrc(IntRectangle(60, 0, 18, 18));
	btnClose.setImageDef(def, true);
	btnClose.scaleWhenPressed = false;
	buttons.push_back(&btnClose);
	m_isPressingSign = false;
	m_showPopup = false;
}

void TextEditScreen::setupPositions() {
	btnClose.width = btnClose.height = 19;
	btnClose.x = width - btnClose.width;
	btnClose.y = 0;
}

bool TextEditScreen::handleBackEvent( bool isDown ) {
    sign->setChanged();
	Packet* signUpdatePacket = sign->getUpdatePacket();
	minecraft->raknetInstance->send(signUpdatePacket);
	minecraft->platform()->hideKeyboard();
	minecraft->setScreen(NULL);
	return true;
}

void TextEditScreen::render( int xm, int ym, float a ) {
	glDepthMask(GL_FALSE);
	renderBackground();
	glPushMatrix();
	glDepthMask(GL_TRUE);
	glDisable(GL_CULL_FACE);
	glLoadIdentity();
	Tesselator& t = Tesselator::instance;

	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	glOrthof(0.0f, (float)minecraft->width, (float)minecraft->height, 0,  -1, 1);
	glMatrixMode(GL_MODELVIEW);

	minecraft->textures->loadAndBindTexture("item/sign.png");
    glColor4f2(1, 1, 1, 1);

	static float minUV[] = {0.03126f, 0.06249f};
    static float maxUV[] = {0.39063f, 0.4374f};
	float scale = ((minecraft->height / 2) / 32) * 0.9f;

	glTranslatef(minecraft->width / 2.0f, 5.0f, 0.0f);
	glScalef2(scale,scale,1);
	t.begin(GL_QUADS);
	t.vertexUV(-32, 0, 0.0f,minUV[0],minUV[1]);
	t.vertexUV(32,  0, 0.0f, maxUV[0], minUV[1]);
	t.vertexUV(32, 0 + 32, 0.0f, maxUV[0], maxUV[1]);
	t.vertexUV(-32, 0 + 32, 0.0f, minUV[0], maxUV[1]);
	t.draw();

	sign->selectedLine = line;
	float textScale = 8.0f / 11.0f;

	glTranslatef(0, 2 ,0);
	glScalef2(textScale, textScale, 1);
	for(int i = 0; i < 4; ++i) {
		std::string msg = sign->messages[i];
		if (i == sign->selectedLine && msg.length() < 14) {
			std::string s = "> " + msg + " <";
			font->draw(s, -(float)font->width(s, true) / 2.0f, 10.0f * i, 0xFF000000, false, true);
		} else {
			font->draw(msg, -(float)font->width(msg, true) / 2.0f, 10.0f * i, 0xFF000000, false, true);
		}
	}
	sign->selectedLine  = -1;

	glPopMatrix();
	glEnable(GL_CULL_FACE);
	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
	glMatrixMode(GL_MODELVIEW);

	if (m_showPopup) {
		fill(m_popupX - 1, m_popupY - 1, m_popupX + m_popupW + 1, m_popupY + m_popupH + 1, 0xff555555);
		fill(m_popupX, m_popupY, m_popupX + m_popupW, m_popupY + m_popupH, 0xeb101010);

		int btnW = m_popupW / 4;
		static const char* btnLabels[4] = { "Select All", "Copy", "Cut", "Paste" };

		for (int i = 0; i < 4; i++) {
			int bx0 = m_popupX + i * btnW;
			int bx1 = bx0 + btnW;

			bool isHover = (xm >= bx0 && xm < bx1 && ym >= m_popupY && ym < m_popupY + m_popupH);
			if (isHover) {
				fill(bx0, m_popupY, bx1, m_popupY + m_popupH, 0x50ffffff);
			}

			if (i > 0) {
				fill(bx0, m_popupY + 2, bx0 + 1, m_popupY + m_popupH - 2, 0xff555555);
			}

			int labelX = bx0 + (btnW - font->width(btnLabels[i])) / 2;
			int labelY = m_popupY + (m_popupH - Font::DefaultLineHeight) / 2;
			font->drawShadow(btnLabels[i], (float)labelX, (float)labelY, isHover ? 0xffffff55 : 0xffffffff);
		}
	}

	super::render(xm, ym, a);
}

void TextEditScreen::lostFocus() {

}

void TextEditScreen::tick() {
	frame++;

	if (m_isPressingSign) {
		if (Mouse::isButtonDown(MouseAction::ACTION_LEFT)) {
			long elapsed = getTimeMs() - m_pressStartTime;
			if (elapsed >= 500) {
				m_isPressingSign = false;
				m_showPopup = true;

				m_popupW = 180;
				m_popupH = 20;
				m_popupX = (width - m_popupW) / 2;
				if (m_popupX < 2) m_popupX = 2;
				if (m_popupX + m_popupW > width - 2) m_popupX = width - m_popupW - 2;

				m_popupY = 10; // Position above sign
			}
		} else {
			m_isPressingSign = false;
		}
	}
}

void TextEditScreen::keyPressed( int eventKey ) {
	bool ctrlDown = Keyboard::isKeyDown(Keyboard::KEY_LEFT_CTRL) || Keyboard::isKeyDown(232) || Keyboard::isKeyDown(224) || Keyboard::isKeyDown(228);

	if (ctrlDown) {
		std::string& curMsg = sign->messages[line];
		if (eventKey == Keyboard::KEY_C || eventKey == 'c' || eventKey == 'C') {
			if (!curMsg.empty()) {
				minecraft->platform()->setClipboardText(curMsg);
			}
			return;
		}
		if (eventKey == Keyboard::KEY_X || eventKey == 'x' || eventKey == 'X') {
			if (!curMsg.empty()) {
				minecraft->platform()->setClipboardText(curMsg);
				curMsg.clear();
			}
			return;
		}
		if (eventKey == Keyboard::KEY_V || eventKey == 'v' || eventKey == 'V') {
			std::string clip = minecraft->platform()->getClipboardText();
			if (!clip.empty()) {
				std::string cleaned = Util::cleanPastedText(clip, 15 - curMsg.length());
				curMsg += cleaned;
			}
			return;
		}
		if (eventKey == Keyboard::KEY_A || eventKey == 'a' || eventKey == 'A') {
			return;
		}
	}

	if(eventKey == Keyboard::KEY_BACKSPACE) {
		if(sign->messages[line].length() > 0) {
			sign->messages[line].erase(sign->messages[line].size() - 1, 1);
		} else {
            line--;
            if(line < 0) {
                line = 3;
            }
        }
	} else if(eventKey == Keyboard::KEY_RETURN)  {
		line = (line + 1) % 4;
	} else {
		super::keyPressed(eventKey);
	}
}

void TextEditScreen::charPressed( char inputChar ) {
	bool ctrlDown = Keyboard::isKeyDown(Keyboard::KEY_LEFT_CTRL) || Keyboard::isKeyDown(232) || Keyboard::isKeyDown(224) || Keyboard::isKeyDown(228);
	if (ctrlDown || (unsigned char)inputChar < 32 || inputChar == 127) return;

	std::string fullstring = sign->messages[line] + inputChar;
	if(fullstring.length() < 16) {
		sign->messages[line] = fullstring;
	}
}

void TextEditScreen::mouseClicked(int x, int y, int buttonNum) {
	if (buttonNum == MouseAction::ACTION_LEFT) {
		if (m_showPopup) {
			if (x >= m_popupX && x <= m_popupX + m_popupW && y >= m_popupY && y <= m_popupY + m_popupH) {
				int btnIndex = (x - m_popupX) / (m_popupW / 4);
				if (btnIndex < 0) btnIndex = 0;
				if (btnIndex > 3) btnIndex = 3;

				std::string& curMsg = sign->messages[line];
				if (btnIndex == 0) { // Select All
				} else if (btnIndex == 1) { // Copy
					if (!curMsg.empty()) {
						minecraft->platform()->setClipboardText(curMsg);
					}
				} else if (btnIndex == 2) { // Cut
					if (!curMsg.empty()) {
						minecraft->platform()->setClipboardText(curMsg);
						curMsg.clear();
					}
				} else if (btnIndex == 3) { // Paste
					std::string clip = minecraft->platform()->getClipboardText();
					if (!clip.empty()) {
						std::string cleaned = Util::cleanPastedText(clip, 15 - curMsg.length());
						curMsg += cleaned;
					}
				}
				m_showPopup = false;
				return;
			} else {
				m_showPopup = false;
			}
		}

		m_isPressingSign = true;
		m_pressStartTime = getTimeMs();
	}
	super::mouseClicked(x, y, buttonNum);
}

void TextEditScreen::mouseReleased(int x, int y, int buttonNum) {
	m_isPressingSign = false;
	super::mouseReleased(x, y, buttonNum);
}

void TextEditScreen::buttonClicked( Button* button ) {
	if(button == &btnClose)
		handleBackEvent(true);
}
