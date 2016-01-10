#include <QDebug>
#include <QColor>
#include <QFont>
#include <Qsci/qsciscintilla.h>
#include <Qsci/qscistyle.h>
#include "rcadscilexer.h"

RCadSciLexer::RCadSciLexer( QObject *parent ) : QsciLexerCustom( parent )
{
}

RCadSciLexer::~RCadSciLexer()
{
}

const char* RCadSciLexer::language() const
{
	return "rcad";
}

QString RCadSciLexer::description( int style ) const
{
	switch( style )
	{
	case Default:
		return "Default";
	case Comment:
		return "Comment";
	}

	return QString();
}

void RCadSciLexer::styleText( int start, int end )
{
	QString source;
	int i;

	if( !editor() )
		return;

	qDebug() << __FUNCTION__ << "start =" << start << " end =" << end;

	int style = editor()->SendScintilla( QsciScintillaBase::SCI_GETSTYLEAT, start );

	char *chars = (char *)malloc( end - start + 1 );
	editor()->SendScintilla( QsciScintilla::SCI_GETTEXTRANGE, start, end, chars );
	source = QString( chars );
	free( chars );

	qDebug() << "source =" << source;

	startStyling( start, 0x1f );
	QStringList list = source.split( "\n" );
	for( i = 0; i < list.size(); i++ )
	{
		QString line = list.at( i );
		int len = line.size();
		int style;
		qDebug() << "line =" << line;

		if( line.startsWith( "//" ) )
		{
			style = Comment;
		}
		else
		{
			style = Default;
		}
		qDebug() << "Styling " << len << "bytes " << description( style );
		setStyling( len, getStyle( style ) );

	}
}

QColor RCadSciLexer::defaultColor( int style ) const
{
	switch( style )
	{
	case Default:
		return QColor( 0xFF, 0x0, 0x0 );
	case Comment:
		return QColor( 0x0, 0xFF, 0x0 );
	default:
		return QColor( 0x0, 0x00, 0xFF );
	}
	return QsciLexer::defaultColor( style );
}

QFont  RCadSciLexer::defaultFont( int style ) const
{
	return QFont( "Courier New", 10 );
}

QColor RCadSciLexer::defaultPaper( int style ) const
{
	return QsciLexer::defaultPaper( style );
}

QsciStyle RCadSciLexer::getStyle( int style )
{
	if( style < MaxStyle )
	{
		return QsciStyle( style, description( style ), defaultColor( style ),
						  defaultPaper( style ), defaultFont( style ) );
	}
	else
	{
		return QsciStyle( style );
	}
}
