
#include "config.h"
#include "engine.h"
#include "gui/gui.h"
#include "gui/worldview.h"
#include "game/world.h"
#include "log.h"
#include "utilities.h"
#include "exceptions.h"
#include "muls/textures.h"
#include "muls/gumpart.h"
#include "texture.h"

#include <qdatetime.h>
#include <qimage.h>

cEngine::cEngine() {
	screen = 0;
	changed = true;
	lockSize_ = false;
	screenshot_ = false;
	pixels = 0;
}

cEngine::~cEngine() {
	delete [] pixels;
}

void cEngine::screenshot(const QString &filename) {
	screenshot_ = true;
	screenshotFilename = filename;
}

void cEngine::load() {
	// Load the settings from the configuration file
	width_ = Config->engineWidth();
	height_ = Config->engineHeight();
	windowed_ = Config->engineWindowed();

	// Log the settings
	Log->print(LOG_MESSAGE, tr("Setting up engine with size %1x%2 (%3)\n").arg(width_).arg(height_).arg(windowed_ ? tr("Windowed") : tr("FullScreen")));

	const SDL_VideoInfo *videoInfo = SDL_GetVideoInfo();

	// Erstellen von OPENGL surface
	unsigned int flags = SDL_OPENGL|SDL_DOUBLEBUF;

	if (!Engine->windowed()) {
		flags |= SDL_FULLSCREEN;
	}

	screen = SDL_SetVideoMode(width_, height_, 0, flags);	
	pixels = new unsigned char[width_ * height_ * 4]; // Create the temporary buffer for our pixel data

	// Initialize OpenGL
	glViewport(0, 0, width_, height_); // Tell OpenGL the size of our window
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f); // Fill the screen with a black color
	glEnable(GL_TEXTURE_2D);
	glDisable(GL_DITHER);
	// TODO: Disable mip mapping
	glEnable(GL_BLEND); // Enable aplha blending of textures
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glShadeModel(GL_SMOOTH); // Go with flat shading for now
	glEnable(GL_NORMALIZE);

	// Set up texture parameters
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
    //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	
	// Set up lighting
	glEnable(GL_LIGHT0);

	GLfloat light_position[] = { -1.0f, -1.0f, 0.5f, 0.0f };
	//GLfloat light_position[] = { 100.0f, -30.0f, 23.0f, 0.0f };
	glLightfv(GL_LIGHT0, GL_POSITION, light_position);

	float specular[] = {2.0f, 2.0f, 2.0f, 1.0f};
	glLightfv(GL_LIGHT0, GL_AMBIENT, specular);

	GLfloat lmodel_ambient[] = { 1.0f, 1.0f, 1.0f, 1.0f };
	glLightModelfv(GL_LIGHT_MODEL_AMBIENT, lmodel_ambient);
	glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, GL_FALSE);

	// Reset the projection matrix
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

	// We're not using real perspectiveb
	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);

	// Set a very simple projection matrix
	gluOrtho2D (0, width_, height_, 0);

	// Switch back to the model view matrix
	glMatrixMode(GL_MODELVIEW);

	if (screen == 0) {
		throw Exception(tr("Unable to initialize SDL Video: %1").arg(SDL_GetError()));
	}

	// Side the cursor
	SDL_ShowCursor(SDL_DISABLE);

	// Set the desired caption of the window
	SDL_WM_SetCaption(tr("Ultima Online").latin1(), 0);

	// Notify the GUI of the change
	Gui->setBounds(0, 0, width_, height_); // Full size
}

void cEngine::resize(unsigned int width, unsigned int height) {
/*	width_ = width;
	height_ = height;
	Config->setEngineHeight(height);
	Config->setEngineWidth(width);

	// If we're not windowed, re-setup the entire thing
	if (!windowed_) {
		reload();
	} else {
		screen = SDL_SetVideoMode(width_, height_, 32, SDL_HWSURFACE|SDL_ANYFORMAT|SDL_RESIZABLE|SDL_DOUBLEBUF); // Initialize the Video Engine

		if (screen == 0) {
			throw Exception(tr("Unable to initialize SDL Video: %1").arg(SDL_GetError()));
		}

		SDL_FillRect(screen, 0, SDL_MapRGB(screen->format, 255, 255, 255));
	}

	delete [] pixels;
	pixels = new unsigned char[width_ * height_ * 4]; // Create the temporary buffer for our pixel data

	// Notify the GUI of the change
	Gui->setBounds(0, 0, width_, height_); // Full size*/
}

