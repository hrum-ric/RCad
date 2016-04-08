
#pragma once
#include "QtWidgets/QMainWindow"
#include <QtCore/QSignalMapper>
#include <QtWidgets/QSplitter>
#include <QtWidgets/QTabBar>
#include <QJsonDocument>
#include <QJsonObject>
#include <functional>
class QTabFramework;
class QTabContainer;
class QArrowCenter;

class QTabWindow : public QMainWindow
{
	Q_OBJECT

public:
	QTabWindow( QTabFramework* tabFramework );

protected:
	virtual void closeEvent( QCloseEvent* event );

private:
	QTabFramework* tabFramework;
	QWidget* overlayWidget;
	QWidget* overlayWidgetTab;
	QWidget* focusTab;

private:
	void setDropOverlayRect( const QRect& globalRect, const QRect& tabRect = QRect() );
	int tabCount();

	QJsonObject writeLayout();
	typedef std::function<QWidget*(const QString& name)> restoreFactory;
	void readLayout( const QJsonObject &, restoreFactory& factory );

	friend class QTabContainer;
	friend class QTabDrawer;
	friend class QTabFramework;
	friend class QTabSplitter;
};

class QTabFramework : public QTabWindow
{
	Q_OBJECT

public:
	enum InsertPolicy
	{
		NoInsert = 0x000,
		InsertFloating = 0x001,
		InsertOnTop = 0x002,
		Insert = 0x004,
		InsertLeft = 0x008,
		InsertRight = 0x010,
		InsertTop = 0x020,
		InsertBottom = 0x040,
		InsertFullLeft = 0x080,
		InsertFullRight = 0x100,
		InsertFullTop = 0x200,
		InsertFullBottom = 0x400
	};
	Q_DECLARE_FLAGS( InsertPolicies, InsertPolicy );

public:
	QTabFramework();
	~QTabFramework();

	void addTab( QWidget* widget, InsertPolicy insertPolicy = InsertFloating, QWidget* position = 0 );
	void moveTab( QWidget* widget, InsertPolicy insertPolicy = InsertFloating, QWidget* position = 0 );
	void removeTab( QWidget* widget );
	void giveFocus( QWidget* widget );

	// choose best place to insert new widget, return policy and position
	InsertPolicy bestInsertPolicy( QWidget* widget, bool bVertical, QWidget*& position);

	template<typename T> 
	T* getWidgetByName( QString name, bool inFloating=true )
	{
		T* widget = findChild<T*>( name );
		for( int i = 0; inFloating && widget == nullptr && i < floatingWindows.size(); ++i ) { widget = floatingWindows.at( i )->findChild<T*>( name ); }
		return widget;
	}

	template<typename T>
	QList<T*> findWidget()
	{
		QList<T*> list = findChildren<T*>();
		for( int i = 0; i < floatingWindows.size(); ++i ) { list.append( floatingWindows.at( i )->findChildren<T*>()); }
		return list;
	}

	// save tab layout
	QJsonDocument saveLayout();
	// restore tab layout	
	void restoreLayout( const QJsonDocument& doc, restoreFactory factory );

signals:
	void focuschanged(QWidget* now);

protected:
	virtual void closeEvent( QCloseEvent* event );
	virtual void showEvent( QShowEvent* event );
	virtual bool eventFilter( QObject* obj, QEvent* event );

private:
	struct TabData
	{
		QWidget* widget;
		QString objectName;
		bool hidden;
		QAction* action;
	};

private:
	QList<QTabWindow*> floatingWindows;
	QList<QTabWindow*> floatingWindowsZOrder;
	QArrowCenter* arrows;
	QTabContainer* lastContainerWithFocus;

private slots:
	void showFloatingWindows();
	void handleFocusChanged( QWidget* old, QWidget* now );

private:
	void addTab( QWidget* widget, QTabContainer* position, InsertPolicy insertPolicy, int tabIndex );
	void moveTab( QWidget* widget, QTabContainer* position, InsertPolicy insertPolicy, int tabIndex );
	void moveTabLater( QWidget* widget, QTabContainer* position, InsertPolicy insertPolicy, int tabIndex );
	bool removeContainerIfEmpty( QTabContainer* tabContainer, bool hide = false );
	QTabWindow* createWindow();
	void removeWindow( QTabWindow* window );

	friend class QTabDrawer;
	friend class QTabContainer;
	friend class QTabWindow;
};

Q_DECLARE_OPERATORS_FOR_FLAGS( QTabFramework::InsertPolicies );
