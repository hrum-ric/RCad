#include "project.h"
#include <QtGui/QIcon>
#include <QDir>
#include <QtCore/QFile>
#include <QtCore/QSaveFile>
#include <QtCore/QFileInfo>
#include <QMimeData>
#include <QDebug>

#include <QJsonObject>
#include <QJsonArray>
#include <QJsonDocument>

#include <QtCore/QStandardPaths>
#include "RCadConstants.h"

#define TypeRole (Qt::UserRole)
#define NameRole (Qt::UserRole+1)

QDebug operator<<(QDebug debug, const Project::TreeItem& c)
{
	QDebugStateSaver saver( debug );

	debug.nospace().noquote();

	const Project::TreeItem* pc = &c;
	while( pc )
	{
		const Project::TreeItem*ppc = pc->parent();
		debug << (pc->name().isEmpty()?"<<noname>>":pc->name());
		if( ppc ) debug << "/";
		pc = ppc;
	}
	return debug;
}

Project* Project::pLoadProject(const QString& fileName, QString& errorString)
{
	QScopedPointer<Project> project(new Project());
	if (project->__bLoad(fileName, errorString)) return project.take();
	return nullptr;
}
Project* Project::pCreateProject(const QString& fileName, QString& errorString)
{
	QScopedPointer<Project> project(new Project());
	if (project->__bCreate(fileName, errorString)) return project.take();
	return nullptr;
}

Project::Project() : modified(false)
{
    rootItem = new TreeItem("", eNone);
    projectItem = new TreeItem("", eProject);
    rootItem->appendChild(projectItem);
    librariesItem = new TreeItem(tr("Libraries"), eLibraries);
    rootItem->appendChild(librariesItem);

    itemIcon[eProject]   = QIcon(":/images/project.png");
    itemIcon[eFolder]    = QIcon(":/images/folder.png");
    itemIcon[eSource]    = QIcon(":/images/source.png");
    itemIcon[eLibraries] = QIcon(":/images/sytem_folder.png");
    itemIcon[eLibrary]   = QIcon(":/images/library.png");
}

Project::~Project()
{
    delete rootItem;
}

bool Project::isModified() const
{
	return modified;
}

bool Project::__bCreate(const QString& fileName, QString& /*errorString*/)
{
	projectItem->setName(QFileInfo(fileName).baseName());
	projectFilename = fileName;
	modified = false;
	return true;
}

bool Project::__bLoad(const QString& fileName, QString& errorString)
{
    QFile file(fileName);
    if( !file.open(QIODevice::ReadOnly) )
    {
        errorString = file.errorString();
        return false;
    }

    projectItem->setName(QFileInfo(fileName).baseName());

	QJsonDocument projectdoc = QJsonDocument::fromJson( file.readAll() );
	if( !projectdoc.isObject() )
	{
		errorString = tr( "This version of RCad is too old to open this project." );
		return false;
	}

	QJsonObject project = projectdoc.object();

	if( project["format"] != "rcad" )
	{
		errorString = tr( "Invalid file format." );
		return false;
	}

	if( project["min_version"].toInt() > 1 )
	{
		errorString = tr( "This version of RCad is too old to open this project." );
		return false;
	}

	QJsonArray sources = project["sources"].toArray();
	for( auto child : sources )
	{
		__ReadSources( child, projectItem );
	}
	QJsonArray libraries = project["libraries"].toArray();
	for( auto child : libraries )
	{
		__ReadLibraries( child, librariesItem );
	}
 
    projectFilename = fileName;
	modified = false;
    return true;
}

