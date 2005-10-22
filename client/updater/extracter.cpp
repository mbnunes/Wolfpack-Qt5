
#include "updater.h"
#include "log.h"
#include "utilities.h"
#include "../version.h"

#include <QLibrary>
#include <QVarLengthArray>
#include <QMessageBox>

#define INITGUID
#include "../7Zip/7zip/archive/7z/7zhandler.h"

typedef HRESULT (STDAPICALLTYPE *fnCreateObject)(const GUID *classID, const GUID *interfaceID, void **outObject);
fnCreateObject CreateObject = 0;

// Create an in-stream subclass for the archive reader
class QFileInStream : public IInStream, public IStreamGetSize, public CMyUnknownImp {
public:
  QFile file;
  QFileInStream() {}
  virtual ~QFileInStream() {}

  bool Open(LPCTSTR fileName) {
	file.setFileName(fileName);
	return file.open(QIODevice::ReadOnly);
  }

  bool Open(LPCWSTR fileName) {
	file.setFileName(QString::fromUtf16(fileName));
	return file.open(QIODevice::ReadOnly);
  }

  MY_UNKNOWN_IMP2(IInStream, IStreamGetSize)

	  STDMETHOD(Read)(void *data, UInt32 size, UInt32 *processedSize) {
			qint64 result = file.read((char*)data, size);

			if (processedSize) {
				*processedSize = result;
			}

			return (result == 0) ? E_FAIL : S_OK;
	  }

	  STDMETHOD(Seek)(Int64 offset, UInt32 seekOrigin, UInt64 *newPosition) {
		if(seekOrigin >= 3)
			return STG_E_INVALIDFUNCTION;

		switch (seekOrigin) {
			case 0:				
				break;
			case 1:
				offset = file.pos() + offset;
				break;
			case 2:
				offset = file.size() - offset;
				break;
		}

		bool result = file.seek(offset);

		if (newPosition) {
			*newPosition = file.pos();
		}

		return result ? S_OK : E_FAIL;
	  }

	  STDMETHOD(GetSize)(UInt64 *size) {
		  *size = file.size();
		  return S_OK;
	  }
};

class QByteArrayOutStream : public ISequentialOutStream, public CMyUnknownImp {
protected:
	QByteArray *array;
public:
	MY_UNKNOWN_IMP

	QByteArrayOutStream(QByteArray *ptr) {
		array = ptr;
	}

	~QByteArrayOutStream() {
		array = 0;
	}

	STDMETHODIMP Write(const void *data, UInt32 size, UInt32 *processedSize) {
		array->append(QByteArray((char*)data, size));
		if (processedSize) {
			*processedSize = size;
		}
		return S_OK;
	}

	STDMETHODIMP WritePart(const void *data, UInt32 size, UInt32 *processedSize) {
		array->append(QByteArray((char*)data, size));
		if (processedSize) {
			*processedSize = size;
		}
		return S_OK;
	}
};

class cExtractCallback : public IArchiveExtractCallback, public CMyUnknownImp {
protected:
	QByteArray array;
	QString currentName;
	QDir destination;
	cExtractionDialog *dialog;
	IInArchive *handler;
	qint64 totalsize;
	QString error_;
	QStringList extracted_;
public:
	MY_UNKNOWN_IMP

	QString error() const { return error_; }
	void setError(QString error) { error_ = error; }

	// List of files extracted
	QStringList extracted() const { return extracted_; }

	cExtractCallback(QDir destination, IInArchive *handler, cExtractionDialog *dialog) {
		this->handler = handler;
		this->destination = destination;
		this->dialog = dialog;
	}

	// IProgress
	STDMETHODIMP SetTotal(UInt64 size) {
		if (dialog) {
			dialog->setTotal(size);
			dialog->setWindowTitle(tr("Extracting update (0%)"));
			qApp->processEvents();
			totalsize = size;
		}
		return S_OK;
	}

