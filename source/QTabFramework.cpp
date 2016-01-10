#include "QTabFramework.h"
#include "QTabFrameworkInternal.h"
#include <QtWidgets/QApplication>
#include <QtWidgets/QWidget>
#include <QtWidgets/QAction>
#include <QtWidgets/QMessageBox>
#include <QtWidgets/QShortcut>
#include <QtGui/QPainter>
#include <QtGui/QMouseEvent>
#include <QtGui/QDrag>
#include <QtCore/QMimeData>
#include <QtCore/QLinkedList>
#include <QtCore/QTimer>
#include <QJsonArray>
#include <QDebug>

// base on https://github.com/craflin/QTabFramework

class QTabDropOverlay : public QWidget
{
public:
	QTabDropOverlay( QWidget* parent ) : QWidget( parent )
	{
		setPalette( Qt::transparent );
		setAttribute( Qt::WA_TransparentForMouseEvents );
	}

private:
	void paintEvent( QPaintEvent* )
	{
		QPainter painter( this );
		painter.setPen( Qt::NoPen );
		painter.setBrush( QBrush( QColor( 88, 88, 88, 88 ) ) );
		painter.drawRect( rect() );
	}
};

QTabDrawer::QTabDrawer( QTabContainer* tabContainer ) : QTabBar( tabContainer ), tabContainer( tabContainer ), pressedIndex( -1 )
{
	setTabsClosable( true );
	setUsesScrollButtons( true );
	setElideMode( Qt::ElideRight );
	connect( this, SIGNAL( tabCloseRequested( int ) ), this, SLOT( closeTab( int ) ) );
}

void QTabDrawer::mousePressEvent( QMouseEvent* event )
{
	if( event->button() != Qt::LeftButton )
	{
		event->ignore();
		return;
	}

	pressedIndex = tabAt( event->pos() );
	if( pressedIndex >= 0 )
	{
		dragStartPosition = event->pos();
	}

	QTabBar::mousePressEvent( event );
}

void QTabDrawer::mouseReleaseEvent( QMouseEvent* event )
{
	if( event->button() != Qt::LeftButton )
	{
		event->ignore();
		return;
	}

	QTabBar::mouseReleaseEvent( event );
}

void QTabDrawer::mouseMoveEvent( QMouseEvent* event )
{
	// Start drag
	if( pressedIndex >= 0 )
	{

		if( (event->pos() - dragStartPosition).manhattanLength() > QApplication::startDragDistance() )
		{
			if( rect().contains( event->pos() ) )
			{
				for( int i = 0, count = this->count(); i < count; ++i )
				{
					QRect tabRect = this->tabRect( i );
					if( i != pressedIndex && tabRect.contains( event->pos() ) )
					{
						moveTab( pressedIndex, i );
						pressedIndex = i;
						dragStartPosition = event->pos();
						break;
					}
				}
			}
			else
			{
				QTabWindow* window = tabContainer->tabWindow;
				QTabFramework* framework = window->tabFramework;
				QWidget* dragWidget = tabContainer->widget( pressedIndex );


				// create drag data
				QMimeData* mimeData = new QMimeData;
				mimeData->setData( "application/x-tabwidget", QByteArray() );

				// create image of window
				QRect tabRect = this->tabRect( pressedIndex );
				tabRect.adjust( -2, 0, 0, 0 );
				QPixmap TabPixmap = this->grab( tabRect );
				QPixmap ContentPixmap = dragWidget->grab();
				QPixmap pixmap( qMax( TabPixmap.width(), ContentPixmap.width() ), TabPixmap.height() + ContentPixmap.height() );
				pixmap.fill( Qt::transparent );
				QPainter painter( &pixmap );
				painter.setOpacity( 0.75 );
				painter.drawPixmap( 0, 0, TabPixmap );
				painter.drawPixmap( 0, TabPixmap.height(), ContentPixmap );


				QDrag* drag = new QDrag( dragWidget );
				drag->setMimeData( mimeData );
				drag->setPixmap( pixmap );
				drag->setHotSpot( QPoint( event->pos().x() - tabRect.x(), event->pos().y() - tabRect.y() ) );

				// remove dragged widget
				tabContainer->removeTab( pressedIndex );
				dragWidget->setParent( NULL );
				pressedIndex = -1;

				// remove intermediate container and hide window if it's not main windows
				bool bDeleteWindow = framework->removeContainerIfEmpty( tabContainer, true );
				// !! this may be destroy !!

				Qt::DropAction ret = drag->exec( Qt::MoveAction );
				if( ret != Qt::MoveAction ) framework->moveTabLater( dragWidget, 0, QTabFramework::InsertFloating, -1 );

				if( bDeleteWindow ) framework->removeWindow( window );
			}
			return;
		}
	}
	else
		QTabBar::mouseMoveEvent( event );
}

void QTabDrawer::closeTab( int index )
{
	QWidget* widget = tabContainer->widget( index );
	tabContainer->tabWindow->tabFramework->removeTab( widget );
	widget->close();
}

QTabSplitter::QTabSplitter( Qt::Orientation orientation, QWidget* parent, QTabWindow* tabWindow ) : QSplitter( orientation, parent ), tabWindow( tabWindow )
{
	setChildrenCollapsible( false );
}

QTabContainer::QTabContainer( QWidget* parent, QTabWindow* tabWindow, QTabFramework::InsertPolicy place ) : QTabWidget( parent ), tabWindow( tabWindow ), place( place )
{
	setTabBar( new QTabDrawer( this ) );
	setAcceptDrops( true );
	connect( this, SIGNAL( currentChanged( int ) ), this, SLOT( handleCurrentChanged( int ) ) );
}
QTabContainer::~QTabContainer()
{
	if( tabWindow->tabFramework->lastContainerWithFocus == this )
		tabWindow->tabFramework->lastContainerWithFocus = nullptr;
}

int QTabContainer::addTab( QWidget* widget, const QString& label )
{
	int index = QTabWidget::addTab( widget, label );
	setTabToolTip( index, label );
	return index;
}

int QTabContainer::insertTab( int index, QWidget* widget, const QString& label )
{
	index = QTabWidget::insertTab( index, widget, label );
	setTabToolTip( index, label );
	return index;
}

