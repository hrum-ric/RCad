#include <QtWidgets/QMessageBox>
#include <QtWidgets/QFileDialog>
#include <QtGui/QCloseEvent>
#include <QtCore/QSettings>
#include <QtWidgets/QAction>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QMenu>
#include <QtWidgets/QMenuBar>
#include <QtCore/QPropertyAnimation>

#include "RCadConstants.h"
#include "mainwindow.h"
#include "project.h"
#include "newfiledialog.h"
#include "codeeditor.h"

MainWindow::MainWindow() : QTabFramework()
{
	__CreateMenu();
	__CreateProjectView();
	__restoreSettings(); 
	__initRecentFileActions();
	__upadateRecentFileActions(nullptr);
	
	QMetaObject::connectSlotsByName(this);
}

MainWindow::~MainWindow()
{
}

void MainWindow::__CreateMenu()
{
	QMenuBar* menuBar = new QMenuBar(this);
	setMenuBar(menuBar);

	// File
	QMenu* FileMenu = new QMenu(tr("&File"),menuBar);
	menuBar->addAction(FileMenu->menuAction());
	// File/New
	QAction* NewProjectAction = new QAction(tr("New project..."), this);
	NewProjectAction->setObjectName("NewProject");
	FileMenu->addAction(NewProjectAction);
	// File/Open
	QAction* OpenProjectAction = new QAction(tr("Open project..."), this);
	OpenProjectAction->setObjectName("OpenProject");
	FileMenu->addAction(OpenProjectAction);
	// File/Recent
	RecentProjectMenu = new QMenu(tr("Recent project"), FileMenu);
	FileMenu->addAction(RecentProjectMenu->menuAction());
	// File/Save all
	QAction* SaveAllAction = new QAction(tr("Save all"), this);
	SaveAllAction->setObjectName( "SaveAll" );
	FileMenu->addAction(SaveAllAction);
	// File/Close project
	CloseProjectAction = new QAction(tr("Close project"), this);
	CloseProjectAction->setEnabled(false);
	CloseProjectAction->setObjectName( "CloseProject" );
	FileMenu->addAction(CloseProjectAction);
	FileMenu->addSeparator();
	// File/Exit
	QAction* ExitAction = new QAction(tr("Exit"), this);
	ExitAction->setObjectName( "Exit" );
	FileMenu->addAction(ExitAction);

	// View
	QMenu* ViewMenu = new QMenu( tr( "&View" ), menuBar );
	menuBar->addAction( ViewMenu->menuAction() );
	// View/Project
	QAction* ViewProjectAction = new QAction( tr( "Project explorer" ), this );
	ViewProjectAction->setObjectName( "ViewProject" );
	ViewMenu->addAction( ViewProjectAction );

	// Edit
	QMenu* EditMenu = new QMenu(tr("&Edit"),menuBar);
	menuBar->addAction(EditMenu->menuAction());
}

void MainWindow::__CreateProjectView()
{
	ProjectTree = new QTreeView();
	ProjectTree->setObjectName( PROJECT_TREEVIEW_NAME );
	ProjectTree->setWindowTitle(tr("Project"));
	ProjectTree->setRootIsDecorated(true);
	ProjectTree->setContextMenuPolicy(Qt::CustomContextMenu);
	ProjectTree->setEditTriggers(QAbstractItemView::NoEditTriggers);
	ProjectTree->setDragDropMode( QAbstractItemView::InternalMove );
	ProjectTree->header()->setDefaultSectionSize(0);
	addTab(ProjectTree,QTabFramework::InsertLeft);

}

void MainWindow::__restoreSettings()
{
	QSettings settings;
	restoreGeometry(settings.value("geometry").toByteArray());
	restoreState(settings.value("windowState").toByteArray());
}

// get the list of all modified files that hadn't been saved
QStringList MainWindow::__getMidifiedFileList()
{
	QStringList modifiedFiles;
	if( project->isModified() ) modifiedFiles.append( project->filename() );
	QList<CodeEditor*> editorList = findWidget<CodeEditor>();
	for( auto code : editorList )
	{ 
		if( code->isModified() )
			modifiedFiles.append( code->filename() );
	}
	return modifiedFiles;
}

