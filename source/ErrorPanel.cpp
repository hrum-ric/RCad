#include "ErrorPanel.h"
#include <QAbstractTableModel>
#include <QHeaderView>
#include <QSortFilterProxyModel>
#include <QFileInfo>
#include "ProgramTree.h"

class ErrorPanelModel : public QAbstractTableModel
{
public:
	void setErrorList(ErrorList errorList);

	virtual QVariant	headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const Q_DECL_OVERRIDE;
	virtual int			rowCount(const QModelIndex &) const Q_DECL_OVERRIDE;
	virtual int			columnCount(const QModelIndex &) const Q_DECL_OVERRIDE;
	virtual QVariant	data(const QModelIndex &, int) const Q_DECL_OVERRIDE;

	bool bGetError(const QModelIndex &index, ModuleErrorList& module, Error& error) const;

private:
	ErrorList	m_errorList;
};

ErrorPanel::ErrorPanel(QWidget *parent) : QTableView(parent)
{
	m_model = new ErrorPanelModel();
	QSortFilterProxyModel* proxyModel = new QSortFilterProxyModel(this);

	proxyModel->setSourceModel(m_model);
	setModel(proxyModel);

	setWindowTitle(tr("Error list"));

	horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
	horizontalHeader()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
	horizontalHeader()->setSectionResizeMode(2, QHeaderView::ResizeToContents);

	connect(this, &QTableView::doubleClicked, [this](const QModelIndex &index) 
	{
		ModuleErrorList module;
		Error error;
		if( m_model->bGetError(index,module,error) ) emit gotoerror(module.m_module,error.position());
	});

	setSortingEnabled(true);
}

void ErrorPanel::setErrorList(ErrorList errorList)
{
	m_model->setErrorList(errorList);
}

void ErrorPanelModel::setErrorList(ErrorList errorList)
{
	beginResetModel();
	m_errorList = errorList;
	endResetModel();
}

int ErrorPanelModel::rowCount(const QModelIndex & parent) const
{
	if(parent.isValid()) return 0;

	int nCount=0;
	for( auto m : m_errorList.m_list )
		for( auto e : m.m_errorList )
			nCount++;
	return nCount;
}

int ErrorPanelModel::columnCount(const QModelIndex &) const
{
	return 3;
}

QVariant ErrorPanelModel::headerData(int section, Qt::Orientation orientation, int role) const
{
	if(orientation == Qt::Horizontal && role == Qt::DisplayRole )
	{
		switch (section)
		{
		case 0: return QString(tr("Description"));
		case 1: return QString(tr("File"));
		case 2: return QString(tr("Line"));
		}   
	}
	return QVariant();
}

bool ErrorPanelModel::bGetError(const QModelIndex &index, ModuleErrorList& module, Error& error) const
{
	int row = index.row();
	for (auto m : m_errorList.m_list)
	{
		int moduleCount = m.m_errorList.size();
		if (row < moduleCount)
		{
			module = m;
			error = m.m_errorList.at(row);
			return true;
		}
		row -= moduleCount;
	}
	return false;
}

QVariant ErrorPanelModel::data(const QModelIndex &index, int role) const
{
	if (role == Qt::DisplayRole)
	{
		ModuleErrorList module; 
		Error error;
		if (bGetError(index, module, error))
		{
			switch (index.column())
			{
			case 0:  return error.message();
			case 1:  return QFileInfo(module.m_module).baseName();
			case 2:  return QVariant(error.startLine());
			default: return QVariant();
			}
		}
	}
	return QVariant();
}