#include "codeeditor.h"
#include "rcadscilexer.h"
#include <QFileInfo>
#include <QMessageBox>

CodeEditor::CodeEditor( QString name, QString fileName ) : QsciScintilla()
{
	setObjectName( name );
	CodeEditor::fileName = fileName;
	setWindowTitle(QFileInfo(fileName).baseName()+" [*]");
	connect( this, &QsciScintilla::modificationChanged, this, &CodeEditor::updateTitle );
	setAttribute( Qt::WA_DeleteOnClose );

	setLexer( new RCadSciLexer() );
}

CodeEditor::~CodeEditor()
{
	qDebug("~CodeEditor");
}

void CodeEditor::updateTitle( bool )
{
	if( isModified() )	setWindowTitle( QFileInfo( fileName ).baseName() + " [*]" );
	else				setWindowTitle( QFileInfo( fileName ).baseName() );
}

bool CodeEditor::bRead( bool askCreate, bool bShowError )
{
	QFile::OpenMode mode = QFile::ReadOnly;
	QFile file(fileName);
	if (!file.exists())
	{
		if( !askCreate )
		{
			if( bShowError ) QMessageBox::critical( this, tr( "file does not exist" ), tr( "The file %1 does not exist." ).arg( fileName ) );
			return false;
		}
		if (QMessageBox::question(this, tr("File does not exist"), tr("The file %1 does not exist.\nDo you want to create it.").arg(fileName)) == QMessageBox::No)
			return false;
		mode = QFile::ReadWrite;
	}
	if (!file.open(mode))
	{
		if( bShowError )
		{
			if( mode == QFile::ReadWrite ) QMessageBox::critical( this, tr( "fail to create the file" ), tr( "The creation of the file %1 fail.\n%2" ).arg( fileName ).arg( file.errorString() ) );
			else						  QMessageBox::critical( this, tr( "fail to read the file" ), tr( "Reading the file %1 fail.\n%2" ).arg( fileName ).arg( file.errorString() ) );
		}
		return false;
	}
	if (!QsciScintilla::read(&file))
	{
		if( bShowError ) QMessageBox::critical( this, tr( "fail to read the file" ), tr( "Reading the file %1 fail.\n%2" ).arg( fileName ).arg( file.errorString() ) );
		return false;
	}
	setModified( false );
	return true;
}

// save to file
bool CodeEditor::bSave( QString& errorString )
{
	QSaveFile file( fileName );
	if( !file.open( QIODevice::WriteOnly ) )
	{
		errorString = file.errorString();
		return false;
	}

	if( !QsciScintilla::write( &file ) )
	{
		errorString = file.errorString();
		return false;
	}

	if( !file.commit() )
	{
		errorString = file.errorString();
		return false;
	}
	setModified( false );
	return true;
}