void QTabContainer::removeTab( int index )
{
	QWidget* widget = this->widget( index );
	QTabWidget::removeTab( index );
	if( widget == tabWindow->focusTab )
		tabWindow->tabFramework->handleFocusChanged( widget, this->widget( this->currentIndex() ) );
	// update style
	setStyleSheet( styleSheet() );
}

void QTabContainer::handleCurrentChanged( int index )
{
	QWidget* widget = this->widget( index );
	if( widget )
	{
		if( tabWindow->isActiveWindow() )
		{
			QWidget* focusWidget = widget->focusWidget();
			if( focusWidget )
				focusWidget->setFocus();
			else
				widget->setFocus();
		}
		else if( tabWindow != tabWindow->tabFramework )
		{
			tabWindow->focusTab = widget;
			tabWindow->setWindowTitle( widget->windowTitle() );
		}
	}
}

QTabFramework::InsertPolicy QTabContainer::findDropRect( const QPoint& globalPos, int tabWidth, QRect( &overlay )[2], int& tabIndex )
{
	QPoint pos = mapFromGlobal( globalPos );

	QRect containerRect = rect();
	QRect result;

	tabIndex = -1;
	QTabFramework::InsertPolicy insertPolicy = QTabFramework::InsertFloating;

	if( tabBar()->geometry().contains( pos ) )
	{
		insertPolicy = QTabFramework::Insert;
		QTabBar* tabBar = this->tabBar();
		for( int i = 0, count = tabBar->count(); i < count; ++i )
		{
			QRect tabRect = tabBar->tabRect( i );
			if( tabRect.contains( pos ) )
			{
				tabIndex = i;
				break;
			}
		}
	}
	else if( pos.y() < containerRect.y() + tabBar()->geometry().height() )
	{
		insertPolicy = QTabFramework::Insert;
		tabIndex = this->tabBar()->count();
	}
	else
	{
		insertPolicy = tabWindow->tabFramework->arrows->findInsertPolicy( globalPos );
	}


	if( insertPolicy == QTabFramework::InsertOnTop )
	{
		insertPolicy = QTabFramework::Insert;
		tabIndex = this->tabBar()->count();
	}

	QWidget* pclMap = this;

	switch( insertPolicy )
	{
	case QTabFramework::InsertFullTop:
	case QTabFramework::InsertFullLeft:
	case QTabFramework::InsertFullRight:
	case QTabFramework::InsertFullBottom:
		pclMap = pclMap->parentWidget();
		if( dynamic_cast<QTabSplitter*>(pclMap) )
		{
			QTabSplitter* splitter = dynamic_cast<QTabSplitter*>(pclMap);
			if( (splitter->orientation() == Qt::Horizontal && (insertPolicy == QTabFramework::InsertFullLeft || insertPolicy == QTabFramework::InsertFullRight))
				|| (splitter->orientation() == Qt::Vertical && (insertPolicy == QTabFramework::InsertFullTop || insertPolicy == QTabFramework::InsertFullBottom)) )
			{
				pclMap = pclMap->parentWidget();
			}
		}
		containerRect = pclMap->rect();
		break;
	}

	switch( insertPolicy )
	{
	case QTabFramework::InsertFullTop:
	case QTabFramework::InsertTop:
		overlay[0] = QRect( containerRect.topLeft(), QPoint( containerRect.right(), containerRect.y() + containerRect.height() / 3 ) );
		break;

	case QTabFramework::InsertFullLeft:
	case QTabFramework::InsertLeft:
		overlay[0] = QRect( containerRect.topLeft(), QPoint( containerRect.x() + containerRect.width() / 3, containerRect.bottom() ) );
		break;

	case QTabFramework::InsertFullRight:
	case QTabFramework::InsertRight:
		overlay[0] = QRect( QPoint( containerRect.x() + containerRect.width() * 2 / 3, containerRect.y() ), containerRect.bottomRight() );
		break;

	case QTabFramework::InsertFullBottom:
	case QTabFramework::InsertBottom:
		overlay[0] = QRect( QPoint( containerRect.x(), containerRect.y() + containerRect.height() * 2 / 3 ), containerRect.bottomRight() );
		break;

	case QTabFramework::Insert:
		overlay[0] = containerRect;
		if( 0 <= tabIndex && tabIndex < tabBar()->count() )
		{
			overlay[1] = tabBar()->tabRect( tabIndex );
		}
		else
		{
			overlay[1] = tabBar()->tabRect( tabBar()->count() - 1 );
			overlay[1].setLeft( overlay[1].right() );
		}
		overlay[1].setWidth( tabWidth );
		break;
	}

	overlay[0].translate( pclMap->mapToGlobal( QPoint( 0, 0 ) ) );
	overlay[1].translate( pclMap->mapToGlobal( QPoint( 0, 0 ) ) );

	return insertPolicy;
}

QTabFramework::InsertPolicies QTabContainer::GetInsertPolicies()
{
	QTabFramework::InsertPolicies policies = QTabFramework::InsertTop | QTabFramework::InsertLeft | QTabFramework::InsertRight | QTabFramework::InsertBottom | QTabFramework::InsertOnTop;
	QTabSplitter* splitter = dynamic_cast<QTabSplitter*>(parent());
	if( splitter && splitter->count() >= 2 )
	{
		if( splitter->orientation() == Qt::Vertical )	policies |= QTabFramework::InsertFullLeft | QTabFramework::InsertFullRight;
		else											policies |= QTabFramework::InsertFullTop | QTabFramework::InsertFullBottom;

		QTabSplitter* childSplitter = splitter;
		splitter = dynamic_cast<QTabSplitter*>(splitter->parent());
		if( splitter && splitter->count() >= 2 )
		{
			int index = childSplitter->indexOf( this );
			if( splitter->orientation() == Qt::Vertical )
			{
				if( index == 0 )					    policies |= QTabFramework::InsertFullRight;
				if( index + 1 == childSplitter->count() ) policies |= QTabFramework::InsertFullLeft;
			}
			else
			{
				if( index == 0 )					    policies |= QTabFramework::InsertFullTop;
				if( index + 1 == childSplitter->count() ) policies |= QTabFramework::InsertFullBottom;
			}
		}
	}

	return policies;
}

void QTabContainer::dragEnterEvent( QDragEnterEvent* event )
{
	dragMoveEvent( event );
	if( event->isAccepted() )
	{
		QTabFramework::InsertPolicies policies = GetInsertPolicies();
		tabWindow->tabFramework->arrows->showOver( this, policies );		
	}
}