// close the project. Return false if the action must be canceled
bool MainWindow::__bCloseProject()
{
    if( project )
    {
		QStringList modifiedFiles = __getMidifiedFileList();
		if( !modifiedFiles.isEmpty() )
        {

            QMessageBox msgBox;
            msgBox.setText("The project has been modified.");
            msgBox.setInformativeText("Do you want to save your changes?");
            msgBox.setStandardButtons(QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);
            msgBox.setDefaultButton(QMessageBox::Save);
            if( modifiedFiles.count()>1 ) msgBox.setDetailedText(tr("The modified files are:\n") + modifiedFiles.join("\n"));
            else                          msgBox.setDetailedText(tr("The modified file is:\n") + modifiedFiles.join("\n"));

            int ret = msgBox.exec();
            switch (ret) {
               case QMessageBox::Save:
                   if( !__bSaveAll() )
                       return false;
               case QMessageBox::Discard:
                   break;
               case QMessageBox::Cancel:
                   return false;
             }
        }
		__SaveEnv();
		// close all editor windows
		QList<CodeEditor*> editorList = findWidget<CodeEditor>();
		for( auto code : editorList ) { QTabFramework::removeTab( code );  }
		// close project
        project.reset();
    }
    ProjectTree->setModel(nullptr);
    return true;
}

void MainWindow::__initRecentFileActions()
{
    QSettings settings;
    int maxRecentFiles = settings.value("maxRecentFiles",10).toInt();
    while( recentFiles.count() < maxRecentFiles )
    {
        QAction* action = new QAction(this);
        action->setVisible(false);
		RecentProjectMenu->addAction(action);
        connect(action, &QAction::triggered, this, &MainWindow::__OpenRecentFile);
        recentFiles.push_back(action);
    }
    while( recentFiles.count() > maxRecentFiles )
    {
        QAction* action = recentFiles.takeLast();
		RecentProjectMenu->removeAction(action);
    }
}

void MainWindow::__upadateRecentFileActions(const QString* filename)
{
    int i;
    bool bChanged = false;
    QSettings settings;
    QStringList files = settings.value("recentFileList").toStringList();
    if( filename )
    {
        files.removeOne(*filename);
        files.push_front(*filename);
        bChanged = true;
    }
    if( files.count() > recentFiles.count() )
    {
        while( files.count() > recentFiles.count() ) files.pop_back();
        bChanged = true;
    }
    if( bChanged ) settings.setValue("recentFileList",files);

    for( i=0; i<files.count(); i++ )
    {
        QString text = tr("&%1 %2").arg(i + 1).arg(QFileInfo(files[i]).fileName());
        recentFiles[i]->setText(text);
        recentFiles[i]->setData(files[i]);
        recentFiles[i]->setVisible(true);
    }
    for( ; i<recentFiles.count(); i++ )
    {
        recentFiles[i]->setVisible(false);
    }
	RecentProjectMenu->setEnabled(files.count()>0);
}

void MainWindow::__OpenRecentFile()
{
    QAction *action = qobject_cast<QAction *>(sender());
	if (action)
	{
		if (__bCloseProject())
		{
			LoadProject(action->data().toString());
		}
	}
}

void MainWindow::LoadProject(const QString &fileName)
{
	Q_ASSERT(project == nullptr);

    QString errorString;
	project.reset(Project::pLoadProject(fileName, errorString));
	if ( project )
    {
        ProjectTree->setModel(project.data());
        __upadateRecentFileActions(&fileName);
		__RestoreEnv();
		CloseProjectAction->setEnabled( true );
    }
    else
    {
        QMessageBox msgBox;
        msgBox.setIcon(QMessageBox::Critical);
        msgBox.setText(tr("Loading the project fail."));
        msgBox.setDetailedText(errorString);
        msgBox.setStandardButtons(QMessageBox::Ok);
        msgBox.setDefaultButton(QMessageBox::Save);
        msgBox.exec();
    }
}

void MainWindow::on_NewProject_triggered()
{
    if( __bCloseProject() )
    {
        QString fileName = QFileDialog::getSaveFileName(this, tr("Create Project"), QDir::homePath(), tr("RCad project (*.rcad.prj)"));
        if( !fileName.isEmpty() )
        {
            if( !fileName.endsWith("rcad.prj") ) fileName.append(".rcad.prj");
			
			QString errorString;
			project.reset(Project::pCreateProject(fileName, errorString));
			if (project)
			{
				ProjectTree->setModel(project.data());
				CloseProjectAction->setEnabled(true);
				__upadateRecentFileActions(&fileName);
			}
			else
			{
				QMessageBox msgBox;
				msgBox.setIcon(QMessageBox::Critical);
				msgBox.setText(tr("Project creation fail."));
				msgBox.setDetailedText(errorString);
				msgBox.setStandardButtons(QMessageBox::Ok);
				msgBox.setDefaultButton(QMessageBox::Save);
				msgBox.exec();
			}
        }
    }

}
void MainWindow::on_OpenProject_triggered()
{
    if( __bCloseProject() )
    {
		CloseProjectAction->setEnabled(true);
        QString fileName = QFileDialog::getOpenFileName(this, tr("Open Project"), QDir::homePath(), tr("RCad project (*.rcad.prj)"));
        if( !fileName.isEmpty() ) LoadProject(fileName);
    }
}
void MainWindow::on_SaveAll_triggered()
{
    __bSaveAll();
}

