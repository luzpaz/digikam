/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2009-03-11
 * Description : Qt item model for database entries, using AlbumManager
 *
 * Copyright (C) 2009-2011 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * Copyright (C) 2015      by Mohamed_Anwer <m_dot_anwer at gmx dot com>
 *
 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General
 * Public License as published by the Free Software Foundation;
 * either version 2, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * ============================================================ */

#include "itemalbumfiltermodel.h"

// Qt includes

#include <QTimer>

// Local includes

#include "itemfiltermodel_p.h"
#include "album.h"
#include "albummanager.h"
#include "coredbaccess.h"
#include "coredbchangesets.h"
#include "coredbwatch.h"
#include "itemalbummodel.h"

namespace Digikam
{

class Q_DECL_HIDDEN ItemAlbumFilterModelPrivate : public ItemFilterModel::ItemFilterModelPrivate
{
public:

    ItemAlbumFilterModelPrivate()
    {
        delayedAlbumNamesTimer = nullptr;
        delayedTagNamesTimer   = nullptr;
    }

    QHash<int, QString> tagNamesHash;
    QHash<int, QString> albumNamesHash;
    QTimer*             delayedAlbumNamesTimer;
    QTimer*             delayedTagNamesTimer;
};

ItemAlbumFilterModel::ItemAlbumFilterModel(QObject* const parent)
    : ItemFilterModel(*new ItemAlbumFilterModelPrivate, parent)
{
    Q_D(ItemAlbumFilterModel);

    connect(AlbumManager::instance(), SIGNAL(signalAlbumAdded(Album*)),
            this, SLOT(slotAlbumAdded(Album*)));

    connect(AlbumManager::instance(), SIGNAL(signalAlbumAboutToBeDeleted(Album*)),
            this, SLOT(slotAlbumAboutToBeDeleted(Album*)));

    connect(AlbumManager::instance(), SIGNAL(signalAlbumsCleared()),
            this, SLOT(slotAlbumsCleared()));

    connect(AlbumManager::instance(), SIGNAL(signalAlbumRenamed(Album*)),
            this, SLOT(slotAlbumRenamed(Album*)));

    d->delayedAlbumNamesTimer = new QTimer(this);
    d->delayedAlbumNamesTimer->setInterval(250);
    d->delayedAlbumNamesTimer->setSingleShot(true);

    d->delayedTagNamesTimer = new QTimer(this);
    d->delayedTagNamesTimer->setInterval(250);
    d->delayedTagNamesTimer->setSingleShot(true);

    connect(d->delayedAlbumNamesTimer, SIGNAL(timeout()),
            this, SLOT(slotDelayedAlbumNamesTimer()));

    connect(d->delayedTagNamesTimer, SIGNAL(timeout()),
            this, SLOT(slotDelayedTagNamesTimer()));

    foreach (Album* const a, AlbumManager::instance()->allPAlbums())
    {
        albumChange(a);
    }

    foreach (Album* const a, AlbumManager::instance()->allTAlbums())
    {
        albumChange(a);
    }
}

ItemAlbumFilterModel::~ItemAlbumFilterModel()
{
}

void ItemAlbumFilterModel::setSourceItemModel(ItemAlbumModel* model)
{
    ItemFilterModel::setSourceItemModel(model);
}

ItemAlbumModel* ItemAlbumFilterModel::sourceModel() const
{
    Q_D(const ItemAlbumFilterModel);

    return static_cast<ItemAlbumModel*>(d->imageModel);
}

void ItemAlbumFilterModel::prepareThumbnails(const QList<QModelIndex>& indexesToPrepare)
{
    sourceModel()->prepareThumbnails(mapListToSource(indexesToPrepare));
}

void ItemAlbumFilterModel::setItemFilterSettings(const ItemFilterSettings& s)
{
    Q_D(ItemAlbumFilterModel);

    ItemFilterSettings settings(s);
    settings.setAlbumNames(d->albumNamesHash);
    settings.setTagNames(d->tagNamesHash);
    ItemFilterModel::setItemFilterSettings(settings);
}

int ItemAlbumFilterModel::compareInfosCategories(const ItemInfo& left, const ItemInfo& right) const
{
    Q_D(const ItemAlbumFilterModel);

    switch (d->sorter.categorizationMode)
    {
        case ItemSortSettings::CategoryByAlbum:
        {
            int leftAlbumId          = left.albumId();
            int rightAlbumId         = right.albumId();
            PAlbum* const leftAlbum  = AlbumManager::instance()->findPAlbum(leftAlbumId);
            PAlbum* const rightAlbum = AlbumManager::instance()->findPAlbum(rightAlbumId);

            if (!leftAlbum || !rightAlbum)
            {
                return -1;
            }

            if (leftAlbum == rightAlbum)
            {
                return 0;
            }

            if (d->sorter.sortRole == ItemSortSettings::SortByCreationDate ||
                d->sorter.sortRole == ItemSortSettings::SortByModificationDate)
            {
                // Here we want to sort the _categories_ by _album_ date if images are sorted by date
                // We must still make sure that categorization is unique!
                QDate leftDate  = leftAlbum->date();
                QDate rightDate = rightAlbum->date();

                if (leftDate != rightDate)
                {

                    return ItemSortSettings::compareByOrder(leftDate > rightDate ? 1 : -1,
                                                            d->sorter.currentCategorizationSortOrder);
                }
            }

            return ItemSortSettings::naturalCompare(leftAlbum->albumPath(), rightAlbum->albumPath(),
                                                    d->sorter.currentCategorizationSortOrder,
                                                    d->sorter.categorizationCaseSensitivity,
                                                    d->sorter.strTypeNatural);
        }
        default:
        {
            return ItemFilterModel::compareInfosCategories(left, right);
        }
    }
}

void ItemAlbumFilterModel::albumChange(Album* album)
{
    Q_D(const ItemAlbumFilterModel);

    if (album->type() == Album::PHYSICAL)
    {
        d->delayedAlbumNamesTimer->start();
    }
    else if (album->type() == Album::TAG)
    {
        d->delayedTagNamesTimer->start();
    }
}

void ItemAlbumFilterModel::slotAlbumRenamed(Album* album)
{
    albumChange(album);
}

void ItemAlbumFilterModel::slotAlbumAdded(Album* album)
{
    albumChange(album);
}

void ItemAlbumFilterModel::slotAlbumAboutToBeDeleted(Album* album)
{
    albumChange(album);
}

void ItemAlbumFilterModel::slotAlbumsCleared()
{
    Q_D(ItemAlbumFilterModel);
    d->albumNamesHash.clear();
    d->tagNamesHash.clear();
}

void ItemAlbumFilterModel::slotDelayedAlbumNamesTimer()
{
    Q_D(ItemAlbumFilterModel);

    d->albumNamesHash = AlbumManager::instance()->albumTitles();

    if (d->filter.isFilteringByText())
    {
        setItemFilterSettings(d->filter);
    }
}

void ItemAlbumFilterModel::slotDelayedTagNamesTimer()
{
    Q_D(ItemAlbumFilterModel);

    d->tagNamesHash = AlbumManager::instance()->tagNames();

    if (d->filter.isFilteringByText() || d->filter.isFilteringByTags())
    {
        setItemFilterSettings(d->filter);
    }
}

} // namespace Digikam