void QTabContainer::dragMoveEvent( QDragMoveEvent* event )
{
	const QMimeData* mineData = event->mimeData();
	if( mineData->hasFormat( "application/x-tabwidget" ) )
	{
		QByteArray tabMimeData = mineData->data( "application/x-tabwidget" );
		int tabWidth = tabMimeData.size() >= sizeof( int ) ? *(const int*)tabMimeData.data() : 100;

		int tabIndex;
		QRect overlay[2];
		findDropRect( mapToGlobal( event->pos() ), tabWidth, overlay, tabIndex );
		tabWindow->setDropOverlayRect( overlay[0], overlay[1] );
		event->acceptProposedAction();
		return;
	}
	event->ignore();
}

void QTabContainer::dragLeaveEvent( QDragLeaveEvent* )
{
	tabWindow->setDropOverlayRect( QRect() );
	tabWindow->tabFramework->arrows->hide();
}

void QTabContainer::dropEvent( QDropEvent* event )
{
	const QMimeData* mineData = event->mimeData();
	if( mineData->hasFormat( "application/x-tabwidget" ) )
	{
		QByteArray tabMimeData = mineData->data( "application/x-tabwidget" );
		int tabWidth = tabMimeData.size() >= sizeof( int ) ? *(const int*)tabMimeData.data() : 100;

		int tabIndex;
		QRect overlay[2];
		QTabFramework::InsertPolicy insertPolicy = findDropRect( mapToGlobal( event->pos() ), tabWidth, overlay, tabIndex );

		tabWindow->tabFramework->moveTabLater( (QWidget*)event->source(), this, insertPolicy, tabIndex );

		tabWindow->setDropOverlayRect( QRect() );
		tabWindow->tabFramework->arrows->hide();

		event->acceptProposedAction();

		return;
	}
	event->ignore();
}

QTabWindow::QTabWindow( QTabFramework* tabFramework ) : tabFramework( tabFramework ), overlayWidget( 0 ), overlayWidgetTab( 0 ), focusTab( 0 )
{
	setStyleSheet( R"--(QTabDrawer::tab {
							background: none;
							border: none;
							min-width: 24ex;
							min-height: 5ex;
							padding: 2px;
							padding-left: 5px;
							padding-right: 5px;
						}
						QTabDrawer::tab:hover {
							background: #70A7DC;
							color: black;
						}
						QTabDrawer[child_has_focus="false"]::tab:selected{
							background: #C4C4C3;
							color: black;
						}
						QTabDrawer[child_has_focus="true"]::tab:selected{
							background: #007ACC;
							color: white;
						}
						QTabDrawer::tab:!selected {
							margin-top: 0px; /* make non-selected tabs look smaller */
						}

						QTabContainer[count="0"], QTabContainer[count="0"]::pane, QTabContainer[count="0"]::tab-bar 
						{ 
							background: transparent; 
							border: 0px; 
						}
					 )--" );
}

void QTabWindow::setDropOverlayRect( const QRect& globalRect, const QRect& globalTabRect )
{
	if( globalRect.isValid() )
	{
		if( !overlayWidget )
			overlayWidget = new QTabDropOverlay( this );
		overlayWidget->raise();
		overlayWidget->show();
		if( globalTabRect.isValid() )
		{
			QRect wrect = globalRect;
			wrect.setTop( globalTabRect.bottom() + 1 );
			QRect rect = wrect.translated( mapFromGlobal( QPoint( 0, 0 ) ) );
			overlayWidget->setGeometry( rect );
			if( !overlayWidgetTab )
				overlayWidgetTab = new QTabDropOverlay( this );
			overlayWidgetTab->raise();
			overlayWidgetTab->show();
			QRect tabRect = globalTabRect.translated( mapFromGlobal( QPoint( 0, 0 ) ) );
			overlayWidgetTab->setGeometry( tabRect );
		}
		else
		{
			QRect rect = globalRect.translated( mapFromGlobal( QPoint( 0, 0 ) ) );
			overlayWidget->setGeometry( rect );
			if( overlayWidgetTab )
				overlayWidgetTab->hide();
		}
	}
	else if( overlayWidget )
	{
		if( overlayWidgetTab )
			overlayWidgetTab->hide();
		overlayWidget->hide();
	}
}

int QTabWindow::tabCount()
{
	QWidget* centralWidget = this->centralWidget();
	if( !centralWidget )
		return 0;

	int count = 0;
	QLinkedList<QWidget*> widgets;
	widgets.append( centralWidget );
	for( QLinkedList<QWidget*>::Iterator i = widgets.begin(); i != widgets.end(); ++i )
	{
		QWidget* widget = *i;
		QTabSplitter* tabSplitter = dynamic_cast<QTabSplitter*>(widget);
		if( tabSplitter )
		{
			for( int i = 0, count = tabSplitter->count(); i < count; ++i )
			{
				QWidget* widget = tabSplitter->widget( i );
				widgets.append( widget );
			}
		}
		else
		{
			QTabContainer* tabContainer = dynamic_cast<QTabContainer*>(widget);
			if( tabContainer )
				count += tabContainer->count();
		}
	}
	return count;
}

void QTabWindow::closeEvent( QCloseEvent* event )
{
	QTabContainer* centralTabContainer = dynamic_cast<QTabContainer*>(centralWidget());
	if( !centralTabContainer || centralTabContainer->count() > 1 )
	{
		int tabCount = this->tabCount();
		if( QMessageBox::question( this, tr( "Close Window" ), tabCount == 2 ? tr( "Do you really want to close both tabs?" ) : tr( "Do you really want to close all %1 tabs?" ).arg( tabCount ), QMessageBox::Yes | QMessageBox::No ) != QMessageBox::Yes )
		{
			event->ignore();
			return;
		}
	}

	QMainWindow::closeEvent( event );

	tabFramework->removeWindow( this );
}

QTabFramework::QTabFramework() : QTabWindow(this), lastContainerWithFocus(nullptr)
{
	connect( qApp, SIGNAL( focusChanged( QWidget*, QWidget* ) ), this, SLOT( handleFocusChanged( QWidget*, QWidget* ) ) );

	setCentralWidget( new QTabContainer( this, this, NoInsert ) );
	arrows = new QArrowCenter( this );
}

QTabFramework::~QTabFramework()
{
	qDeleteAll( floatingWindows );
}