void Project::__ReadSources(const QJsonValue& value, TreeItem* parent)
{
	if( value.isObject() )
	{
		QJsonObject obj = value.toObject();

		if( obj["type"] == "source" )
		{
			QString fileName = obj["filename"].toString();
			parent->appendChild( new TreeItem( fileName, eSource ) );
		}
		else if( obj["type"] == "folder" )
		{
			QString name = obj["name"].toString();
			if( !name.isEmpty() )
			{
				TreeItem* item = new TreeItem( name, eFolder );
				parent->appendChild( item );
				QJsonArray childrenarray = obj["children"].toArray();
				for( auto child : childrenarray )
				{
					__ReadSources( child, item );
				}
			}
		}
	}
}
void Project::__ReadLibraries( const QJsonValue& value, TreeItem* parent )
{
	if( value.isObject() )
	{
		QJsonObject obj = value.toObject();

		if( obj["type"] == "library" )
		{
			QString fileName = obj["name"].toString();
			parent->appendChild( new TreeItem( fileName, eLibrary ) );
		}
		else if( obj["type"] == "folder" )
		{
			QString name = obj["name"].toString();
			if( !name.isEmpty() )
			{
				TreeItem* item = new TreeItem( name, eFolder );
				parent->appendChild( item );
				QJsonArray childrenarray = obj["children"].toArray();
				for( auto child : childrenarray )
				{
					__ReadLibraries( child, item );
				}
			}
		}
	}
}

bool Project::bSave(QString& errorString)
{
    QSaveFile file(projectFilename);
    if( !file.open(QIODevice::WriteOnly) )
    {
        errorString = file.errorString();
        return false;
    }

	QJsonObject project;
	project["min_version"] = 1;
	project["format"] = "rcad";
	project["sources"] = projectItem->save();
	project["libraries"] = librariesItem->save();
	QJsonDocument jsondoc( project );
	QByteArray json_env = jsondoc.toJson( QJsonDocument::Indented );
	file.write( json_env );

    if( !file.commit() )
    {
        errorString = file.errorString();
        return false;
    }
	modified = false;
    return true;
}

QJsonValue	Project::TreeItem::save()
{
	switch( type() )
	{
	case eSource:
		{
			QJsonObject source;
			source["type"] = "source";
			source["filename"] = name();
			return source;
		}
	case eLibrary:
		{
			QJsonObject library;
			library["type"] = "library";
			library["name"] = name();
			return library;
		}
	case eFolder:
		{
			QJsonObject folder;
			folder["type"] = "folder";
			folder["name"] = name();
			QJsonArray childrenarray;
			foreach( TreeItem* item, children() ) childrenarray.append( item->save() );
			folder["children"] = childrenarray;
			return folder;
		}
	case eProject:
	case eLibraries:
		{
			QJsonArray childrenarray;
			foreach( TreeItem* item, children() ) childrenarray.append( item->save() );
			return childrenarray;
		}
	default:
		return QJsonValue();
	}
}

// path of RCAD libraries
QString Project::LibraryPath()
{
	return QStandardPaths::standardLocations( QStandardPaths::AppDataLocation ).first();
}

Project::eITEMTYPE Project::eGetType(const QModelIndex &index)
{
    if (!index.isValid()) return eNone;
    TreeItem *item = static_cast<TreeItem*>(index.internalPointer());
    if (nullptr==item) return eNone;
    return item->type();
}

QString	Project::__GetFileName( Project::TreeItem *item )
{
	if( item->type() == eSource ) return QFileInfo( projectFilename ).dir().absoluteFilePath( item->name() );
	if( item->type() == eLibrary ) return QDir( LibraryPath() ).absoluteFilePath( item->name() );

	return QString();
}

QString Project::GetFileName(const QModelIndex &index)
{
	if (!index.isValid()) return QString();
	TreeItem *item = static_cast<TreeItem*>(index.internalPointer());
	if (nullptr == item ) return QString();
	return __GetFileName( item );
}

QString Project::GetElementIdentifier( const QModelIndex &index )
{
	if( !index.isValid() ) return QString();
	TreeItem *item = static_cast<TreeItem*>(index.internalPointer());
	if( nullptr == item ) return QString();
	if( item->type() == eSource )  return SOURCE_IDENTIFIR_PREFIX + item->name();
	if( item->type() == eLibrary ) return LIBRARY_IDENTIFIR_PREFIX + item->name();
	return QString();
}

