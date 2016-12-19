/*
*     Wolfpack Emu (WP)
* UO Server Emulation Program
*
* Copyright 2001-2016 by holders identified in AUTHORS.txt
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation; either version 2 of the License, or
* (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program; if not, write to the Free Software
* Foundation, Inc., 59 Temple Palace - Suite 330, Boston, MA 02111-1307, USA.
*
* In addition to that license, if you are running this program or modified
* versions of it on a public system you HAVE TO make the complete source of
* the version used by you available or provide people with a location to
* download it.
*
* Wolfpack Homepage: http://developer.berlios.de/projects/wolfpack/
*/


#include "profilersessionmodel.h"
#include <QtGui>
#include <QList>

class TreeItem
{
public:
	TreeItem( TreeItem* parent = 0 )
	{
		parentItem = parent;
	}

	TreeItem(const QList<QVariant> &data, TreeItem *parent = 0)
	{
		parentItem = parent;
		itemData = data;
	}

	~TreeItem()
	{
		qDeleteAll(childItems);
	}

	void appendChild(TreeItem *child)
	{
		childItems.append(child);
	}

	TreeItem *child(int row)
	{
		return childItems.value(row);
	}

	int childCount() const
	{
		return childItems.count();
	}

	int columnCount() const
	{
		return itemData.count();
	}

	QVariant data(int column) const
	{
		return itemData.value(column);
	}

	void setData( const QList<QVariant> &data )
	{
		itemData = data;
	}

	int row() const
	{
		if (parentItem)
			return parentItem->childItems.indexOf(const_cast<TreeItem*>(this));

		return 0;
	}

	TreeItem *parent()
	{
		return parentItem;
	}

private:
	QList<TreeItem*> childItems;
	QList<QVariant> itemData;
	TreeItem *parentItem;
};

ProfileSessionModel::ProfileSessionModel( const ProfileSession& data, QObject* parent /* = 0  */) : QAbstractItemModel( parent )
{
	QList<QVariant> rootData;
	rootData << tr("File") << tr("Function") << tr("Calls") << tr("Callees");
	rootItem = new TreeItem(rootData);
	setupModelData(data, rootItem);
}

ProfileSessionModel::~ProfileSessionModel()
{
	delete rootItem;
}

int ProfileSessionModel::columnCount(const QModelIndex &parent) const
{
	if (parent.isValid())
		return static_cast<TreeItem*>(parent.internalPointer())->columnCount();
	else
		return rootItem->columnCount();
}

QVariant ProfileSessionModel::data(const QModelIndex &index, int role) const
{
	if (!index.isValid())
		return QVariant();

	if (role != Qt::DisplayRole)
		return QVariant();

	TreeItem *item = static_cast<TreeItem*>(index.internalPointer());

	return item->data(index.column());
}

Qt::ItemFlags ProfileSessionModel::flags(const QModelIndex &index) const
{
	if (!index.isValid())
		return Qt::ItemIsEnabled;

	return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
}

QVariant ProfileSessionModel::headerData(int section, Qt::Orientation orientation,
							   int role) const
{
	if (orientation == Qt::Horizontal && role == Qt::DisplayRole)
		return rootItem->data(section);

	return QVariant();
}

QModelIndex ProfileSessionModel::index(int row, int column, const QModelIndex &parent) const
{
	TreeItem *parentItem;

	if (!parent.isValid())
		parentItem = rootItem;
	else
		parentItem = static_cast<TreeItem*>(parent.internalPointer());

	TreeItem *childItem = parentItem->child(row);
	if (childItem)
		return createIndex(row, column, childItem);
	else
		return QModelIndex();
}

QModelIndex ProfileSessionModel::parent(const QModelIndex &index) const
{
	if (!index.isValid())
		return QModelIndex();

	TreeItem *childItem = static_cast<TreeItem*>(index.internalPointer());
	TreeItem *parentItem = childItem->parent();

	if (parentItem == rootItem)
		return QModelIndex();

	return createIndex(parentItem->row(), 0, parentItem);
}

int ProfileSessionModel::rowCount(const QModelIndex &parent) const
{
	TreeItem *parentItem;

	if (!parent.isValid())
		parentItem = rootItem;
	else
		parentItem = static_cast<TreeItem*>(parent.internalPointer());

	return parentItem->childCount();
}

void ProfileSessionModel::setupModelData(const ProfileSession& data, TreeItem *parent)
{
	for ( ProfileSession::const_iterator it = data.constBegin(); it != data.constEnd(); ++it )
	{
		TreeItem* upperLevel = new TreeItem( parent );
		uint accumulatedCalls = 0;
		for ( QMap< QByteArray, ProfileData >::const_iterator funcs = it.value().constBegin(); funcs != it.value().constEnd(); ++funcs )
		{
			QList<QVariant> columnData;
			columnData << it.key() << funcs.key() << funcs.value().calls;
			accumulatedCalls += funcs.value().calls;
			TreeItem* function = new TreeItem( columnData, upperLevel );
			upperLevel->appendChild( function );
			foreach( QByteArray callee, funcs.value().callees )
			{
				function->appendChild( new TreeItem( QList<QVariant>() << "" << "" << "" << callee, function ) );
			}
		}
		upperLevel->setData( QList<QVariant>() << it.key() << tr("Accumulated") << accumulatedCalls );
		parent->appendChild( upperLevel );
	}
}