void QTabFramework::addTab( QWidget* widget, InsertPolicy insertPolicy, QWidget* position )
{
	QTabContainer* container = position ? dynamic_cast<QTabContainer*>(position->parent()->parent()) : 0;

	widget->installEventFilter( this );
	addTab( widget, container, insertPolicy, -1 );
}

void QTabFramework::moveTab( QWidget* widget, InsertPolicy insertPolicy, QWidget* position )
{
	moveTab( widget, position ? dynamic_cast<QTabContainer*>(position->parent()->parent()) : 0, insertPolicy, -1 );
}

void QTabFramework::removeTab( QWidget* widget )
{
	QTabContainer* tabContainer = dynamic_cast<QTabContainer*>(widget->parent()->parent());
	int movedIndex = tabContainer->indexOf( widget );
	tabContainer->removeTab( movedIndex );
	widget->setParent( NULL );
	removeContainerIfEmpty( tabContainer );

	widget->removeEventFilter( this );
}

void QTabFramework::giveFocus( QWidget* widget )
{
	QTabContainer* tabContainer = dynamic_cast<QTabContainer*>(widget->parent()->parent());
	tabContainer->setCurrentWidget( widget );
	widget->setFocus();
}

void traceWidget( QObject* pclObject, int nIdent = 0 )
{
	QString className = pclObject->metaObject()->className();

	if( className == "QWidgetLineControl" ||
		className == "QTabDrawer" ||
		className == "QStackedLayout" ||
		className == "QRubberBand" ||
		className == "QSplitterHandle" ||
		className == "QShortcut" ||
		className == "QMenuBar" ||
		className == "QAction" ||
		className == "QPropertyAnimation" )
		return;

	QWidget* pclWidget = dynamic_cast<QWidget*>(pclObject);
	QSplitter* pclSplitter = dynamic_cast<QSplitter*>(pclObject);

	if( pclSplitter )	qDebug().noquote() << QString( nIdent, '\t' ) << className << " : " << ((pclSplitter->orientation() == Qt::Vertical) ? "Vertical" : "Horizontal");
	else if( pclWidget )	qDebug().noquote() << QString( nIdent, '\t' ) << className << " : " << pclWidget->windowTitle();
	else				qDebug().noquote() << QString( nIdent, '\t' ) << className;

	auto children = pclObject->children();
	foreach( QObject* child, children )
	{
		Q_ASSERT( child->parent() == pclObject );
		traceWidget( child, nIdent + 1 );
	}
}

void QTabFramework::addTab( QWidget* widget, QTabContainer* container, InsertPolicy insertPolicy, int tabIndex )
{
	if( container == nullptr && insertPolicy != InsertFloating )
	{
		container = findChild<QTabContainer*>();
	}

	QWidget* centralWidget = this->centralWidget();
	bool createWindow = !container || insertPolicy == InsertFloating || !centralWidget;
	if( createWindow )
	{
		if( centralWidget || insertPolicy == InsertFloating )
		{
			QTabWindow* tabWindow = this->createWindow();
			tabWindow->move( QCursor::pos() );
			tabWindow->resize( widget->size() );

			QTabContainer* container = new QTabContainer( tabWindow, tabWindow, insertPolicy );
			container->addTab( widget, widget->windowTitle() );
			tabWindow->setCentralWidget( container );
			if( QMainWindow::isVisible() )
				tabWindow->show();
		}
		else
		{
			QTabContainer* container = new QTabContainer( this, this, insertPolicy );
			container->addTab( widget, widget->windowTitle() );
			setCentralWidget( container );
		}
	}
	else
	{
		if( insertPolicy == InsertPolicy::InsertOnTop || (insertPolicy == InsertPolicy::Insert && tabIndex < 0) )
		{
			insertPolicy = InsertPolicy::Insert;
			tabIndex = container->count();
		}
		if( insertPolicy == InsertPolicy::Insert )
		{
			container->insertTab( tabIndex, widget, widget->windowTitle() );
			container->setCurrentIndex( tabIndex );
		}
		else if( insertPolicy == InsertPolicy::InsertFullLeft || insertPolicy == InsertPolicy::InsertFullRight ||
				 insertPolicy == InsertPolicy::InsertLeft || insertPolicy == InsertPolicy::InsertRight ||
				 insertPolicy == InsertPolicy::InsertFullTop || insertPolicy == InsertPolicy::InsertFullBottom ||
				 insertPolicy == InsertPolicy::InsertTop || insertPolicy == InsertPolicy::InsertBottom )
		{
			Qt::Orientation orientation = Qt::Vertical;
			if( insertPolicy == InsertPolicy::InsertFullLeft || insertPolicy == InsertPolicy::InsertFullRight ||
				insertPolicy == InsertPolicy::InsertLeft || insertPolicy == InsertPolicy::InsertRight )
			{
				orientation = Qt::Horizontal;
			}

			int widthOrHeight = 0;
			if( orientation == Qt::Horizontal ) widthOrHeight = container->width();
			else							   widthOrHeight = container->height();

			QWidget* reference = container;
			QTabSplitter* splitter = dynamic_cast<QTabSplitter*>(container->parent());

			// insert left in parent, right in parent,... go on level upper
			if( insertPolicy == InsertPolicy::InsertFullLeft || insertPolicy == InsertPolicy::InsertFullRight ||
				insertPolicy == InsertPolicy::InsertFullTop || insertPolicy == InsertPolicy::InsertFullBottom )
			{
				// if there is one upper level of layout
				if( splitter )
				{
					reference = splitter;
					splitter = dynamic_cast<QTabSplitter*>(splitter->parent());

					if( splitter )
					{
						if( (splitter->orientation() == Qt::Vertical && (insertPolicy == QTabFramework::InsertFullLeft || insertPolicy == QTabFramework::InsertFullRight))
							|| (splitter->orientation() == Qt::Horizontal && (insertPolicy == QTabFramework::InsertFullTop || insertPolicy == QTabFramework::InsertFullBottom)) )
						{
							reference = splitter;
							splitter = dynamic_cast<QTabSplitter*>(splitter->parent());
						}
					}
				}
			}

			if( splitter && splitter->orientation() == orientation )
			{
				QTabContainer* newContainer = new QTabContainer( splitter, container->tabWindow, insertPolicy );
				newContainer->addTab( widget, widget->windowTitle() );
				int containerIndex = splitter->indexOf( reference );
				if( insertPolicy == InsertPolicy::InsertFullRight || insertPolicy == InsertPolicy::InsertFullBottom ||
					insertPolicy == InsertPolicy::InsertRight || insertPolicy == InsertPolicy::InsertBottom )
				{
					QList<int> sizes = splitter->sizes();
					splitter->insertWidget( containerIndex + 1, newContainer );
					sizes[containerIndex] -= widthOrHeight / 3;
					sizes.insert( containerIndex + 1, widthOrHeight / 3 - splitter->handleWidth() );
					splitter->setSizes( sizes );
				}
				else
				{
					QList<int> sizes = splitter->sizes();
					splitter->insertWidget( containerIndex, newContainer );
					sizes[containerIndex] -= widthOrHeight / 3;
					sizes.insert( containerIndex, widthOrHeight / 3 - splitter->handleWidth() );
					splitter->setSizes( sizes );
				}
			}
			else
			{
				QTabSplitter* newSplitter = new QTabSplitter( orientation, splitter ? (QWidget*)splitter : (QWidget*)container->tabWindow, container->tabWindow );
				QTabContainer* newContainer = new QTabContainer( newSplitter, container->tabWindow, insertPolicy );
				int containerIndex = -1;
				QList<int> sizes;
				if( splitter )
				{
					containerIndex = splitter->indexOf( reference );
					sizes = splitter->sizes();
				}
				newContainer->addTab( widget, widget->windowTitle() );
				reference->setParent( NULL ); // remove container from splitter or tabWindow
				if( !splitter )
				{
					container->tabWindow->setCentralWidget( newSplitter );
				}
				else
				{
					splitter->insertWidget( containerIndex, newSplitter );
					splitter->setSizes( sizes );
				}
				if( insertPolicy == InsertPolicy::InsertFullRight || insertPolicy == InsertPolicy::InsertFullBottom ||
					insertPolicy == InsertPolicy::InsertRight || insertPolicy == InsertPolicy::InsertBottom )
				{
					newSplitter->addWidget( reference );
					newSplitter->addWidget( newContainer );
					QList<int> sizes;
					sizes.append( widthOrHeight - widthOrHeight / 3 );
					sizes.append( widthOrHeight / 3 - newSplitter->handleWidth() );
					newSplitter->setSizes( sizes );
				}
				else
				{
					newSplitter->addWidget( newContainer );
					newSplitter->addWidget( reference );
					QList<int> sizes;
					sizes.append( widthOrHeight / 3 - newSplitter->handleWidth() );
					sizes.append( widthOrHeight - widthOrHeight / 3 );
					newSplitter->setSizes( sizes );
				}
			}

		}
		else
		{
			Q_ASSERT( false );
		}
	}
}