	STDMETHODIMP SetCompleted(const UInt64 *completeValue) {
		if (dialog) {
			dialog->setCurrent(*completeValue);
			uint percentage = 0;
			if (*completeValue > 0) {
				percentage = qMin<int>(100, ((*completeValue) / (double)totalsize) * 100);				
			}
			dialog->setWindowTitle(tr("Extracting update (%1%)").arg(percentage));
			qApp->processEvents();
		}
		return S_OK;
	}

	// IExtractCallBack
	STDMETHODIMP GetStream(UInt32 anIndex, ISequentialOutStream **outStream, 
		Int32 askExtractMode) {
			if (dialog && !dialog->isVisible()) {
				return E_FAIL;
			}

			if (askExtractMode == NArchive::NExtract::NAskMode::kExtract) {
				// Retrieve the filename for the given index
				NWindows::NCOM::CPropVariant prop;
				HRESULT result = handler->GetProperty(anIndex, kpidPath, &prop);
				if (result == S_OK && prop.vt == VT_BSTR) {
					currentName = QString::fromUtf16(prop.bstrVal);

					if (dialog) {
						qint64 size = 0;
						result = handler->GetProperty(anIndex, kpidSize, &prop);
						if (result == S_OK && prop.vt == VT_UI8) {
							size = prop.uhVal.QuadPart;
						}
						size /= 1024;

						if (size != 0) {
							dialog->setText(tr("Extracting %1 (%2 kb)...").arg(currentName).arg(size));
						} else {
							dialog->setText(tr("Extracting %1...").arg(currentName));
						}
						qApp->processEvents();
					}
				}
				*outStream = new QByteArrayOutStream(&array);
				(*outStream)->AddRef();
			}
			else
			{
				currentName.clear();
				*outStream = NULL;
			}
			return S_OK;
		}

	STDMETHOD(PrepareOperation)(Int32 askExtractMode) {
		return S_OK;
	}

	STDMETHOD(SetOperationResult)(Int32 resultEOperationResult) {
		// Only save to file if extraction was successful
		if (resultEOperationResult == S_OK) {
			// Write the file to disk
			QFile output(destination.absoluteFilePath(currentName));

			destination.mkpath(QFileInfo(currentName).path());
	        
			if (output.open(QIODevice::WriteOnly|QIODevice::Truncate)) {
				extracted_.append(output.fileName());
				Log->print(LOG_MESSAGE, tr("Writing %1 (%2 byte) to disk.\n").arg(currentName).arg(array.size()));
				output.write(array);
				output.close();
			} else {
				setError(tr("Unable to open file %1 for writing.").arg(output.fileName()));
				Log->print(LOG_ERROR, error() + "\n");
				return E_FAIL;
			}
		} else {
			setError(tr("Error extracting %1: %2.").arg(currentName).arg(resultEOperationResult));
			Log->print(LOG_ERROR, error() + "\n");
			return E_FAIL;
		}
		array.clear();
		return S_OK;
	}
};

// Static-SFX (for Linux) can be big
const UInt64 kMaxCheckStartPosition = 
#ifdef _WIN32
1 << 20;
#else
1 << 22;
#endif

