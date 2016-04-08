#ifndef MAINWINDOW_H
#define MAINWINDOW_H
#include <QMainWindow>
#include <QTreeView>
#include <memory>
#include "QTabFramework.h"
class Project;
class Program;
class ErrorPanel;
class TokenPositionBase;
class CodeEditor;

namespace Ui {
class MainWindow;
}

class MainWindow : public QTabFramework
{
    Q_OBJECT

public:
    explicit MainWindow();
    ~MainWindow();

	void LoadProject( const QString &fileName );

private slots:
    void on_Exit_triggered();
	void on_NewProject_triggered();
	void on_OpenProject_triggered();
	void on_CloseProject_triggered();
	void on_SaveAll_triggered();
	void on_ViewProject_triggered();
	void on_ViewError_triggered();
	void on_Compile_triggered();
	void on_ProjectView_customContextMenuRequested(const QPoint &pos);
	void on_ProjectView_clicked(const QModelIndex& index);
	void on_focuschanged(QWidget* now);

	void __OpenRecentFile();
    void __AddFolder(QModelIndex);
    void __AddNewFile(QModelIndex);
    void __AddExistingFile(QModelIndex);
    void __AddLibrary();
    void __Edit(QModelIndex);
    void __Rename(QModelIndex);
    void __Remove(QModelIndex);
	void __gotocode(QString file, const TokenPositionBase& position);

protected:
	void closeEvent(QCloseEvent *event) override;

private:
	void		__CreateMenu();
	void		__CreateProjectView();
    bool		__bCloseProject();
    void		__initRecentFileActions();
    void		__upadateRecentFileActions(const QString *filename);
	void		__restoreSettings();
	QStringList __getMidifiedFileList();
	bool		__bSaveAll();
	void		__SaveEnv();
	void		__RestoreEnv();
	CodeEditor* __Edit(QString identifier, QString filename, bool askCreate);
	template<class T> bool __bSaveElement( T* element );

private:
	QTreeView*					m_projectTree;
	ErrorPanel*					m_errorPanel;
	QMenu*						m_recentProjectMenu;
	QList<QAction*>				m_recentFiles;
	QAction*					m_closeProjectAction;
	QAction*					m_saveDocument;
	std::unique_ptr<Project>	m_project;
	std::unique_ptr<Program>	m_program;
};

#endif // MAINWINDOW_H
