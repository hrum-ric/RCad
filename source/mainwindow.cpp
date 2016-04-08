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
#include "Compiler.h"
#include "ProgramTree.h"
#include "ErrorPanel.h"

MainWindow::MainWindow() : QTabFramework()
{
	__CreateMenu();
	__CreateProjectView();
	__restoreSettings(); 
	__initRecentFileActions();
	__upadateRecentFileActions(nullptr);
	
	QMetaObject::connectSlotsByName(this);
	connect(this,&QTabFramework::focuschanged, this, &MainWindow::on_focuschanged);
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
	m_recentProjectMenu = new QMenu(tr("Recent project"), FileMenu);
	FileMenu->addAction(m_recentProjectMenu->menuAction());
	FileMenu->addSeparator();
	// File/Save
	m_saveDocument = new QAction(tr("Save"), this);
	m_saveDocument->setEnabled(false);
	FileMenu->addAction(m_saveDocument);
	// File/Save all
	QAction* SaveAllAction = new QAction(tr("Save all"), this);
	SaveAllAction->setObjectName( "SaveAll" );
	FileMenu->addAction(SaveAllAction);
	// File/Close project
	m_closeProjectAction = new QAction(tr("Close project"), this);
	m_closeProjectAction->setEnabled(false);
	m_closeProjectAction->setObjectName( "CloseProject" );
	FileMenu->addAction(m_closeProjectAction);
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
	// View/Error
	QAction* ViewErrorAction = new QAction(tr("Error list"), this);
	ViewErrorAction->setObjectName("ViewError");
	ViewMenu->addAction(ViewErrorAction);

	QAction* CompileAction = new QAction(tr("Compile"), this);
	CompileAction->setObjectName("Compile");
	ViewMenu->addAction(CompileAction);

	// Edit
	QMenu* EditMenu = new QMenu(tr("&Edit"),menuBar);
	menuBar->addAction(EditMenu->menuAction());
}

void MainWindow::__CreateProjectView()
{
	m_projectTree = new QTreeView();
	m_projectTree->setObjectName( PROJECT_TREEVIEW_NAME );
	m_projectTree->setWindowTitle(tr("Project"));
	m_projectTree->setRootIsDecorated(true);
	m_projectTree->setHeaderHidden(true);
	m_projectTree->setContextMenuPolicy(Qt::CustomContextMenu);
	m_projectTree->setEditTriggers(QAbstractItemView::NoEditTriggers);
	m_projectTree->setDragDropMode( QAbstractItemView::InternalMove );
	m_projectTree->header()->setDefaultSectionSize(0);
	addTab(m_projectTree,QTabFramework::InsertLeft);

	m_errorPanel = new ErrorPanel();
	m_errorPanel->setObjectName(ERROR_PANEL_NAME);
	connect(m_errorPanel,&ErrorPanel::gotoerror,this, &MainWindow::__gotocode);
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
	if( m_project->isModified() ) modifiedFiles.append( m_project->filename() );
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
    if( m_project )
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
        m_project.reset();
    }
    m_projectTree->setModel(nullptr);
    return true;
}

