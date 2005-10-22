
#if !defined(__GUI_H__)
#define __GUI_H__

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

class cGui {
private:
	HBRUSH whiteBrush;
	HBRUSH grayBrush;
	HFONT dialogFont;
	HFONT topFont;
	HWND topLabel;

protected:
	unsigned short clientWidth_;
	unsigned short clientHeight_;
	unsigned short width_;
	unsigned short height_;
	HWND window_;
	HWND progressBar_;
	HWND label_;
	HINSTANCE instance_;
	bool running_;

	bool registerClass();

	// Event handling members
	LRESULT onCreate(HWND hwnd, WPARAM wparam, LPARAM lparam);
	LRESULT onDestroy(HWND hwnd, WPARAM wparam, LPARAM lparam);
	LRESULT onPaint(HWND hwnd, WPARAM wparam, LPARAM lparam);
	LRESULT onCommand(HWND hwnd, WPARAM wparam, LPARAM lparam);
	LRESULT onCtlColorStatic(HWND hwnd, WPARAM wparam, LPARAM lparam);
public:
	cGui(HINSTANCE instance);
	~cGui();

	HWND window() const;
	HWND progressBar() const;
	HWND label() const;
	HINSTANCE instance() const;
	bool isRunning() const;
	unsigned short clientWidth() const;
	unsigned short clientHeight() const;
	unsigned short width() const;
	unsigned short height() const;

	// Initialize the GUI (register class, etc.)
	bool initialize();
    
	// Show the main window
	void show();

	// Hide the main window
	void hide();

	// Process pending window messages
	void processMessages();

	// Process and dispatch a window message
	LRESULT processMessage(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam);

	// Set the range of the progress bar
	void setProgressRange(unsigned int min, unsigned int max);

	// Set the progress position
	void setProgressValue(unsigned int value);

	// Set the text of the status label
	void setStatusText(const char *text);
};

inline unsigned short cGui::clientWidth() const {
	return clientWidth_;
}

inline unsigned short cGui::clientHeight() const {
	return clientHeight_;
}

inline unsigned short cGui::width() const {
	return width_;
}

inline unsigned short cGui::height() const {
	return height_;
}

inline HWND cGui::window() const {
	return window_;
}

inline HWND cGui::progressBar() const {
	return progressBar_;
}

inline HWND cGui::label() const {
	return label_;
}

inline HINSTANCE cGui::instance() const {
	return instance_;
}

inline bool cGui::isRunning() const {
	return running_;
}

extern cGui *Gui;

#endif