template<class T> 
bool MainWindow::__bSaveElement( T* element )
{
	QString errorString;
	if( element->isModified() && !element->bSave( errorString ) )
	{
		QMessageBox msgBox;
		msgBox.setIcon( QMessageBox::Critical );
		msgBox.setText( tr("Error while saving <%1>.").arg( element->filename() ) );
		msgBox.setDetailedText( errorString );
		msgBox.setStandardButtons( QMessageBox::Abort | QMessageBox::Ignore );
		msgBox.setDefaultButton( QMessageBox::Ignore );
		return msgBox.exec() == QMessageBox::Ignore;
	}
	return true;
}
bool MainWindow::__bSaveAll()
{
    if( project )
    {
		if( !__bSaveElement( project.data() ) ) return false;
		QList<CodeEditor*> editorList = findWidget<CodeEditor>();
		for( auto code : editorList ) { if( !__bSaveElement( code )) return false; }
    }
    return true;
}

void MainWindow::on_CloseProject_triggered()
{
    __bCloseProject();
}
void MainWindow::closeEvent(QCloseEvent *event)
{
    if( __bCloseProject() )
    {
        event->accept();

		QSettings settings;
		settings.setValue("geometry", saveGeometry());
		settings.setValue("windowState", saveState());
    }
    else
    {
        event->ignore();
    }
}

void MainWindow::on_Exit_triggered()
{
    close();
}

void MainWindow::on_ViewProject_triggered()
{
	if( getWidgetByName<QTreeView>( PROJECT_TREEVIEW_NAME ) != nullptr )
	{
		ProjectTree->setFocus();
		return;
	}
	addTab( ProjectTree, QTabFramework::InsertFullLeft, nullptr );
}

void MainWindow::on_ProjectView_customContextMenuRequested(const QPoint& pos)
{
    if( project != nullptr )
    {
        QPoint globalPos = ProjectTree->mapToGlobal(pos);
        QModelIndex index = ProjectTree->indexAt(pos);
        Project::eITEMTYPE type = project->eGetType(index);
        QAction* action;

        QMenu myMenu;
        switch(type)
        {
        case Project::eProject:
        case Project::eNone:
            action = new QAction(QIcon(":/images/folder.png"), tr("Add folder"), this);
            connect(action, &QAction::triggered, this, [this,index]{__AddFolder(index);} );
            myMenu.addAction(action);
            action = new QAction(QIcon(":/images/source.png"), tr("Add new file"), this);
            connect(action, &QAction::triggered, this, [this,index]{__AddNewFile(index);} );
            myMenu.addAction(action);
            action = new QAction(QIcon(":/images/source.png"), tr("Add existing file"), this);
            connect(action, &QAction::triggered, this, [this,index]{__AddExistingFile(index);} );
            myMenu.addAction(action);
            action = new QAction(QIcon(":/images/library.png"), tr("Add library"), this);
            connect(action, &QAction::triggered, this, &MainWindow::__AddLibrary );
            myMenu.addAction(action);
            break;
        case Project::eSource:
            action = new QAction(QIcon(":/images/edit.png"), tr("edit"), this);
            connect(action, &QAction::triggered, this, [this,index]{__Edit(index);} );
            myMenu.addAction(action);
            myMenu.addSeparator();
            action = new QAction(QIcon(":/images/folder.png"), tr("Add folder"), this);
            connect(action, &QAction::triggered, this, [this,index]{__AddFolder(index);} );
            myMenu.addAction(action);
            action = new QAction(QIcon(":/images/source.png"), tr("Add new file"), this);
            connect(action, &QAction::triggered, this, [this,index]{__AddNewFile(index);} );
            myMenu.addAction(action);
            action = new QAction(QIcon(":/images/source.png"), tr("Add existing file"), this);
            connect(action, &QAction::triggered, this, [this,index]{__AddExistingFile(index);} );
            myMenu.addAction(action);
            myMenu.addSeparator();
            action = new QAction(QIcon(":/images/delete.png"), tr("Remove"), this);
            connect(action, &QAction::triggered, this, [this,index]{__Remove(index);} );
            myMenu.addAction(action);
            break;
        case Project::eFolder:
            action = new QAction(QIcon(":/images/folder.png"), tr("Add folder"), this);
            connect(action, &QAction::triggered, this, [this,index]{__AddFolder(index);} );
            myMenu.addAction(action);
            action = new QAction(QIcon(":/images/source.png"), tr("Add new file"), this);
            connect(action, &QAction::triggered, this, [this,index]{__AddNewFile(index);} );
            myMenu.addAction(action);
            action = new QAction(QIcon(":/images/source.png"), tr("Add existing file"), this);
            connect(action, &QAction::triggered, this, [this,index]{__AddExistingFile(index);} );
            myMenu.addAction(action);
            myMenu.addSeparator();
            action = new QAction(QIcon(":/images/rename.png"), tr("Rename"), this);
            connect(action, &QAction::triggered, this, [this,index]{__Rename(index);} );
            myMenu.addAction(action);
            action = new QAction(QIcon(":/images/delete.png"), tr("Remove"), this);
            connect(action, &QAction::triggered, this, [this,index]{__Remove(index);} );
            myMenu.addAction(action);
            break;
        case Project::eLibraries:
            action = new QAction(QIcon(":/images/library.png"), tr("Add library"), this);
            connect(action, &QAction::triggered, this, &MainWindow::__AddLibrary );
            myMenu.addAction(action);break;
        case Project::eLibrary:
            action = new QAction(QIcon(":/images/library.png"), tr("Add library"), this);
            connect(action, &QAction::triggered, this, &MainWindow::__AddLibrary );
            myMenu.addAction(action);
            myMenu.addSeparator();
            action = new QAction(QIcon(":/images/delete.png"), tr("Remove"), this);
            connect(action, &QAction::triggered, this, [this,index]{__Remove(index);} );
            myMenu.addAction(action);
            break;
        }
        myMenu.exec(globalPos);
    }
}