void MainWindow::__initRecentFileActions()
{
    QSettings settings;
    int maxRecentFiles = settings.value("maxRecentFiles",10).toInt();
    while( m_recentFiles.count() < maxRecentFiles )
    {
        QAction* action = new QAction(this);
        action->setVisible(false);
		m_recentProjectMenu->addAction(action);
        connect(action, &QAction::triggered, this, &MainWindow::__OpenRecentFile);
        m_recentFiles.push_back(action);
    }
    while( m_recentFiles.count() > maxRecentFiles )
    {
        QAction* action = m_recentFiles.takeLast();
		m_recentProjectMenu->removeAction(action);
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
    if( files.count() > m_recentFiles.count() )
    {
        while( files.count() > m_recentFiles.count() ) files.pop_back();
        bChanged = true;
    }
    if( bChanged ) settings.setValue("recentFileList",files);

    for( i=0; i<files.count(); i++ )
    {
        QString text = tr("&%1 %2").arg(i + 1).arg(QFileInfo(files[i]).fileName());
        m_recentFiles[i]->setText(text);
        m_recentFiles[i]->setData(files[i]);
        m_recentFiles[i]->setVisible(true);
    }
    for( ; i<m_recentFiles.count(); i++ )
    {
        m_recentFiles[i]->setVisible(false);
    }
	m_recentProjectMenu->setEnabled(files.count()>0);
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
	Q_ASSERT(m_project == nullptr);

    QString errorString;
	m_project.reset(Project::pLoadProject(fileName, errorString));
	if ( m_project )
    {
        m_projectTree->setModel(m_project.get());
        __upadateRecentFileActions(&fileName);
		__RestoreEnv();
		m_closeProjectAction->setEnabled( true );
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
			m_project.reset(Project::pCreateProject(fileName, errorString));
			if (m_project)
			{
				m_projectTree->setModel(m_project.get());
				m_closeProjectAction->setEnabled(true);
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
		m_closeProjectAction->setEnabled(true);
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
    if( m_project )
    {
		if( !__bSaveElement( m_project.get() ) ) return false;
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
		m_projectTree->setFocus();
		return;
	}
	addTab( m_projectTree, QTabFramework::InsertFullLeft, nullptr );
}
void MainWindow::on_ViewError_triggered()
{
	if (getWidgetByName<ErrorPanel>(ERROR_PANEL_NAME ) != nullptr)
	{
		m_errorPanel->setFocus();
		return;
	}
	addTab(m_errorPanel, QTabFramework::InsertFullBottom, nullptr);
}

void MainWindow::on_Compile_triggered()
{
	Compiler compiler;
	QStringList fileList = m_project->listAllFile();

	QList<Compiler::SourceInfo> list;
	for(QString file : fileList)
	{
		list.append( Compiler::SourceInfo(file) );
	}

	// 
	Program* program = compiler.compile(list);
	// update ui
	m_errorPanel->setErrorList(program->getErrorList());
	m_program.reset(program);
}

void MainWindow::on_ProjectView_customContextMenuRequested(const QPoint& pos)
{
    if( m_project != nullptr )
    {
        QPoint globalPos = m_projectTree->mapToGlobal(pos);
        QModelIndex index = m_projectTree->indexAt(pos);
        Project::eITEMTYPE type = m_project->eGetType(index);
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

void MainWindow::on_focuschanged(QWidget* now)
{
	if( now && !now->windowTitle().isEmpty() && now->metaObject()->indexOfSlot("save()")>-1 )
	{ 
		m_saveDocument->setText(tr("Save (%1)").arg(now->windowTitle()));
		m_saveDocument->setEnabled(true);
		connect(m_saveDocument,SIGNAL(triggered()),now,SLOT(save()));
	}
	else
	{
		m_saveDocument->disconnect();
		m_saveDocument->setText(tr("Save"));
		m_saveDocument->setEnabled(false);
	}
}

void MainWindow::__AddFolder(QModelIndex index)
{
    if( m_project )
    {
        QModelIndex newFolder = m_project->AddFolder(index);
        if( newFolder.isValid() ) m_projectTree->edit(newFolder);
    }
}
void MainWindow::__AddNewFile(QModelIndex index)
{
    if( m_project )
    {
		QDir DefaultDir = QFileInfo(m_project->filename()).dir();
		newFileDialog d(DefaultDir,this);
        if( QDialog::Accepted == d.exec() )
        {
			m_project->AddSourceFile(index, d.getFullName());
        }
    }
}
void MainWindow::__AddExistingFile(QModelIndex index)
{
    if( m_project )
    {
        QString fileName = QFileDialog::getOpenFileName(this, tr("Open Project"), QFileInfo(m_project->filename()).filePath(), tr("RCad source (*.rcad)"));
        if( !fileName.isEmpty() && m_project ) m_project->AddSourceFile(index,fileName);
    }
}
void MainWindow::__AddLibrary()
{
    if( m_project ) m_project->AddLibrary("aaa");
}
void MainWindow::__Edit( QModelIndex index )
{
	Project::eITEMTYPE eType = m_project->eGetType( index );
	if( eType != Project::eSource && eType != Project::eLibrary ) return;

	QString	identifier = m_project->GetElementIdentifier( index );
	QString	filename = m_project->GetFileName(index);

	__Edit(identifier, filename, eType == Project::eSource);
}

CodeEditor* MainWindow::__Edit(QString identifier, QString filename, bool askCreate)
{
	CodeEditor* editor = getWidgetByName<CodeEditor>(identifier);
	if( editor )
	{
		giveFocus( editor );
		return editor;
	}
	// create new editor
	editor = new CodeEditor(identifier, filename);
	if( !editor->bRead(askCreate, true ) )
	{
		delete editor;
		return nullptr;
	}
	// search best placement
	auto otherEditor = findWidget<CodeEditor>();
	if( !otherEditor.isEmpty() ) { addTab( editor, InsertOnTop, otherEditor.first() ); return editor; }

	auto project = getWidgetByName<QTreeView>(PROJECT_TREEVIEW_NAME, false );
	if( project )
	{ 
		QWidget* position=nullptr;
		InsertPolicy policy = bestInsertPolicy(project, false, position );
		if( policy != NoInsert )
		{
			addTab( editor, policy, position);
			return editor;
		}
	}
	addTab( editor, InsertOnTop, nullptr );
	return editor;
}
void MainWindow::__Rename(QModelIndex index)
{
    m_projectTree->edit(index);
}
void MainWindow::__Remove(QModelIndex index)
{
    if( m_project ) m_project->Remove(index);
}

void MainWindow::__SaveEnv()
{
	QJsonDocument env = saveLayout();
	QByteArray json_env = env.toJson( QJsonDocument::Indented );
	
	QSaveFile file( m_project->filename()+".env" );
	if( !file.open( QIODevice::WriteOnly ) ) return;
	file.write( json_env );
	file.commit();
}

void MainWindow::__RestoreEnv()
{
	QFile file( m_project->filename() + ".env" );
	if( !file.open( QIODevice::ReadOnly ) ) return;
	QJsonDocument env = QJsonDocument::fromJson(file.readAll());

	restoreLayout( env, [this]( const QString& name ) ->QWidget* {
		if( name.compare( PROJECT_TREEVIEW_NAME, Qt::CaseInsensitive ) == 0 )
		{
			return m_projectTree;
		}
		else if (name.compare(ERROR_PANEL_NAME, Qt::CaseInsensitive) == 0)
		{
			return m_errorPanel;
		}
		else
		{
			QString fileName = m_project->GetFileNameFromIdentifier( name );
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

void MainWindow::__gotocode(QString file, const TokenPositionBase& position)
{
	CodeEditor* editor = __Edit(SOURCE_IDENTIFIR_PREFIX + QFileInfo(file).baseName(), file, false);
	editor->setCursorPosition(position.m_startLine-1,position.m_startColumn);
}
