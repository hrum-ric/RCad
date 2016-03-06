#include "mainwindow.h"
#include <QApplication>
#include <QCommandLineParser>
#include <QFile>

#include "Parser.h"
void test();

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

	test();
	return 0;

// 
//     MainWindow w;
// 
// 	if( QFile::exists( parser.value( "l" ) ) )
// 		w.LoadProject( parser.value( "l" ) );
// 
// 	w.show();
// 
//     return app.exec();
}

void test()
{
	RCADParser parser;

	QFile file( "C:\\Users\\romaric\\RCadTest\\main.rcad" );
	file.open( QFile::ReadOnly );
	QByteArray data = file.readAll();

	parser.Parse( "main.rcad", (const unsigned char*)data.constData(), (const unsigned char*)data.constEnd() );


}