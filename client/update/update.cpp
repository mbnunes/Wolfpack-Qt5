
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <vector>
#include <string>
#include <direct.h>

#include "gui.h"

void compileFileList(std::string basePath, std::string subDirectory, std::vector<std::string> &filenames, std::vector<std::string> &directories, size_t &fullSize) {
	std::string temp = basePath;
	if (!subDirectory.empty()) {
		temp.append("\\");
		temp.append(subDirectory);
	}
	temp.append("\\*");

	// Find all files
	WIN32_FIND_DATA info;
	HANDLE search = FindFirstFile(temp.c_str(), &info);

	if (search == INVALID_HANDLE_VALUE) {
		return;
	}

	do {
		if ((info.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == 0) {
			std::string relativePath;
			if (!subDirectory.empty()) {
				relativePath = subDirectory;
				relativePath.append("\\");
			}
			relativePath.append(info.cFileName);
			filenames.push_back(relativePath);
			fullSize += info.nFileSizeLow;
		} else {
			if (!strcmp(info.cFileName, ".") || !strcmp(info.cFileName, "..")) {
				continue;
			}

			// Entry is a directory
			std::string temp;
			if (!subDirectory.empty()) {
				temp = subDirectory;
				temp.append("\\");
			} 
			temp.append(info.cFileName);

			directories.push_back(temp);
			compileFileList(basePath, temp, filenames, directories, fullSize);
		}
	} while(FindNextFile(search, &info));
    
	FindClose(search);
}

void launchClient(std::string path) {
	std::string launchPath = path;
	launchPath.append("\\uoclient.exe");

	STARTUPINFO si;
	ZeroMemory(&si, sizeof(si));
	si.wShowWindow = SW_NORMAL;

	PROCESS_INFORMATION pi;
	CreateProcess(launchPath.c_str(), "", 0, 0, FALSE, 0, 0, path.c_str(), &si, &pi);
}

bool fileExists(std::string path) {
	WIN32_FIND_DATA info;
	HANDLE handle = FindFirstFile(path.c_str(), &info);

	if (handle == INVALID_HANDLE_VALUE) {
		return false;
	} else {
		FindClose(handle);
		return true;
	}
}

void copyFiles() {
	// Get our path
	char szOurPath[MAX_PATH];
	GetModuleFileName(0, szOurPath, MAX_PATH);
	*strrchr(szOurPath, '\\') = 0;
	std::string ourPath = szOurPath;
	std::string searchPath = ourPath;
	searchPath.append("\\update");
	
	if (!fileExists(searchPath.c_str())) {
		launchClient(ourPath);
		return;
	}

	// List of files to copy
	std::vector<std::string> files;
	std::vector<std::string> directories;
	size_t fullSize = 0;

	compileFileList(searchPath, "", files, directories, fullSize);

	char status[MAX_PATH*2];

	// Create directories
	for (size_t i = 0; i < directories.size(); ++i) {
		sprintf(status, "Creating directory %s...", directories[i].c_str());
		Gui->setStatusText(status);
		Gui->processMessages();

		std::string fullpath = ourPath;
		fullpath.append("\\");
		fullpath.append(directories[i]);		
		mkdir(fullpath.c_str());		
	}

	Gui->setProgressRange(0, (int)fullSize);
    
	// Start copying files
	size_t currentProgress = 0;
	for (size_t i = 0; i < files.size(); ++i) {
		sprintf(status, "Copying %s...", files[i].c_str());
		Gui->setStatusText(status);
		Gui->processMessages();

		std::string realPath = searchPath;
		realPath.append("\\");
		realPath.append(files[i]);

		FILE *fp = fopen(realPath.c_str(), "rb");
		
		if (!fp) {
			sprintf(status, "Unable to open file %s.", realPath.c_str());
			MessageBox(0, status, "Copying Error", MB_OK|MB_ICONERROR);
			continue;
		}

		realPath = ourPath;
		realPath.append("\\");
		realPath.append(files[i]);

		FILE *fpOut = fopen(realPath.c_str(), "wb");

		if (!fpOut) {
			fclose(fp);
			sprintf(status, "Unable to create file %s.", realPath.c_str());
			MessageBox(0, status, "Copying Error", MB_OK|MB_ICONERROR);
			continue;
		}

		size_t readSize;
		size_t offset = 0;
		char buffer[4096];
		while (true) {
			readSize = fread(buffer + offset, 1, 4096, fp);
			Gui->processMessages();
			fwrite(buffer + offset, 1, readSize, fpOut);
            
			currentProgress += readSize;
			Gui->setProgressValue((int)currentProgress);
			Gui->processMessages();

			if (readSize != 4096) {
				break;
			}
		}

		fclose(fpOut);
		fclose(fp);

		// The file was correctly copied, delete it now
		realPath = searchPath;
		realPath.append("\\");
		realPath.append(files[i]);

		DeleteFile(realPath.c_str());
	}

	// Iterate from end to beginning trough created directories and
	// remove them.
	for (int i = (int)directories.size() - 1; i >= 0; --i) {
		std::string realPath = searchPath;
		realPath.append("\\");
		realPath.append(directories[i]);

		rmdir(realPath.c_str());
	}

	rmdir(searchPath.c_str());

	launchClient(ourPath);
}

int CALLBACK WinMain(HINSTANCE instance, HINSTANCE prevInstance, LPSTR cmdline, int cmdShow) {
	Gui = new cGui(instance);

	Gui->initialize();

	Gui->show();
	Gui->processMessages(); // Make sure it's visible

	HANDLE mutex;
	bool doUpdate = true;

	CreateMutex(0, TRUE, "UOCLIENT_MUTEX");

	// Wait for the patcher to exit (if neccesary)
	if ((mutex = OpenMutex(MUTEX_ALL_ACCESS, FALSE, "UOCLIENT_MUTEX"))) {
		Gui->setStatusText("Waiting for the client to close...");
		Gui->processMessages();

		unsigned int totalWait = 0;
		// we timed out...
		while (WaitForSingleObject(mutex, 100) == WAIT_TIMEOUT) {
			totalWait += 100;
			if (totalWait > 5000) {
				MessageBox(0, "The client is still running, please close the client and run it again.", "Error", MB_OK|MB_ICONERROR);
				doUpdate = false;
				break;
			}
			Gui->processMessages();
		}
		CloseHandle(mutex);
	}

	if (doUpdate) {
		copyFiles();
	}

	Gui->hide();

	delete Gui;

	return 0;
}