QString Project::GetFileNameFromIdentifier( QString identifier )
{
	eITEMTYPE eType = eNone;
	QString name;

	if( identifier.startsWith( SOURCE_IDENTIFIR_PREFIX ) ) { eType = eSource; name = identifier.remove( 0, NBCHAR( SOURCE_IDENTIFIR_PREFIX) ); }
	else if( identifier.startsWith( LIBRARY_IDENTIFIR_PREFIX ) ) { eType = eSource; name = identifier.remove( 0, NBCHAR( LIBRARY_IDENTIFIR_PREFIX ) ); }
	else return nullptr;

	TreeItem *item = rootItem->find( name, eType );
	if( item == nullptr ) return QString();
	return __GetFileName( item );
}

QModelIndex Project::__AddItemInFolder(QModelIndex &index, const QString& name, Project::eITEMTYPE eType)
{
    QModelIndex result;

    QModelIndex parent = index;
    if( !parent.isValid() ) parent = createIndex(0,0, projectItem);

    // go to the folder (or project) containing the source file)
    int nPosition=0;
    if( __getItem(parent)->type()==eSource )
    {
        nPosition = parent.row()+1;
        parent = parent.parent();
    }

    TreeItem *parentItem = __getItem(parent);

    // item can be under a folder or the prject
    if( parentItem->type()==eProject || parentItem->type()==eFolder )
    {
        beginInsertRows(parent,nPosition,nPosition);
        TreeItem* pFolder = new TreeItem(name,eType);
        parentItem->insertChild(nPosition,pFolder);
        result = createIndex(nPosition,1,pFolder);
        endInsertRows();
    }
	modified = true;
    return result;
}

QModelIndex Project::AddFolder(QModelIndex &index)
{
    return __AddItemInFolder(index,tr("unnamed"), eFolder);
}

void Project::AddSourceFile(QModelIndex &index, const QString& fileName)
{
	// store file in project directory as relative path
	QString fullFileName = QFileInfo(projectFilename).dir().relativeFilePath(fileName);
	if (fullFileName.startsWith("..")) fullFileName = fileName;

	// add extension
	if (!fullFileName.endsWith(".rcad")) fullFileName.append(".rcad");

	__AddItemInFolder(index, fullFileName, eSource);
}

void Project::AddLibrary(const QString& name)
{
    QModelIndex parent = createIndex(1,0, librariesItem);
    beginInsertRows(parent,0,0);
    librariesItem->insertChild(0,new TreeItem(name,eLibrary));
	modified = true;
    endInsertRows();
}

void Project::Remove(QModelIndex &index)
{
    TreeItem* item = __getItem(index);
    switch( item->type() )
    {
    case eFolder:
    case eSource:
    case eLibrary:
    {
        QModelIndex parentIndex = index.parent();
        TreeItem* parent = __getItem(parentIndex);
        int nPos = parent->position(item);

        beginRemoveRows(parentIndex,nPos,nPos);
        parent->removeChild(nPos);
		modified = true;
        endRemoveRows();
        break;
    }
    case eProject:
    case eLibraries:
    case eNone:
        break;
    }
}

////////////////////////////////////////////////////////////////////////////////
/// QAbstractItemModel
////////////////////////////////////////////////////////////////////////////////
Project::TreeItem *Project::__getItem(const QModelIndex &index) const
{
    if (index.isValid()) {
        TreeItem *item = static_cast<TreeItem*>(index.internalPointer());
        if (item)
            return item;
    }
    return rootItem;
}
QModelIndex Project::index(int row, int column, const QModelIndex &parent) const
{ 
    if (!hasIndex(row, column, parent))
            return QModelIndex();

    TreeItem *parentItem=__getItem(parent);
    TreeItem *childItem = parentItem->child(row);
    if (childItem)
        return createIndex(row, column, childItem);
    else
        return QModelIndex();
}

QModelIndex Project::parent(const QModelIndex &index) const
{
    if (!index.isValid())
           return QModelIndex();

   TreeItem *childItem = static_cast<TreeItem*>(index.internalPointer());
   TreeItem *parentItem = childItem->parent();

   if (parentItem == rootItem)
       return QModelIndex();

   return createIndex(parentItem->row(), 0, parentItem);
}

