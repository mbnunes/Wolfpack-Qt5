	
#include "game/statictile.h"
#include "game/world.h"
#include "muls/art.h"
#include "muls/textures.h"
#include "muls/maps.h"
#include "muls/localization.h"
#include "gui/worldview.h"
#include "gui/gui.h"
#include "log.h"
#include "config.h"
#include <qgl.h>
//Added by qt3to4:
#include <QMouseEvent>

cStaticTile::cStaticTile() {
	id_ = 0;
	texture = 0;
	tiledata_ = Tiledata->getItemInfo(0);
	tiledata_->incref();
	type_ = STATIC;
	animated = false;
	animation = 0;
	drawStacked_ = false;
	hue_ = 0;
}

cStaticTile::cStaticTile(unsigned short x, unsigned short y, signed char z, enFacet facet) : cEntity(x, y, z, facet) {
	id_ = 0;
	texture = 0;
	tiledata_ = Tiledata->getItemInfo(0);
	tiledata_->incref();
	type_ = STATIC;
	animated = false;
	animation = 0;
	drawStacked_ = false;
	hue_ = 0;
}

cStaticTile::~cStaticTile() {
	if (texture) {
		texture->decref();
		texture = 0;
	}
	if (animation) {
		animation->decref();
		animation = 0;
	}
	tiledata_->decref();
}

void cStaticTile::setId(unsigned short data) {
	id_ = data;
	tiledata_->decref();
	tiledata_ = Tiledata->getItemInfo(data);
	tiledata_->incref();
	animated = tiledata_->isAnimated();
	if (texture) {
		texture->decref();
		texture = 0;
	}
	if (animation) {
		animation->decref();
		animation = 0;
	}
}

void cStaticTile::setHue(unsigned short data) {
	hue_ = data;
	if (texture) {
		texture->decref();
		texture = 0;
	}
	if (animation) {
		animation->decref();
		animation = 0;
	}
}