void cEngine::unload() {
	Log->print(LOG_MESSAGE, "Stopping Engine\n");
}

void cEngine::reload() {
	unload();
	load();
}

void cEngine::createScreenshot(const QString &filename) {
	// Read the framebuffer data
	glReadPixels(0, 0, width_, height_, GL_RGBA, GL_UNSIGNED_BYTE, pixels);

	QImage image(width_, height_, 32, 0);
	
	unsigned char *ptr = pixels;

	// We have to flip the framebuffer data here
	for (int y = height_ - 1; y >= 0; --y) {
		for (unsigned int x = 0; x < width_; ++x) {
			image.setPixel(x, y, qRgb(ptr[0], ptr[1], ptr[2]));
			ptr += 4;
		}
	}

	// Save the image as JPG
	if (!image.save(filename, "JPEG")) {
		if (WorldView) {
			WorldView->addSysMessage(tr("Unable to save screenshot as %1.").arg(filename));
		}
	} else {
		if (WorldView) {
			WorldView->addSysMessage(tr("Screenshot saved as %1.").arg(filename));
		}
	}
}

void cEngine::poll() {
	// FPS counter
	static unsigned int lastswap_start = 0;
	static unsigned int lastswap_count = 0;

	// Swap the buffers
	SDL_GL_SwapBuffers();

	// Read the framebuffer if there's a screenshot tobe made
	if (screenshot_) {
		if (screenshotFilename.isEmpty()) {
			QDateTime current = QDateTime::currentDateTime();
			screenshotFilename = QString("screenshot-%1.jpg").arg(current.toString( "yyyyMMdd-hhmmss" ));
		}

		createScreenshot(screenshotFilename);
		screenshot_ = false;
	}

	if (lastswap_start + 1000 < SDL_GetTicks()) {
		SDL_WM_SetCaption(tr("Ultima Online %1 fps (%1,%2,%3,%4)").arg(lastswap_count).arg(World->x()).arg(World->y()).arg(World->z()).arg(World->facet()).latin1(), 0);
		lastswap_count = 0;
		lastswap_start = SDL_GetTicks();
	} else {
		++lastswap_count;
	}
}

void cEngine::drawPixel(int x, int y, unsigned char r, unsigned char g, unsigned char b, unsigned char a) {
	// Convert to screen pixel format
	SDL_PixelFormat *format = screen->format;
	unsigned int pixel;
	if (a != 255) {
		pixel = SDL_MapRGBA(format, r, g, b, a);
	} else {
		pixel = SDL_MapRGB(format, r, g, b);
	}

	// Calculate the offset
	unsigned char *ptr = (unsigned char*)screen->pixels + (screen->pitch * y) + (x * format->BytesPerPixel);

	switch (format->BytesPerPixel) {
		case 4:
			*(unsigned int*)ptr = pixel;
			break;
		case 2:
			*(unsigned short*)ptr = pixel;
			break;
		default:
			throw Exception(tr("Invalid bytes per pixel value: %1").arg(format->BytesPerPixel));
	}
}

void cEngine::drawSurface(int x, int y, SDL_Surface *surface, SDL_Rect *srcrect) {
	if (!surface) {
		return; // Dont draw invalid surfaces
	}

	SDL_Rect dest;
	dest.x = x;
	dest.y = y;

	SDL_BlitSurface(surface, srcrect, screen, &dest);
}

void cEngine::invertPixel(int x, int y) {
}

void cEngine::getPixel(int x, int y, unsigned char &r, unsigned char &g, unsigned char &b) {
}

cEngine *Engine;
