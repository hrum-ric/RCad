#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTreeView>
#include "QTabFramework.h"
class Project;

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
	void on_ProjectView_customContextMenuRequested(const QPoint &pos);
	void on_ProjectView_clicked(const QModelIndex& index);

	void __OpenRecentFile();
    void __AddFolder(QModelIndex);
    void __AddNewFile(QModelIndex);
    void __AddExistingFile(QModelIndex);
    void __AddLibrary();
    void __Edit(QModelIndex);
    void __Rename(QModelIndex);
    void __Remove(QModelIndex);

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
	template<class T> bool __bSaveElement( T* element );

private:
	QTreeView*				ProjectTree;
	QMenu*					RecentProjectMenu;
	QList<QAction*>			recentFiles;
	QAction*				CloseProjectAction;
	QScopedPointer<Project> project;
};

#endif // MAINWINDOW_H
