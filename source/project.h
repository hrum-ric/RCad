#ifndef PROJECT_H
#define PROJECT_H
#include <QtCore/QStringList>
#include <QtCore/QAbstractItemModel>
#include <QtGui/QIcon>
class QXmlStreamReader;
class QXmlStreamWriter;

class Project : public QAbstractItemModel
{
    class TreeItem;
public:
    enum eITEMTYPE { eProject, eFolder, eSource, eLibraries, eLibrary, eNone };

public:
	static Project* pLoadProject(const QString& fileName, QString& errorString);
	static Project* pCreateProject(const QString& fileName, QString& errorString);
    ~Project();

    QString     filename() const						{ return projectFilename; }
    bool        isModified() const;
    bool        bSave(QString& errorString);

public: // QAbstractItemModel
    virtual QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const Q_DECL_OVERRIDE;
    virtual QModelIndex parent(const QModelIndex &child) const Q_DECL_OVERRIDE;
    virtual int rowCount(const QModelIndex &parent = QModelIndex()) const Q_DECL_OVERRIDE;
    virtual int columnCount(const QModelIndex &parent = QModelIndex()) const Q_DECL_OVERRIDE;
    virtual QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const Q_DECL_OVERRIDE;
    virtual Qt::ItemFlags flags(const QModelIndex &index) const Q_DECL_OVERRIDE;
    virtual bool setData(const QModelIndex &index, const QVariant &value, int role);
	virtual Qt::DropActions supportedDropActions() const Q_DECL_OVERRIDE{ return Qt::MoveAction; }
	virtual bool insertRows( int row, int count, const QModelIndex &parent = QModelIndex() ) Q_DECL_OVERRIDE;
	virtual bool removeRows( int row, int count, const QModelIndex &parent = QModelIndex() ) Q_DECL_OVERRIDE;
	virtual QStringList mimeTypes() const Q_DECL_OVERRIDE;
	virtual QMimeData* mimeData( const QModelIndexList & indexes ) const Q_DECL_OVERRIDE;
	virtual bool dropMimeData( const QMimeData * data, Qt::DropAction action, int row, int column, const QModelIndex & parent ) Q_DECL_OVERRIDE;
	virtual QMap<int, QVariant> itemData( const QModelIndex &index ) const Q_DECL_OVERRIDE;

public:
	eITEMTYPE   eGetType(const QModelIndex &index);
	QString		GetFileName( const QModelIndex &index );
	QString		GetElementIdentifier( const QModelIndex &index );
	QString		GetFileNameFromIdentifier( QString identifier );

    QModelIndex AddFolder(QModelIndex &index);
    void        AddSourceFile(QModelIndex &index, const QString& fileName);
    void        AddLibrary(const QString& name);
    void        Remove(QModelIndex &index);

	// path of RCAD libraries
	QString		LibraryPath();

private:
	Project();
	bool				__bLoad(const QString& fileName, QString& errorString);
	bool				__bCreate(const QString& fileName, QString& errorString);
    TreeItem*			__getItem(const QModelIndex &index) const;
    QModelIndex			__AddItemInFolder(QModelIndex &index, const QString &name, Project::eITEMTYPE eType);
	void				__ReadSources( const QJsonValue& value, TreeItem* parent );
	void				__ReadLibraries( const QJsonValue& value, TreeItem* parent );
	QString				__GetFileName( Project::TreeItem *item );
	void				__encodeItem( QDataStream& itemsStream, Project::TreeItem* item ) const;
	Project::TreeItem*	__decodeItem( QDataStream& itemsStream ) const;

private:
    class TreeItem
    {
    public:
		explicit TreeItem() : eItemType(eNone) {}
        explicit TreeItem(QString name, eITEMTYPE eType);
        ~TreeItem() { qDeleteAll(childItems); }

        void		appendChild(TreeItem *child);
        void		insertChild(int nPosition, TreeItem *item);
        int			position(TreeItem *child);
        int			position(QString name, eITEMTYPE eType);
		TreeItem*	find( QString name, eITEMTYPE eType );
        void		removeChild(int nPosition);
		QJsonValue	save();

        TreeItem *child(int row);
        int childCount() const;
        const QList<TreeItem*>& children() { return childItems; }
        int row() const;
		TreeItem* parent()							{ return parentItem; }
		const TreeItem* parent() const				{ return parentItem; }
        QString name() const                        { return itemName; }
        void    setName(const QString& name)        { itemName = name; }
        eITEMTYPE type()                            { return eItemType; }
		void	setType( eITEMTYPE eType )			{ eItemType = eType;  }

    private:
        QList<TreeItem*> childItems;
        TreeItem         *parentItem;
        QString          itemName;
        eITEMTYPE        eItemType;
    };
	friend QDebug operator<<(QDebug debug, const Project::TreeItem& c);

private:
    TreeItem    *rootItem;
    TreeItem    *projectItem;
    TreeItem    *librariesItem;
    QIcon       itemIcon[eNone+1];
    QString     projectFilename;
	bool		modified;

};

#endif // PROJECT_H
