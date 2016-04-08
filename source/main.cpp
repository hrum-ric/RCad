#include "mainwindow.h"
#include <QApplication>
#include <QCommandLineParser>
#include <QFile>


int main(int argc, char *argv[])
{
	QCoreApplication::setOrganizationName("hrum");
	QCoreApplication::setApplicationName("RCad");
	QCoreApplication::setApplicationVersion( "1.0" );
	QApplication app( argc, argv );

	// command line
	QCommandLineParser parser;
	parser.setApplicationDescription( QCoreApplication::translate( "main", "textual CAD editor and render" ) );
	parser.addHelpOption();
	parser.addVersionOption();
	parser.addOptions( {
		{ { "l", "load-project" }, QCoreApplication::translate( "main", "load the project <project>." ), QCoreApplication::translate( "main", "project" ) },
	} );
	parser.process( app );

    MainWindow w;

	if( QFile::exists( parser.value( "l" ) ) )
		w.LoadProject( parser.value( "l" ) );

	w.show();

    return app.exec();
}