void QTabFramework::moveTab( QWidget* widget, QTabContainer* position, InsertPolicy insertPolicy, int tabIndex )
{
	QTabContainer* tabContainer = dynamic_cast<QTabContainer*>(widget->parent()->parent());
	int movedIndex = tabContainer->indexOf( widget );
	tabContainer->removeTab( movedIndex );
	addTab( widget, position, insertPolicy, tabIndex );
	removeContainerIfEmpty( tabContainer );
}

void QTabFramework::moveTabLater( QWidget* widget, QTabContainer* position, InsertPolicy insertPolicy, int tabIndex )
{
	QTimer::singleShot( 0, this, [=]() {
		addTab( widget, position, insertPolicy, tabIndex );
	} );
}


void QTabFramework::showFloatingWindows()
{
	if( !QMainWindow::isVisible() )
	{
		for( QList<QTabWindow*>::Iterator i = floatingWindows.begin(), end = floatingWindows.end(); i != end; ++i )
			(*i)->hide();
	}
	else
	{
		QList<QTabWindow*> floatingWindowsZOrder = this->floatingWindowsZOrder;
		for( QList<QTabWindow*>::Iterator i = floatingWindows.begin(), end = floatingWindows.end(); i != end; ++i )
			(*i)->show();
		for( QList<QTabWindow*>::Iterator i = floatingWindowsZOrder.begin(), end = floatingWindowsZOrder.end(); i != end; ++i )
			(*i)->raise();
		activateWindow();
	}
}

void QTabFramework::handleFocusChanged( QWidget* old, QWidget* now )
{
	if( now )
	{
		QWidget* widget = now;
		QWidget* parent = dynamic_cast<QWidget*>(now->parent());
		QWidget* parentParent = dynamic_cast<QWidget*>(parent->parent());
		while( parentParent )
		{
			QTabContainer* tabContainer = dynamic_cast<QTabContainer*>(parentParent);
			if( tabContainer )
			{
				if( tabContainer != lastContainerWithFocus )
				{
					if( lastContainerWithFocus )
					{
						lastContainerWithFocus->tabBar()->setProperty( "child_has_focus", false );
						lastContainerWithFocus->setStyleSheet( lastContainerWithFocus->styleSheet() );
					}
					lastContainerWithFocus = tabContainer;
					tabContainer->tabBar()->setProperty( "child_has_focus", true );
					tabContainer->setStyleSheet( tabContainer->styleSheet() );
				}
				if( tabContainer->tabWindow != tabContainer->tabWindow->tabFramework )
				{
					tabContainer->tabWindow->focusTab = widget;
					tabContainer->tabWindow->setWindowTitle( widget->windowTitle() );
				}
				break;
			}
			widget = parent;
			parent = parentParent;
			parentParent = dynamic_cast<QWidget*>(parent->parent());
		}
	}
	else
	{
		for( QList<QTabWindow*>::Iterator i = floatingWindows.begin(), end = floatingWindows.end(); i != end; ++i )
		{
			QTabWindow* tabWindow = *i;
			if( tabWindow->focusTab == old )
				tabWindow->focusTab = 0;
		}
	}
}