void MainWindow::on_ProjectView_clicked(const QModelIndex& index)
{
	__Edit( index );
}

void MainWindow::__AddFolder(QModelIndex index)
{
    if( project )
    {
        QModelIndex newFolder = project->AddFolder(index);
        if( newFolder.isValid() ) ProjectTree->edit(newFolder);
    }
}
void MainWindow::__AddNewFile(QModelIndex index)
{
    if( project )
    {
		QDir DefaultDir = QFileInfo(project->filename()).dir();
		newFileDialog d(DefaultDir,this);
        if( QDialog::Accepted == d.exec() )
        {
			project->AddSourceFile(index, d.getFullName());
        }
    }
}
void MainWindow::__AddExistingFile(QModelIndex index)
{
    if( project )
    {
        QString fileName = QFileDialog::getOpenFileName(this, tr("Open Project"), QFileInfo(project->filename()).filePath(), tr("RCad source (*.rcad)"));
        if( !fileName.isEmpty() && project ) project->AddSourceFile(index,fileName);
    }
}
void MainWindow::__AddLibrary()
{
    if( project ) project->AddLibrary("aaa");
}
void MainWindow::__Edit( QModelIndex index )
{
	Project::eITEMTYPE eType = project->eGetType( index );
	if( eType != Project::eSource && eType != Project::eLibrary ) return;

	QString	name = project->GetElementIdentifier( index );
	CodeEditor* editor = getWidgetByName<CodeEditor>( name );
	if( editor )
	{
		giveFocus( editor );
		return;
	}
	// create new editor
	editor = new CodeEditor( name, project->GetFileName( index ) );
	if( !editor->bRead( eType == Project::eSource, true ) )
	{
		delete editor;
		return;
	}
	// search best placement
	QWidget* placement = getWidgetByName<CodeEditor>("");
	if( placement ) { addTab( editor, InsertOnTop, placement ); return; }

	placement = getWidgetByName<QTreeView>( "", false );
	if( placement ) 
	{ 
		InsertPolicy policy = bestInsertPolicy( placement, false );
		if( policy != NoInsert )
		{
			addTab( editor, InsertRight, placement );
			return;
		}
	}
	addTab( editor, InsertOnTop, nullptr );
}
void MainWindow::__Rename(QModelIndex index)
{
    ProjectTree->edit(index);
}
void MainWindow::__Remove(QModelIndex index)
{
    if( project ) project->Remove(index);
}

void MainWindow::__SaveEnv()
{
	QJsonDocument env = saveLayout();
	QByteArray json_env = env.toJson( QJsonDocument::Indented );
	
	QSaveFile file( project->filename()+".env" );
	if( !file.open( QIODevice::WriteOnly ) ) return;
	file.write( json_env );
	file.commit();
}

void MainWindow::__RestoreEnv()
{
	QFile file( project->filename() + ".env" );
	if( !file.open( QIODevice::ReadOnly ) ) return;
	QJsonDocument env = QJsonDocument::fromJson(file.readAll());

	restoreLayout( env, [this]( const QString& name ) ->QWidget* {
		if( name.compare( PROJECT_TREEVIEW_NAME, Qt::CaseInsensitive ) == 0 )
		{
			return ProjectTree;
		}
		else
		{
			QString fileName = project->GetFileNameFromIdentifier( name );
			if( !fileName.isEmpty() )
			{
				CodeEditor* editor = new CodeEditor( name, fileName );
				if( editor->bRead(false,false) ) return editor;
				delete editor;
			}
		}
		return nullptr;
	} );
}
