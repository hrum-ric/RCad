#ifndef __CODEEDITOR__
#define __CODEEDITOR__
#include <Qsci/qsciscintilla.h>
#include <QSaveFile>

class CodeEditor : public QsciScintilla
{
	Q_OBJECT
public:
	CodeEditor( QString name, QString fileName );
	~CodeEditor();

	QString filename() const { return fileName;  }
	// load text from file
	bool bRead( bool askCreate, bool bShowError );
	// save to file
	bool bSave( QString& errorString );

private:
	void updateTitle( bool bModified );

private:
	QString fileName;
};

#endif