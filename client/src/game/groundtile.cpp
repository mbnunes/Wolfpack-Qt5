
#include "game/groundtile.h"
#include "game/world.h"
#include "muls/art.h"
#include "muls/textures.h"
#include "muls/maps.h"
#include "gui/worldview.h"
#include "gui/contextmenu.h"
#include <qgl.h>
//Added by qt3to4:
#include <QMouseEvent>
#include "config.h"

cGroundTile::cGroundTile(unsigned short x, unsigned short y, signed char z, enFacet facet) : cEntity(x, y, z, facet) {
	id_ = 0;
	texture = 0;
	tiledata_ = Tiledata->getLandInfo(0);
	tiledata_->incref();
	left = 0;
	right = 0;
	bottom = 0;
	stretched = false;
	updated = false;
	type_ = GROUND;
	averagez_ = 0;
}

void cGroundTile::updateSurroundings() {
	stGroundInfo self;
	World->getGroundInfo(x_, y_, &self);

	left = self.left;
	right = self.right;
	bottom = self.bottom;

	bool newStretched = !(left == 0 && right == 0 && bottom == 0);

	// Replace the old texture if the stretching changed.
	if (newStretched != stretched) {
		if (texture) {
			texture->decref();
			texture = 0;
		}
	}

	stretched = newStretched;
	
	// Never even bother to calculate normals if the tile is not stretchable (water)
	// or doesnt have a texture
	if (!tiledata_->isWet() && tiledata_->texture()) {
		// Read in the 16 cells
		stGroundInfo cells[3][3];
		for (int x = 0; x < 3; ++x) {
			for (int y = 0; y < 3; ++y) {
				World->getGroundInfo(x_ - 1 + x, y_ - 1 + y, &(cells[x][y]));
				if (cells[x][y].bottom != cells[x][y].left || cells[x][y].left != cells[x][y].right) {
					stretched = true;
				}
			}
		}

		// Calculate the correct lighting normals for our vertices if we're stretched
		if (stretched) {
			// The four normals in relation to one corner
			cVector upperNormal, leftNormal, rightNormal, lowerNormal;

			// Upper Corner of the Tile
			upperNormal = cells[0][0].normals[2];
			leftNormal = cells[0][1].normals[1];
			rightNormal = cells[1][0].normals[3];
			lowerNormal = cells[1][1].normals[0];
			normals[0] = (upperNormal + lowerNormal + rightNormal + leftNormal).normalize();

			// Right Corner of the Tile
			upperNormal = cells[1][0].normals[2];
			leftNormal = cells[1][1].normals[1];
			rightNormal = cells[2][0].normals[3];
			lowerNormal = cells[2][1].normals[0];
			normals[1] = (upperNormal + lowerNormal + rightNormal + leftNormal).normalize();

			// Lower Corner of the Tile
			upperNormal = cells[1][1].normals[2];
			leftNormal = cells[1][2].normals[1];
			rightNormal = cells[2][1].normals[3];
			lowerNormal = cells[2][2].normals[0];
			normals[2] = (upperNormal + lowerNormal + rightNormal + leftNormal).normalize();

			// Left Corner of the Tile
			upperNormal = cells[0][1].normals[2];
			leftNormal = cells[0][2].normals[1];
			rightNormal = cells[1][1].normals[3];
			lowerNormal = cells[1][2].normals[0];
			normals[3] = (upperNormal + lowerNormal + rightNormal + leftNormal).normalize();
		}
	} else {
		stretched = false;
	}
}

cGroundTile::~cGroundTile() {
	if (texture) {
		texture->decref();
	}
	tiledata_->decref();
}

void cGroundTile::setId(unsigned short data) {
	id_ = data;
	// Free the reference to the old tiledata object
	tiledata_->decref();

	// Get a new tiledata reference
	tiledata_ = Tiledata->getLandInfo(data);
	tiledata_->incref();

	// Free the texture currently in use
	if (texture) {
		texture->decref();
		texture = 0;
	}
}