// This extracts data for the given files
/*
	This is more or less a port of the LZMA C Extractor Code.
	You can find LZMA/7Zip here:
    http://www.7-zip.org/
	Donate if you can. It's a _very_ good compression algorithm.
*/
bool cUpdater::extractData(QString archive, QDir destination) {
	// Check for 7Zip extraction library
	QLibrary library("7zip");

#if !defined(NDEBUG)
	// Use debug library in debug mode
	library.setFileName("7zipd");
#endif

	if (!library.load()) {
		Log->print(LOG_ERROR, tr("Unable to load extraction library %1.\n").arg(library.fileName()));
		return false;
	}

	// Get the handle to the CreateObject function
	CreateObject = (fnCreateObject)library.resolve("CreateObject");

	if (!CreateObject) {
		Log->print(LOG_ERROR, tr("%1 does not export CreateObject.\n").arg(library.fileName()));
		library.unload();
		return false;
	}

	IInArchive *handler;
	HRESULT result;

	// Create the 7z Handler
	result = CreateObject(&NArchive::N7z::CLSID_CFormat7z, &IID_IInArchive, (void**)&handler);

	if (result != S_OK) {
		Log->print(LOG_ERROR, tr("Unable to create 7ZipHandler object: %1.\n").arg(result));
		library.unload();
		return false;
	}

	// Create the file object
	QFileInStream *stream = new QFileInStream;
	stream->Open(archive.toLatin1());

	result = handler->Open(stream, &kMaxCheckStartPosition, 0);	

	if (result == S_OK) {
		uint itemCount;
		handler->GetNumberOfItems(&itemCount);
        
		QVarLengthArray<uint, 512> fileIds; // Reserve space for 512 file ids
		uint numFileIds = 0;

		cExtractionDialog *dialog = new cExtractionDialog;
		dialog->show();

		qApp->setQuitOnLastWindowClosed(false);
		qApp->processEvents();

		// List of directories we created
		QStringList createdDirectories;
	
		// Search for extractable file ids and create subdirectories while we're at it
		for (uint i = 0; i < itemCount; ++i) {
			QString name;
			bool isFolder;
			NWindows::NCOM::CPropVariant prop;

			result = handler->GetProperty(i, kpidPath, &prop);
			if (result == S_OK && prop.vt == VT_BSTR) {
				name = QString::fromUtf16(prop.bstrVal);
			} else {
				continue;
			}
            
			result = handler->GetProperty(i, kpidIsFolder, &prop);
			if (result == S_OK && prop.vt == VT_BOOL) {
				isFolder = prop.boolVal;
			} else {
				continue;
			}

			if (isFolder) {
				Log->print(LOG_MESSAGE, tr("Creating directory %1...\n").arg(name));
				if (!destination.exists(name)) {
					if (!destination.mkpath(name)) {
						QMessageBox::critical(dialog, "Error", tr("Cannot create directory %1.").arg(name), QMessageBox::Ok, QMessageBox::NoButton);
						result = E_FAIL;
						break;
					}
					createdDirectories.prepend(name);
				}
				continue;
			}

			fileIds.append(i);
		}

		if (result == S_OK) {
			// Extract them all at once
			if (!fileIds.isEmpty()) {
				cExtractCallback *callback = new cExtractCallback(destination, handler, dialog);
				callback->AddRef();
				result = handler->Extract(fileIds.data(), fileIds.count(), 0, callback);
				if (result != S_OK) {
					// Roll back
					foreach (QString filename, callback->extracted()) {
						destination.remove(filename);
					}

					// Only show an error if there was any
					if (!callback->error().isEmpty()) {
						QMessageBox::critical(dialog, tr("Extraction error"), callback->error());
					}
				}
				callback->Release();
			}
		}

		// Roll back created directories here
		if (result != S_OK) {
			foreach (QString directory, createdDirectories) {
				destination.rmpath(directory);
			}
		}

		qApp->setQuitOnLastWindowClosed(true);
		delete dialog;
	} else {
		Log->print(LOG_ERROR, tr("Unable to process archive %1: %2.\n").arg(archive).arg(result));
	}
   
	// Clean up behind us
	handler->Release();
	library.unload();

	return (result == S_OK);
}

cExtractionDialog::cExtractionDialog(QWidget *parent) {
	setupUi(this);
}

void cExtractionDialog::setCurrent(uint progress) {
	progressBar->setValue(progress);
}

void cExtractionDialog::setTotal(uint progress) {
	progressBar->setRange(0, progress);
}

void cExtractionDialog::setText(QString text) {
	label->setText(text);
}
