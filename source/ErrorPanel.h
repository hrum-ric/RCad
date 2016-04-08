#ifndef __ERROR_PANEL__
#define __ERROR_PANEL__
#include <QTableView>
class ErrorList;
class ErrorPanelModel;
class TokenPositionBase;

class ErrorPanel : public QTableView
{
	Q_OBJECT

public:
	ErrorPanel(QWidget *parent = Q_NULLPTR);

	void setErrorList(ErrorList errorList);

signals:
	void gotoerror(QString file, const TokenPositionBase& position);

private:
	ErrorPanelModel* m_model;
};

#endif // __ERROR_PANEL__