void cGroundTile::draw(int cellx, int celly, int leftClip, int topClip, int rightClip, int bottomClip) {
	if (Config->gameHideMap()) {
		return;
	}

	// Since the width is fixed, we can do that check here
	if (cellx - 22 > rightClip || cellx + 22 < leftClip || celly - 22 > bottomClip) {
		// Don't bother hit testing
		drawx_ = -9999;
		drawy_ = -9999;
		return;
	}

	// Calculate the normals for this cell
	if (!updated) {
		updateSurroundings();
		updated = true;
	}

	// If we don't have a texture yet. Retrieve it.
	if (!texture && id_) {
		if (stretched) {
			texture = Textures->readTexture(tiledata_->texture());
			texture->incref();
		} else {
			texture = Art->readLandTexture(id_);
		}
	}

	// Calculate the stretched tile positions
	if (texture) {
		bool highlight = World->mouseOver() == this;
		if (highlight) {
			glColor4f(1.0f, 0.0f, 0.0f, 1.0f); // White. No Alpha.
		} else {
			glColor4f(1.0f, 1.0f, 1.0f, 1.0f); // White. No Alpha.
		}

		if (!stretched) {
			// Save these values to speed up hit testing
			drawx_ = cellx - 22;
			drawy_ = celly - 22;
			width_ = 44;
			height_ = 44;

			// Don't draw if we're not visible
			if (celly + 22 < topClip) {
				return;
			}

			// Simply case: unstretched tile
			texture->bind();

			glBegin(GL_QUADS);
			int right = drawx_ + texture->width();
			int bottom = drawy_ + texture->height();		
			glTexCoord2f(0, 0); glVertex2f(drawx_, drawy_); // Upper left corner
			glTexCoord2f(1, 0); glVertex2f(right, drawy_); // Upper Right Corner
			glTexCoord2f(1, 1); glVertex2f(right, bottom); // Lower Right Corner
			glTexCoord2f(0, 1); glVertex2f(drawx_, bottom); // Lower Right Corner
			glEnd();
		} else {
			// Save these values to speed up hit testing
			drawx_ = cellx - 22;
			drawy_ = celly - 22;
			width_ = 44;
			height_ = QMAX(22 + QMAX(left, right), 44 + bottom);

			// Don't draw if we're not visible
			if (drawy_ + height_ < topClip) {
				return;
			}

			texture->bind();
			//glBindTexture(GL_TEXTURE_2D, 0);

			// enable lighting for this polygon
			if (!highlight) {
				glEnable(GL_LIGHTING);
			}

			/*glBegin(GL_QUADS);
			glNormal3f( normals[2].x, normals[2].y, normals[2].z);
			glTexCoord2f(1, 1); glVertex2f(cellx, celly + 22 + bottom);
			glNormal3f( normals[3].x, normals[3].y, normals[3].z);
			glTexCoord2f(0, 1); glVertex2f(cellx - 22, celly + left);
			glNormal3f( normals[0].x, normals[0].y, normals[0].z);
			glTexCoord2f(0, 0); glVertex2f(cellx, celly - 22);
			glNormal3f( normals[1].x, normals[1].y, normals[1].z);
			glTexCoord2f(1, 0); glVertex2f(cellx + 22, celly + right);
			glEnd();*/

			// Try the same thing with triangles
			glBegin(GL_TRIANGLES);
			glNormal3f( normals[2].x, normals[2].y, normals[2].z);
			glTexCoord2f(1, 1); glVertex2f(cellx - 0.25f, celly + 23 + bottom);
			glNormal3f( normals[3].x, normals[3].y, normals[3].z);
			glTexCoord2f(0, 1); glVertex2f(cellx - 22.5f, celly + left);
			glNormal3f( normals[0].x, normals[0].y, normals[0].z);
			glTexCoord2f(0, 0); glVertex2f(cellx - 0.25f, celly - 22);

			glNormal3f( normals[0].x, normals[0].y, normals[0].z);
			glTexCoord2f(0, 0); glVertex2f(cellx + 0.25f, celly - 22);
			glNormal3f( normals[1].x, normals[1].y, normals[1].z);
			glTexCoord2f(1, 0); glVertex2f(cellx + 22.5f, celly + right);
			glNormal3f( normals[2].x, normals[2].y, normals[2].z);
			glTexCoord2f(1, 1); glVertex2f(cellx + 0.25f, celly + 23 + bottom);
			glEnd();

			if (!highlight) {
				glDisable(GL_LIGHTING);
			}

			// Wireframe
			/*
			glColor4f(1.0f, 0.0f, 0.0f, 1.0f); // This is the color that will be shaded against the lighting
			glBindTexture(GL_TEXTURE_2D, 0); // Unbind texture

			glBegin(GL_LINES);			
			glVertex2f(cellx, celly + 22 + bottom - 1);
			glVertex2f(cellx - 22, celly + left - 1);

			glVertex2f(cellx - 22, celly + left - 1);
			glVertex2f(cellx, celly - 22);

			glVertex2f(cellx, celly - 22);
			glVertex2f(cellx + 22, celly + right - 1);

			glVertex2f(cellx + 22, celly + right - 1);
			glVertex2f(cellx, celly + 22 + bottom - 1);
			glEnd();*/
		}
	}
}