bool QTabFramework::removeContainerIfEmpty( QTabContainer* tabContainer, bool hide )
{
	if( tabContainer->count() == 0 )
	{
		QObject* parent = tabContainer->parent();
		QTabSplitter* splitter = dynamic_cast<QTabSplitter*>(parent);
		if( splitter )
		{
			if( splitter->count() == 2 )
			{
				// the splitter will be pointless, remove it
				QWidget* sibling = splitter->widget( 0 );
				if( sibling == tabContainer )
					sibling = splitter->widget( 1 );

				QTabContainer* siblingContainer = dynamic_cast<QTabContainer*>(sibling);
				if( siblingContainer )
				{
					switch( siblingContainer->place )
					{
					case InsertLeft:
					case InsertRight:
					case InsertTop:
					case InsertBottom:
					case InsertFullLeft:
					case InsertFullRight:
					case InsertFullTop:
					case InsertFullBottom:
						return false;
					}
				}

				// detach
				sibling->setParent( NULL );

				QTabSplitter* parentSplitter = dynamic_cast<QTabSplitter*>(splitter->parent());
				if( parentSplitter )
				{
					QList<int> sizes = parentSplitter->sizes();
					int splitterIndex = parentSplitter->indexOf( splitter );
					splitter->setParent( NULL );
					QTabSplitter* siblingSplitter = dynamic_cast<QTabSplitter*>(sibling);
					if( !siblingSplitter || siblingSplitter->orientation() != parentSplitter->orientation() )
					{
						parentSplitter->insertWidget( splitterIndex, sibling );
						parentSplitter->setSizes( sizes );
					}
					else
					{
						QList<int> sibSizes = siblingSplitter->sizes();
						while( siblingSplitter->count() > 0 )
						{
							QWidget* widget = siblingSplitter->widget( 0 );
							widget->setParent( NULL );
							parentSplitter->insertWidget( splitterIndex, widget );
							sizes.insert( splitterIndex, sibSizes[0] );
							sibSizes.removeFirst();
							++splitterIndex;
						}
						delete siblingSplitter;
					}
					delete splitter;
				}
				else
				{
					splitter->setParent( NULL );
					tabContainer->tabWindow->setCentralWidget( sibling );
					delete splitter;
				}
			}
			else
			{
				// remove container
				int splitCount = splitter->count();
				tabContainer->setParent( NULL );
				delete tabContainer;
				Q_ASSERT( splitter->count() == splitCount - 1 );
			}
		}
		else
		{
			if( hide )
			{
				if( tabContainer->tabWindow != this )
				{
					tabContainer->tabWindow->hide();
					return true;
				}
			}
			else
			{
				tabContainer->tabWindow->tabFramework->removeWindow( tabContainer->tabWindow );
				return true;
			}
		}
	}
	return false;
}

QTabWindow* QTabFramework::createWindow()
{
	QTabWindow* tabWindow = new QTabWindow( this );
	tabWindow->setWindowIcon( windowIcon() );
	tabWindow->setAttribute( Qt::WA_DeleteOnClose );
	floatingWindows.append( tabWindow );
	floatingWindowsZOrder.append( tabWindow );
	return tabWindow;
}

void QTabFramework::removeWindow( QTabWindow* window )
{
	if( window == this )
		return;
	floatingWindows.removeOne( window );
	floatingWindowsZOrder.removeOne( window );
	delete window;
}

void QTabFramework::closeEvent( QCloseEvent* event )
{
	qDeleteAll( floatingWindows );
	floatingWindows.clear();
	QWidget* centralWidget = this->centralWidget();
	if( centralWidget )
	{
		setCentralWidget( 0 );
		delete centralWidget;
	}

	QMainWindow::closeEvent( event ); // skip QTabWindow closeEvent handler
}

void QTabFramework::showEvent( QShowEvent* event )
{
	QTimer::singleShot( 0, this, SLOT( showFloatingWindows() ) );

	QTabWindow::showEvent( event );
}

bool QTabFramework::eventFilter( QObject* obj, QEvent* event )
{
	switch( event->type() )
	{
	case QEvent::WindowTitleChange:
		{
			QWidget* widget = dynamic_cast<QWidget*>(obj);

			QWidget* widgetParent = dynamic_cast<QWidget*>(widget->parent());
			if( widgetParent )
			{
				QTabContainer* tabContainer = dynamic_cast<QTabContainer*>(widgetParent->parent());
				if( tabContainer )
				{
					int index = tabContainer->indexOf( widget );
					QString title = widget->windowTitle();
					tabContainer->setTabText( index, title );
					tabContainer->setTabToolTip( index, title );
					if( tabContainer->tabWindow->focusTab == widget )
						tabContainer->tabWindow->setWindowTitle( title );
				}
			}
		}
		break;
	case QEvent::WindowActivate:
		{
			QTabWindow* tabWindow = dynamic_cast<QTabWindow*>(obj);
			if( floatingWindows.contains( tabWindow ) )
			{
				floatingWindowsZOrder.removeOne( tabWindow );
				floatingWindowsZOrder.append( tabWindow );
			}
		}
		break;

	default:
		break;
	}

	return QMainWindow::eventFilter( obj, event );
}

QArrowCenter::QArrowCenter( QWidget* parent ) : QWidget( parent ),
fullLeft( "://images/dock/DockLeft.png" ),
fullRight( "://images/dock/DockRight.png" ),
fullTop( "://images/dock/DockTop.png" ),
fullBottom( "://images/dock/DockBottom.png" ),
left( "://images/dock/DockLeft.png" ),
right( "://images/dock/DockRight.png" ),
top( "://images/dock/DockTop.png" ),
bottom( "://images/dock/DockBottom.png" ),
center( "://images/dock/DockInside.png" )
{
	setWindowFlags( Qt::Tool | Qt::FramelessWindowHint | Qt::WindowTransparentForInput );
	setAttribute( Qt::WA_TransparentForMouseEvents );
	setAttribute( Qt::WA_TranslucentBackground );
	imgSize = center.width();
	blockSize = imgSpace + imgSize + imgSpace;
	resize( 5 * blockSize + 1, 5 * blockSize + 1 );
}

