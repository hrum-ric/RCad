#include "colorpicker.h"
#include <QVBoxLayout>
#include <QListView>
#include <QPainter>
#include <QColorDialog>
#include <QFrame>
#include <QMouseEvent>

#define CELLSIZE (16+4)

class ColorPickerPopupDelegate : public QAbstractItemDelegate 
{
public:
	ColorPickerPopupDelegate(QObject *parent = 0) : QAbstractItemDelegate(parent) {}

	void paint(QPainter * painter, const QStyleOptionViewItem & option, const QModelIndex & index) const 
	{
		if (!index.isValid()) return;

		QColor c = qvariant_cast<QColor>(index.data(Qt::DecorationRole));
		painter->save();
		painter->setBrush(c);

		if (option.state & QStyle::State_Selected)
		{
			painter->setPen(Qt::blue);
			painter->setBrush(c);
			painter->drawRect(option.rect.adjusted(0, 0, -1, -1));
 			painter->setPen(QColor(Qt::blue).lighter());
 			painter->drawRect(option.rect.adjusted(1, 1, -2, -2));
		}
		else
		{
			painter->setPen(Qt::gray);
			painter->drawRect(option.rect.adjusted(0, 0, -1, -1));
 			painter->setPen(Qt::white);
			painter->drawRect(option.rect.adjusted(1, 1, -2, -2));
		}
		painter->restore();

	}
	QSize sizeHint(const QStyleOptionViewItem & /*option*/, const QModelIndex & /*index*/) const 
	{
		return QSize(CELLSIZE, CELLSIZE);
	}
};

class ColorPickerPopupModel : public QAbstractListModel
{
public:
	ColorPickerPopupModel()
	{
		for( int i=0; i<48; i++ )
			m_standardColor.append(QColorDialog::standardColor(i));
	}
	virtual int	rowCount(const QModelIndex&/*parent*/) const Q_DECL_OVERRIDE
	{
		return m_standardColor.count();
	}
	virtual QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const Q_DECL_OVERRIDE
	{
		if (role == Qt::DecorationRole)
		{
			return indiceToColor(index.row());
		}
		return QVariant();
	}
	int colorCount() const 
	{ 
		return m_standardColor.count(); 
	}
	const QColor& indiceToColor(int indice) const
	{
		int c = indice % 8;
		int r = indice / 8;
		return m_standardColor.at(r + c * 6);
	}
	QModelIndex getColorIndex(const QColor& color)
	{
		for( int i=0; i<m_standardColor.count(); i++ )
			if(indiceToColor(i)==color )
				return index(i);
		return QModelIndex();
	}

private:
	QList<QColor> m_standardColor;
};

class ColorView : public QListView
{
public:

	ColorView() 
	{
		setFlow(QListView::LeftToRight);
		setResizeMode(QListView::Adjust);
		setWrapping(true);
		setUniformItemSizes(true);
		setMouseTracking(true);
		setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
		setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
		setItemDelegate(new ColorPickerPopupDelegate(this));
		setSpacing(2);
		setStyleSheet("QListView { border: none; background-color: transparent; }");

		connect(this, &QListView::entered, [this](const QModelIndex &index)
		{
			selectionModel()->select(m_originalColorIndex, QItemSelectionModel::ClearAndSelect);
			selectionModel()->select(index, QItemSelectionModel::Select);
		});
	}

	void leaveEvent(QEvent* /*event*/)
	{
		selectionModel()->select(m_originalColorIndex, QItemSelectionModel::ClearAndSelect);
	}

	void setOriginalColorIndex(QModelIndex originalColorIndex)
	{
		m_originalColorIndex = originalColorIndex;
		selectionModel()->select(m_originalColorIndex, QItemSelectionModel::ClearAndSelect);
	}
private:
	QModelIndex m_originalColorIndex;
};

class ColorPickerPopup : public QFrame
{
public:
	ColorPickerPopup(ColorPicker *parent = 0) : QFrame(parent, Qt::Popup)
	{
		QVBoxLayout *l = new QVBoxLayout(this);
		l->setSpacing(0);
		l->setMargin(0);

		m_model = new ColorPickerPopupModel();
		m_view = new ColorView();
		m_view->setModel(m_model);
		l->addWidget(m_view);

		QPushButton* button = new QPushButton(tr("Other"));
		l->addWidget(button);
		
		connect(button, &QPushButton::clicked, [this,parent] 
		{			
			QColor c = QColorDialog::getColor(parent->color(), parent, tr("Choose color"), QColorDialog::ShowAlphaChannel);
			if (c.isValid()) 
			{
				parent->__selectColor(c);
				hide();
			}
		});

		connect(m_view, &QListView::clicked, [this, parent](const QModelIndex& index)
		{
			QColor c = qvariant_cast<QColor>(index.data(Qt::DecorationRole));
			parent->__selectColor(c);
			hide();
		});
		
		setFrameShape(QFrame::Box);
		setFrameShadow(QFrame::Plain);
		
		int cols = 8;
		int rows = (m_model->colorCount()%cols==0)?(m_model->colorCount()/cols):(1+ m_model->colorCount() / cols);

		int width = (CELLSIZE+2* m_view->spacing())*cols+1;
		int height = (CELLSIZE + 2 * m_view->spacing())*rows;
		m_view->setFixedSize(width, height);
	}

	void setOriginalColor(QColor color)
	{
		QModelIndex originalColorIndex = m_model->getColorIndex(color);
		m_view->setOriginalColorIndex(originalColorIndex);
	}

	void mousePressEvent(QMouseEvent *event)
	{
		if( parentWidget()->rect().contains(parentWidget()->mapFromGlobal(event->globalPos()))) 
		{
			ColorPicker* colorPicker = qobject_cast<ColorPicker*>(parentWidget());
			if(colorPicker) colorPicker->__ignoreNextClick();
		}
		QFrame::mousePressEvent(event);
	}
	
private:
	ColorView*				m_view;
	ColorPickerPopupModel*	m_model;
};

ColorPicker::ColorPicker(QWidget *parent) : QPushButton(parent), m_popup(nullptr), m_bIgnoreNextClick(false)
{
	setColor(Qt::white);

	connect(this, &QPushButton::clicked, [this]{
		if( m_popup==nullptr ) 
		{
			m_popup = new ColorPickerPopup(this);
		}
		if (m_bIgnoreNextClick)
		{
			m_bIgnoreNextClick=false;
		}
		else
		{
			QPoint p = mapToGlobal(rect().bottomLeft());
			m_popup->move(p);
			m_popup->setOriginalColor(m_color);
			m_popup->show();
		}
	});
}
void ColorPicker::__ignoreNextClick()
{
	m_bIgnoreNextClick=true;
}

QColor ColorPicker::color()
{
	return m_color;
}
void ColorPicker::setColor(QColor color)
{
	m_color = color;
	QString buttonstyle = QStringLiteral("ColorPicker {background-color: %1; border-width: 2px; border-style: solid; border-color: rgb(163, 163, 163); } ColorPicker:pressed{border-color: black;}");
	setStyleSheet(buttonstyle.arg(m_color.name()));
}
void ColorPicker::__selectColor(QColor color)
{
	setColor(color);
	emit colorSelected(color);
}