bool cGroundTile::hitTest(int x, int y) {
	if (Config->gameHideMap()) {
		return false;
	}

	if (stretched) {
		/// Do a simple bounding box check first
		if (x < 0 || y < 0 || x >= width_ || y >= height_) {
			return false;
		}

		// Now treat the left and right side differently
		if (x < 22) {
            // Now See if the mouse is within the upper left quadrant
			if (y < 22 + left) {
				// Calculate the increase and get a function value at the given
				// x coordinate.
				float increase = (22 + left) / 22.0f;
				if (22 + left - y < increase * x) {
					return true;
				}
			// Otherwise check the lower quadrant
			} else {
				float increase = (bottom + 22 - left) / 22.0f;
				if ((y - 22 + left) < x * increase) {
					return true;
				}
			}
		} else {
			// check the upper right quadrant
			if (y < 22 + right) {
				// Calculate the increase and get a function value at the given
				// x coordinate.
				float increase = (22 + right) / 22.0f;
				if (y > increase * (x - 22)) {
					return true;
				}
			// check the lower right quadrant
			} else {
				// Calculate the increase and get a function value at the given
				// x coordinate.
				float increase = (22 + bottom - right) / 22.0f;
				if ((44 + bottom - y) >= increase * (x - 22)) {
					return true;
				}
			}
		}

		return false;
	} else {
		if (texture) {
			return texture->hitTest(x, y);
		} else {
			return false;
		}
	}	
}

/*
	Ground tile context menu callback.
*/
static void STDCALL cmCallback(cContextMenu *menu, int id, cGroundTile *tile) {
	switch (id) {
		case 0:
			World->removeEntity(tile);
			tile->decref();
			break;
	}	
	tile->decref();
}

void cGroundTile::onRightClick(QMouseEvent *e) {	
}

void cGroundTile::onClick(QMouseEvent *e) {
	// SHIFT+Click shows contextmenu
	if ((e->state() & Qt::ShiftButton) != 0) {
		ContextMenu->clear();
		ContextMenu->addEntry("Delete", 0x3b2, 0); // Delete the ground tile
		incref();
		ContextMenu->setCallback((fnContextMenuCallback)cmCallback, this);
		ContextMenu->show();
		return;
	}

	QString message;
	if (tiledata_->isArticleA()) {
		message = QString("a %1").arg(tiledata_->name().data());
	} else if (tiledata_->isArticleAn()) {
		message = QString("an %1").arg(tiledata_->name().data());
	} else {
		message = tiledata_->name();
	}

	WorldView->addSysMessage(message);

	message = QString("[Id: 0x%1, Z: %2, Stretch: %4,%5,%6]").arg(id_, 0, 16).arg(z_).arg(left).arg(bottom).arg(right);
	WorldView->addSysMessage(message);
}

void cGroundTile::updatePriority() {
	//priority_ = averagez_;
	priority_ = z_;
}