void QArrowCenter::showOver( QWidget* over, QTabFramework::InsertPolicies policies_ )
{
	policies = policies_;
	int topY = policies.testFlag( QTabFramework::InsertFullTop ) ? 0 : blockSize;
	int leftX = policies.testFlag( QTabFramework::InsertFullLeft ) ? 0 : blockSize;
	int rightX = policies.testFlag( QTabFramework::InsertFullRight ) ? 5 * blockSize : 4 * blockSize;
	int botomY = policies.testFlag( QTabFramework::InsertFullBottom ) ? 5 * blockSize : 4 * blockSize;

	background.clear();
	background << QPointF( 2 * blockSize - bevelSize, 2 * blockSize ) << QPointF( 2 * blockSize, 2 * blockSize - bevelSize );
	background << QPointF( 2 * blockSize, topY ) << QPointF( 3 * blockSize, topY );
	background << QPointF( 3 * blockSize, 2 * blockSize - bevelSize ) << QPointF( 3 * blockSize + bevelSize, 2 * blockSize );
	background << QPointF( rightX, 2 * blockSize ) << QPointF( rightX, 3 * blockSize );
	background << QPointF( 3 * blockSize + bevelSize, 3 * blockSize ) << QPointF( 3 * blockSize, 3 * blockSize + bevelSize );
	background << QPointF( 3 * blockSize, botomY ) << QPointF( 2 * blockSize, botomY );
	background << QPointF( 2 * blockSize, 3 * blockSize + bevelSize ) << QPointF( 2 * blockSize - bevelSize, 3 * blockSize );
	background << QPointF( leftX, 3 * blockSize ) << QPointF( leftX, 2 * blockSize );

	QPoint overpos = over->mapToGlobal( over->rect().center() );
	overpos.rx() -= (width() / 2);
	overpos.ry() -= (height() / 2);
	move( overpos );
	raise();
	show();
}

void QArrowCenter::paintEvent( QPaintEvent * )
{
	QPainter painter( this );
	painter.setOpacity( 0.75 );
	painter.setPen( QColor( Qt::lightGray ) );
	painter.setBrush( QColor( Qt::lightGray ).light( 120 ) );
	painter.drawPolygon( background );
	painter.setOpacity( 0.9 );

	if( policies.testFlag( QTabFramework::InsertFullTop ) )	painter.drawPixmap( 2 * blockSize + imgSpace, 0 * blockSize + imgSpace, fullTop );
	if( policies.testFlag( QTabFramework::InsertTop ) )		painter.drawPixmap( 2 * blockSize + imgSpace, 1 * blockSize + imgSpace, top );
	if( policies.testFlag( QTabFramework::InsertFullLeft ) )	painter.drawPixmap( 0 * blockSize + imgSpace, 2 * blockSize + imgSpace, fullLeft );
	if( policies.testFlag( QTabFramework::InsertLeft ) )		painter.drawPixmap( 1 * blockSize + imgSpace, 2 * blockSize + imgSpace, left );
	if( policies.testFlag( QTabFramework::InsertRight ) )		painter.drawPixmap( 3 * blockSize + imgSpace, 2 * blockSize + imgSpace, right );
	if( policies.testFlag( QTabFramework::InsertFullRight ) )	painter.drawPixmap( 4 * blockSize + imgSpace, 2 * blockSize + imgSpace, fullRight );
	if( policies.testFlag( QTabFramework::InsertOnTop ) )		painter.drawPixmap( 2 * blockSize + imgSpace, 2 * blockSize + imgSpace, center );
	if( policies.testFlag( QTabFramework::InsertBottom ) )		painter.drawPixmap( 2 * blockSize + imgSpace, 3 * blockSize + imgSpace, bottom );
	if( policies.testFlag( QTabFramework::InsertFullBottom ) )	painter.drawPixmap( 2 * blockSize + imgSpace, 4 * blockSize + imgSpace, fullBottom );
}

bool QArrowCenter::__isInImage( const QPoint& localPos, int x, int y )
{
	return x*blockSize <= localPos.x() && localPos.x() <= x*blockSize + blockSize &&
		y*blockSize <= localPos.y() && localPos.y() <= y*blockSize + blockSize;
}

QTabFramework::InsertPolicy QArrowCenter::findInsertPolicy( const QPoint& globalPos )
{
	QPoint localPos = mapFromGlobal( globalPos );

	if( policies.testFlag( QTabFramework::InsertFullTop ) && __isInImage( localPos, 2, 0 ) )		return QTabFramework::InsertFullTop;
	if( policies.testFlag( QTabFramework::InsertTop ) && __isInImage( localPos, 2, 1 ) )		return QTabFramework::InsertTop;
	if( policies.testFlag( QTabFramework::InsertFullLeft ) && __isInImage( localPos, 0, 2 ) )		return QTabFramework::InsertFullLeft;
	if( policies.testFlag( QTabFramework::InsertLeft ) && __isInImage( localPos, 1, 2 ) )		return QTabFramework::InsertLeft;
	if( policies.testFlag( QTabFramework::InsertRight ) && __isInImage( localPos, 3, 2 ) )		return QTabFramework::InsertRight;
	if( policies.testFlag( QTabFramework::InsertFullRight ) && __isInImage( localPos, 4, 2 ) )		return QTabFramework::InsertFullRight;
	if( policies.testFlag( QTabFramework::InsertOnTop ) && __isInImage( localPos, 2, 2 ) )		return QTabFramework::InsertOnTop;
	if( policies.testFlag( QTabFramework::InsertBottom ) && __isInImage( localPos, 2, 3 ) )		return QTabFramework::InsertBottom;
	if( policies.testFlag( QTabFramework::InsertFullBottom ) && __isInImage( localPos, 2, 4 ) )		return QTabFramework::InsertFullBottom;
	return QTabFramework::InsertFloating;
}

QTabFramework::InsertPolicy QTabFramework::bestInsertPolicy( QWidget* widget, bool bVertical )
{
	QWidget* current = dynamic_cast<QTabContainer*>(widget->parent());
	if( current == nullptr ) return NoInsert;

	forever
	{
		QSplitter* splitter = dynamic_cast<QSplitter*>(current->parent());
		if( splitter == nullptr ) return bVertical ? InsertBottom : InsertRight;
		if( bVertical && splitter->orientation() == Qt::Vertical )
		{
			if( splitter->indexOf( current ) + 1 == splitter->count() ) return InsertTop;
			return InsertBottom;
		}
		else if( !bVertical && splitter->orientation() == Qt::Horizontal )
		{
			if( splitter->indexOf( current ) + 1 == splitter->count() ) return InsertLeft;
			return InsertRight;
		}
		current = splitter;
	}
}