bool Project::insertRows( int row, int count, const QModelIndex &parent )
{
	if( !parent.isValid() ) return false;
	TreeItem *parentItem = static_cast<TreeItem*>(parent.internalPointer());

	if( row > parentItem->childCount() || row < 0 || count < 1 ) return false;

	beginInsertRows( parent, row, row + count - 1 );
	for( int i = 0; i < count; i++ )
	{
		TreeItem* child = new TreeItem;
		parentItem->insertChild( row, child );
	}
	modified = true;
	endInsertRows();
	return true;
}
bool Project::removeRows( int row, int count, const QModelIndex &parent )
{
	if( !parent.isValid() ) return false;
	TreeItem *parentItem = static_cast<TreeItem*>(parent.internalPointer());

	if( row < 0 || count <= 0 || row + count > parentItem->childCount() ) return false;
	
	beginRemoveRows( parent, row, row + count - 1 );
	for( int i = 0; i < count && row<parentItem->childCount(); i++ )
	{
		TreeItem* child = parentItem->child( row );
		parentItem->removeChild( row );
		delete child;
	}
	modified = true;
	endRemoveRows();

	return true;
}

int Project::rowCount(const QModelIndex &parent) const
{
    if (parent.column() > 0)
        return 0;

    TreeItem *parentItem=__getItem(parent);

    return parentItem->childCount();
}

int Project::columnCount(const QModelIndex &/*parent*/) const
{
    return 1;
}

QVariant Project::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    TreeItem *item = __getItem(index);

    if (role == Qt::DisplayRole)
    {        
		if (item->type() == eSource) return QVariant(QFileInfo(item->name()).baseName());
        return QVariant(item->name());
    }
    else if(role == Qt::DecorationRole)
    {
        return itemIcon[item->type()];
    }
	else if( role == Qt::EditRole && item->type() == eFolder )
	{
		return QVariant( item->name() );
	}
    return QVariant();
}

bool Project::setData(const QModelIndex &index, const QVariant &value, int role)
{
    TreeItem *item = __getItem(index);

	qDebug() << "setData " << *item << "  role:" << role << "  value:" << value;

	if( role == Qt::EditRole && item->type() == eFolder && value.canConvert<QString>() )
    {
        QString name = value.toString();
        if( !name.isEmpty() && item->parent()->position(name,eFolder)==-1 )
        {
			qDebug() << "set edit role";
            item->setName(name);
			modified = true;
            emit dataChanged(index, index);
            return true;
        }
    }
	else if( role == NameRole && (item->type() == eFolder || item->type() == eSource || item->type() == eLibrary) && value.canConvert<QString>() )
	{
		QString name = value.toString();
		if( !name.isEmpty() && item->parent()->position( name, eFolder ) == -1 )
		{
			qDebug() << "set name role";
			item->setName( name );
			modified = true;
			emit dataChanged( index, index );
			return true;
		}
	}
	else if( role == TypeRole && item->type() == eNone )
	{
		eITEMTYPE eType = (eITEMTYPE)value.toInt();
		if( eType == eFolder || eType == eSource || eType == eLibrary )
		{
			qDebug() << "set type role";
			item->setType( eType );
			modified = true;
			emit dataChanged( index, index );
			return true;
		}
	}
	
    return false;
}

void Project::__encodeItem( QDataStream& itemsStream, Project::TreeItem* item ) const
{
	itemsStream << (quint16)item->type() << item->name() << (quint32)item->childCount();
	for( int i = 0; i < item->childCount(); i++ )
		__encodeItem( itemsStream, item->child( i ) );
}

Project::TreeItem* Project::__decodeItem( QDataStream& itemsStream ) const
{
	quint16 type;
	itemsStream >> type;
	QString name;
	itemsStream >> name;
	quint32 childcount;
	itemsStream >> childcount;

	TreeItem* item = new TreeItem( name, (eITEMTYPE)type );
	for( quint32 i = 0; i < childcount; i++ )
		item->appendChild( __decodeItem( itemsStream ) );

	return item;
}

QStringList Project::mimeTypes() const
{
	QStringList ret;
	ret << "rcad/project/model";
	return ret;
}