void cStaticTile::draw(int cellx, int celly, int leftClip, int topClip, int rightClip, int bottomClip) {
	if (!isInWorld()) {
		return;
	}

	// Check if these tiles are hidden
	if (type() == STATIC && Config->gameHideStatics() || type() == ITEM && Config->gameHideDynamics()) {
		return;
	}

	if (id_ == 1 || id_ == 0x21bc || id_ == 0x21a4) {
		return; // Nodraw Tile
	}

	// If the bottom is out of view, don't even bother to draw
	if (celly + 22 < topClip) {
		return;
	}

	if (!animated && !texture && id_) {
		texture = Art->readItemTexture(id_, hue_, tiledata_->isPartialHue());
		if (texture) {
			width_ = texture->realWidth();
			height_ = texture->realHeight();
		}
	} else if (animated && !animation && id_) {
		animation = Art->readAnimation(id_, hue_, tiledata_->isPartialHue());

		if (animation) {
			frame = 0;
			nextFrame = Utilities::getTicks() + animation->frameDelay(); // + animation->startDelay();
		} else {
			Log->print(LOG_DEBUG, tr("Unable to load animation for art tile id 0x%1. Pos (%2,%3,%4,%5).\n").arg(id_).arg(x_).arg(y_).arg(z_).arg(facet_));
			animated = false;
		}
	}

	float alpha = 1.0f;

	// Translucent tiles have 50% alpha
	if (tiledata_ && tiledata_->isTranslucent()) {
		alpha = 0.5f;
	}

	if (animated && animation) {
		// Check for frame expiry
		/*if (nextFrame < Utilities::getTicks()) {
			if (++frame == animation->frameCount()) {
				nextFrame = Utilities::getTicks() + animation->frameDelay(); // + animation->startDelay(); // With start delay
				frame = 0; // Module framecount
			} else {
				nextFrame = Utilities::getTicks() + animation->frameDelay(); // Normal frame delay
			}
		}*/

		// UO is using a crap algorithm here. ALL animated art tiles are using the same time base.
		// Just use a modulo of the Utilities::getTicks() here
		frame = ((Utilities::getTicks() / animation->frameDelay()) % animation->frameCount());

		// Save a reference to the frame value
		const stArtFrame &info = animation->frames()[frame];

		// Draw the given frame
		drawx_ = cellx - (info.width >> 1);

		// Check if the draw coordinates are out of view
		if (drawx_ > rightClip || drawx_ + (int)info.width < leftClip) {
			return;
		}

		drawy_ = celly + 22 - info.height;

		// Check if the draw coordinate are out of view
		if (drawy_ > bottomClip) {
			return;
		}

		width_ = info.width;
		height_ = info.height;

		if (Config->gameHighlightStatics() && World->mouseOver() == this) {
			glColor4f(1.0f, 0.8f, 0.7f, alpha); // Red. No Alpha.
		} else {
			glColor4f(1.0f, 1.0f, 1.0f, alpha); // White. No Alpha.
		}

		animation->texture()->bind();
		glBegin(GL_QUADS);		
		glTexCoord2f(info.texelLeft, info.texelTop); glVertex2i(drawx_, drawy_);
		glTexCoord2f(info.texelRight, info.texelTop); glVertex2f(drawx_ + info.width, drawy_);
		glTexCoord2f(info.texelRight, info.texelBottom); glVertex2i(drawx_ + info.width, drawy_ + info.height);
		glTexCoord2f(info.texelLeft, info.texelBottom); glVertex2i(drawx_, drawy_ + info.height);
		glEnd();

	// Calculate the stretched tile positions
	} else if (!animated && texture) {
		// Save these values to speed up hit testing
		drawx_ = cellx - (texture->realWidth() >> 1);

		// Check if the draw coordinates are out of view
		if (drawx_ > rightClip || drawx_ + (int)texture->realWidth() < leftClip) {
			return;
		}

		drawy_ = celly + 22 - texture->realHeight();

		// Check if the draw coordinate are out of view
		if (drawy_ > bottomClip) {
			return;
		}

		if (Config->gameHighlightStatics() && World->mouseOver() == this) {
			glColor4f(1.0f, 0.8f, 0.7f, alpha); // White. No Alpha.
		} else {
			glColor4f(1.0f, 1.0f, 1.0f, alpha); // White. No Alpha.
		}

		// Simply case: unstretched tile
		texture->bind();

		glBegin(GL_QUADS);
		int right = drawx_ + texture->width();
		int bottom = drawy_ + texture->height();

		/*if (tiledata_->isFoliage()) {
			static int topMove = 0;
			static bool moveDir = true;
			static uint nextChange = 0;
			if (Utilities::getTicks() >= nextChange) {
				if (moveDir) {
					topMove++;
				} else {
					topMove--;
				}
				if (topMove > 3) {
					topMove = 3;
					moveDir = !moveDir;
				} else if (topMove < -3) {
					topMove = -3;
					moveDir = !moveDir;
				}
				nextChange = Utilities::getTicks() + 100;
			}

			glTexCoord2f(0, 0); glVertex2f(drawx_ - topMove, drawy_); // Upper left corner
			glTexCoord2f(1, 0); glVertex2f(right - topMove, drawy_); // Upper Right Corner
			glTexCoord2f(1, 1); glVertex2f(right, bottom); // Lower Right Corner
			glTexCoord2f(0, 1); glVertex2f(drawx_, bottom); // Lower Right Corner
		} else {*/
			glTexCoord2f(0, 0); glVertex2f(drawx_, drawy_); // Upper left corner
			glTexCoord2f(1, 0); glVertex2f(right, drawy_); // Upper Right Corner
			glTexCoord2f(1, 1); glVertex2f(right, bottom); // Lower Right Corner
			glTexCoord2f(0, 1); glVertex2f(drawx_, bottom); // Lower Right Corner
		//}

		if (drawStacked()) {
			glTexCoord2f(0, 0); glVertex2f(drawx_ + 5, drawy_ + 5); // Upper left corner
			glTexCoord2f(1, 0); glVertex2f(right + 5, drawy_ + 5); // Upper Right Corner
			glTexCoord2f(1, 1); glVertex2f(right + 5, bottom + 5); // Lower Right Corner
			glTexCoord2f(0, 1); glVertex2f(drawx_ + 5, bottom + 5); // Lower Right Corner
		}
		glEnd();
	}
}

bool cStaticTile::hitTest(int x, int y) {
	// Check if these tiles are hidden
	if (type() == STATIC && Config->gameHideStatics() || type() == ITEM && Config->gameHideDynamics()) {
		return false;
	}

	if (!animated && texture) {
		return texture->hitTest(x, y);
	} else if (animated && animation && x >= 0 && y >= 0 && x < width_ && y < height_) {
		if (frame < animation->frameCount()) {
			const stArtFrame &info = animation->frames()[frame];
			return animation->texture()->hitTest(x + info.x, y + info.y);
		}
		return false;
	} else {
		return false;
	}
}

void cStaticTile::onClick(QMouseEvent *e) {
	QString message = Localization->get(1020000 + id_);

	/*if (tiledata_->isArticleA()) {
		message = QString("a %1").arg(tiledata_->name().data());
	} else if (tiledata_->isArticleAn()) {
		message = QString("an %1").arg(tiledata_->name().data());
	} else {
		message = tiledata_->name();
	}*/

	//WorldView->addSysMessage(message);
	Gui->addOverheadText(e->x() - drawx(), e->y() - drawy(), 3000, message, 0x3b2, 3, this);
}

void cStaticTile::updatePriority() {
	priorityBonus_ = 0;

	if (tiledata_) {
		if (!tiledata_->isBackground()) {
			++priorityBonus_;
		}
		if (tiledata_->height() > 0) {
			++priorityBonus_;
		}
	}

	priority_ = z_ + priorityBonus_;
}
