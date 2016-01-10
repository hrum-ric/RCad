
#pragma once
#include "QTabFramework.h"
#include "QtWidgets/QMainWindow"
#include <QtCore/QSignalMapper>
#include <QtWidgets/QSplitter>
#include <QtWidgets/QTabBar>
class QTabFramework;
class QTabContainer;
class QArrowCenter;

class QTabSplitter : public QSplitter
{
public:
	QTabSplitter( Qt::Orientation orientation, QWidget* parent, QTabWindow* tabWindow );

private:
	QJsonObject writeLayout();
	void readLayout( const QJsonObject &, QTabFramework::restoreFactory& );
private:
	QTabWindow* tabWindow;
	friend class QTabWindow;
};

class QTabContainer : public QTabWidget
{
	Q_OBJECT

public:
	QTabContainer( QWidget* parent, QTabWindow* tabWindow, QTabFramework::InsertPolicy place );
	~QTabContainer();

	int addTab( QWidget* widget, const QString& label );
	int insertTab( int index, QWidget* widget, const QString& label );
	void removeTab( int index );

protected:
	QTabFramework::InsertPolicies GetInsertPolicies();
	virtual void dragEnterEvent( QDragEnterEvent* event );
	virtual void dragLeaveEvent( QDragLeaveEvent* event );
	virtual void dragMoveEvent( QDragMoveEvent* event );
	virtual void dropEvent( QDropEvent* event );

private:
	QTabWindow* tabWindow;
	QTabFramework::InsertPolicy place;

private slots:
	void handleCurrentChanged( int index );

private:
	QJsonObject writeLayout();
	void readLayout( const QJsonObject &, QTabFramework::restoreFactory& );
	QTabFramework::InsertPolicy findDropRect( const QPoint& globalPos, int tabWidth, QRect( &overlay )[2], int& tabIndex );

	friend class QTabDrawer;
	friend class QTabFramework;
	friend class QTabWindow;
	friend class QTabSplitter;
};

class QTabDrawer : public QTabBar
{
	Q_OBJECT

public:
	QTabDrawer( QTabContainer* tabContainer );

protected:
	virtual void mousePressEvent( QMouseEvent* event );
	virtual void mouseReleaseEvent( QMouseEvent* event );
	virtual void mouseMoveEvent( QMouseEvent* event );

private:
	QTabContainer* tabContainer;
	int pressedIndex;
	QPoint dragStartPosition;

	private slots:
	void closeTab( int index );
};

class QArrowCenter : public QWidget
{
public:
	QArrowCenter( QWidget* parent );
	void showOver( QWidget* over, QTabFramework::InsertPolicies );
	QTabFramework::InsertPolicy findInsertPolicy( const QPoint& globalPos );

protected:
	void paintEvent( QPaintEvent * );

private:
	bool __isInImage( const QPoint& localPos, int x, int y );

private:
	const int imgSpace = 3;
	const int bevelSize = 11;
	int imgSize;
	int blockSize;

private:
	QTabFramework::InsertPolicies	policies;
	QPolygonF						background;
	QPixmap							fullLeft;
	QPixmap							fullRight;
	QPixmap							fullTop;
	QPixmap							fullBottom;
	QPixmap							left;
	QPixmap							right;
	QPixmap							top;
	QPixmap							bottom;
	QPixmap							center;
};