QMimeData* Project::mimeData( const QModelIndexList &indexes ) const
{
	QMimeData *mimeData = new QMimeData();

	QByteArray itemsEncodedData;
	QDataStream itemsStream( &itemsEncodedData, QIODevice::WriteOnly );

	foreach( const QModelIndex &index, indexes )
	{
		if( index.isValid() )
		{
			TreeItem *item = static_cast<TreeItem*>(index.internalPointer());
			__encodeItem( itemsStream, item );
		}
	}
	mimeData->setData( "rcad/project/model", itemsEncodedData );
	return mimeData;
}

bool Project::dropMimeData( const QMimeData* data, Qt::DropAction action, int row, int /*column*/, const QModelIndex & parent )
{
	if( action == Qt::IgnoreAction ) return true;
	if( action != Qt::MoveAction ) return false;

	if( !parent.isValid() ) return false;
	TreeItem *parentItem = static_cast<TreeItem*>(parent.internalPointer());
	if( row < 0 || row > parentItem->childCount() ) row = parentItem->childCount();

	if( data->hasFormat( "rcad/project/model" ) )
	{
		emit layoutAboutToBeChanged();

		// First record all our items
		QByteArray itemsEncodedData = data->data( "rcad/project/model" );
		QDataStream itemsStream( &itemsEncodedData, QIODevice::ReadOnly );

		TreeItem* child = __decodeItem( itemsStream );

		parentItem->insertChild( row, child );

		modified = true;
		emit layoutChanged();
		return true;
	}
	return false;
}

QMap<int, QVariant> Project::itemData( const QModelIndex &index ) const
{
	QMap<int, QVariant> map;
	if( index.isValid() )
	{
		TreeItem *item = static_cast<TreeItem*>(index.internalPointer());
		map.insert( TypeRole, item->type() );
		map.insert( NameRole, item->name() );
	}
	return map;

}

Qt::ItemFlags Project::flags(const QModelIndex &index) const
{
    if (!index.isValid())
        return 0;

    TreeItem *item = static_cast<TreeItem*>(index.internalPointer());
	if( item->type() == eFolder ) return Qt::ItemIsEditable | Qt::ItemIsDragEnabled | Qt::ItemIsDropEnabled | QAbstractItemModel::flags( index );
	else if( item->type() == eSource ) return Qt::ItemIsEditable | Qt::ItemIsDragEnabled | QAbstractItemModel::flags( index );
	else if( item->type() == eProject ) return Qt::ItemIsDropEnabled | QAbstractItemModel::flags( index );
    else                        return QAbstractItemModel::flags(index);
}

////////////////////////////////////////////////////////////////////////////////
/// TreeItem
////////////////////////////////////////////////////////////////////////////////
Project::TreeItem::TreeItem(QString name, eITEMTYPE eType) : parentItem(nullptr)
{
    itemName = name;
    eItemType = eType;
}
void Project::TreeItem::appendChild(Project::TreeItem *item)
{
    item->parentItem = this;
    childItems.append(item);
}
void Project::TreeItem::insertChild(int nPosition, Project::TreeItem *item)
{
    item->parentItem = this;
    childItems.insert(nPosition,item);
}
int Project::TreeItem::position(Project::TreeItem *child)
{
    return childItems.indexOf(child);
}
int Project::TreeItem::position(QString name, eITEMTYPE type)
{
    for( int i=childItems.count()-1; i>=0; --i )
        if( childItems[i]->type()==type && childItems[i]->name()==name )
            return i;
    return -1;
}
Project::TreeItem* Project::TreeItem::find( QString name, eITEMTYPE eType )
{
	if( eType == eItemType && name.compare( itemName, Qt::CaseInsensitive )==0 )
		return this;
	for( auto child : childItems )
	{
		TreeItem* found = child->find( name, eType );
		if( found ) return found;
	}
	return nullptr;
}
void Project::TreeItem::removeChild(int nPosition)
{
    childItems.removeAt(nPosition);
}
Project::TreeItem* Project::TreeItem::child(int row)
{
    return childItems.value(row);
}
int Project::TreeItem::childCount() const
{
    return childItems.count();
}
int Project::TreeItem::row() const
{
    if (parentItem)
        return parentItem->childItems.indexOf(const_cast<TreeItem*>(this));
    return 0;
}