QJsonDocument QTabFramework::saveLayout()
{
	QJsonObject root;
	root["version"] = 1;

	// this windows
	root["mainwindow"] = writeLayout();
	// floating windows
	QJsonArray windows;
	for( QList<QTabWindow*>::Iterator i = floatingWindows.begin(), end = floatingWindows.end(); i != end; ++i )
	{
		QTabWindow* tabWindow = *i;
		windows.append( tabWindow->writeLayout() );
	}
	root["windows"] = windows;

	// z-order
	QJsonArray zOrder;
	for( QList<QTabWindow*>::Iterator i = floatingWindowsZOrder.begin(), end = floatingWindowsZOrder.end(); i != end; ++i )
		zOrder.append( floatingWindows.indexOf( *i ) );
	root["zorder"] = zOrder;

	QWidget* focuswidget = QApplication::focusWidget();
	if( focuswidget ) root["focus"] = focuswidget->objectName();

	return QJsonDocument( root );
}

void QTabFramework::restoreLayout( const QJsonDocument& doc, QTabFramework::restoreFactory factory )
{
	if( !doc.isObject() )
		return;
	QJsonObject root = doc.object();

	// check format version
	if( root["version"] != 1 )
		return;

	// this window
	readLayout( root["mainwindow"].toObject(), factory );
	// floating
	QJsonArray windows = root["windows"].toArray();
	for( auto wininfo : windows )
	{
		QTabWindow* tabWindow = createWindow();
		tabWindow->readLayout( wininfo.toObject(), factory );
	}

	// z-order
	floatingWindowsZOrder.clear();
	for( auto winindex : root["zorder"].toArray() ) floatingWindows.append( floatingWindows.at( winindex.toInt() ) );

	QString focusname = root["focus"].toString();
	QWidget* focuswidget = findChild<QWidget*>( focusname );
	if( focuswidget ) focuswidget->setFocus();

	showFloatingWindows();
}

QJsonObject QTabWindow::writeLayout()
{
	QJsonObject window;
	window["geometry"] = QJsonValue::fromVariant( saveGeometry() );
	window["state"] = QJsonValue::fromVariant( saveState() );
	QWidget* focuswidget = focusWidget();
	if( focuswidget ) window["focus"] = focuswidget->objectName();

	QWidget* widget = centralWidget();
	QTabSplitter* tabSplitter = dynamic_cast<QTabSplitter*>(widget);
	if( tabSplitter )
	{
		window["content"] = tabSplitter->writeLayout();
	}
	else
	{
		QTabContainer* tabContainer = dynamic_cast<QTabContainer*>(widget);
		if( tabContainer )
		{
			window["content"] = tabContainer->writeLayout();
		}
	}

	return window;
}

void QTabWindow::readLayout( const QJsonObject & window, QTabFramework::restoreFactory& factory )
{
	restoreGeometry( window["geometry"].toVariant().toByteArray() );
	restoreState( window["state"].toVariant().toByteArray() );

	QJsonObject child = window["content"].toObject();
	if( child["type"].toString() == "splitter" )
	{
		QTabSplitter* tabSplitter = new QTabSplitter( Qt::Horizontal, this, this );
		tabSplitter->readLayout( child, factory );
		setCentralWidget( tabSplitter );
	}
	else if( child["type"].toString() == "container" )
	{
		QTabContainer* tabContainer = new QTabContainer( this, this, QTabFramework::InsertOnTop );
		tabContainer->readLayout( child, factory );
		setCentralWidget( tabContainer );
	}

	QString focusname = window["focus"].toString();
	QWidget* focuswidget = findChild<QWidget*>( focusname );
	if( focuswidget ) focuswidget->setFocus();
}

QJsonObject QTabSplitter::writeLayout()
{
	QJsonObject object;
	object["type"] = "splitter";

	object["orientation"] = orientation() == Qt::Horizontal ? "Horizontal" : "Vertical";

	QJsonArray children;
	for( int i = 0, count = this->count(); i < count; ++i )
	{
		QWidget* widget = this->widget( i );
		QTabSplitter* tabSplitter = dynamic_cast<QTabSplitter*>(widget);
		if( tabSplitter )
		{
			children.append( tabSplitter->writeLayout() );
		}
		else
		{
			QTabContainer* tabContainer = dynamic_cast<QTabContainer*>(widget);
			if( tabContainer )
			{
				children.append( tabContainer->writeLayout() );
			}
		}
	}
	object["children"] = children;

	QJsonArray childrensize;
	for ( int size : sizes() ) { childrensize.append( size ); }
	object["sizes"] = childrensize;

	return object;
}

void QTabSplitter::readLayout( const QJsonObject & splitter, QTabFramework::restoreFactory& factory )
{
	setOrientation( splitter["orientation"].toString() == "Horizontal" ? Qt::Horizontal : Qt::Vertical );

	QJsonArray children = splitter["children"].toArray();
	for( auto childValue : children )
	{
		auto child = childValue.toObject();
		if( child["type"].toString() == "splitter" )
		{
			QTabSplitter* tabSplitter = new QTabSplitter( Qt::Horizontal, this, tabWindow );
			tabSplitter->readLayout( child, factory );
			addWidget( tabSplitter );
		}
		else if( child["type"].toString() == "container" )
		{
			QTabContainer* tabContainer = new QTabContainer( this, tabWindow, QTabFramework::InsertOnTop );
			tabContainer->readLayout( child, factory );
			addWidget( tabContainer );
		}
	}
	QJsonArray childrensizesarray = splitter["sizes"].toArray();
	QList<int> childrensizes;
	for( auto js : childrensizesarray ) { childrensizes.append( js.toInt() ); }
	setSizes( childrensizes );
}

QJsonObject QTabContainer::writeLayout()
{
	QJsonObject object;
	object["type"] = "container";
	object["currentIndex"] = currentIndex();
	object["place"] = place;

	QJsonArray children;
	for( int i = 0, count = this->count(); i < count; ++i )
	{
		QWidget* widget = this->widget( i );
		QJsonObject child;
		child["name"] = widget->objectName();
		children.append( child );
	}
	object["children"] = children;
	return object;
}

void QTabContainer::readLayout( const QJsonObject & container, QTabFramework::restoreFactory& factory )
{
	QJsonArray children = container["children"].toArray();
	for( auto childValue : children )
	{
		auto child = childValue.toObject();
		QWidget* childWidget = factory( child["name"].toString() );
		if( childWidget )
		{
			childWidget->installEventFilter( tabWindow->tabFramework );
			addTab( childWidget, childWidget->windowTitle() );
		}
	}
	setCurrentIndex( container["currentIndex"].toInt() );
	place = (QTabFramework::InsertPolicy)(container["place"].toInt());
}