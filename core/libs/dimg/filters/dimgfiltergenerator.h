/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2010-06-24
 * Description : class for creating a particular filter
 *
 * Copyright (C) 2010-2011 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
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

#ifndef DIGIKAM_DIMG_FILTER_GENERATOR_H
#define DIGIKAM_DIMG_FILTER_GENERATOR_H

// Qt includes

#include <QList>
#include <QString>
#include <QStringList>

// Local includes

#include "digikam_export.h"

namespace Digikam
{

class DImgThreadedFilter;

class DIGIKAM_EXPORT DImgFilterGenerator
{
public:

    DImgFilterGenerator() {};
    virtual ~DImgFilterGenerator() {};

    /// Returns a list with identifiers of supported filters
    virtual QStringList supportedFilters() = 0;

    /// Returns a list with the supported versions for the given identifier
    virtual QList<int>  supportedVersions(const QString& filterIdentifier) = 0;

    /// Returns a QString with filter name for displaying in views
    virtual QString displayableName(const QString& filterIdentifier) = 0;

    /// Convenience methods
    virtual bool isSupported(const QString& filterIdentifier);
    virtual bool isSupported(const QString& filterIdentifier, int version);

    /// Create the filter for the given combination of identifier and version
    virtual DImgThreadedFilter* createFilter(const QString& filterIdentifier, int version) = 0;
};

// -----------------------------------------------------------------------------------

template <class T>

class BasicDImgFilterGenerator : public DImgFilterGenerator
{
public:

    /** A sample implementation for one DImgThreadedFilter class.
     *  The class must provide two static methods, FilterIdentifier() and SupportedVersions().
     */

    BasicDImgFilterGenerator()
    {
    }

    QStringList supportedFilters() override
    {
        return QList<QString>() << T::FilterIdentifier();
    }

    QList<int> supportedVersions(const QString& filterIdentifier) override
    {
        if (filterIdentifier == T::FilterIdentifier())
        {
            return T::SupportedVersions();
        }

        return QList<int>();
    }

    DImgThreadedFilter* createFilter(const QString& filterIdentifier, int version) override
    {
        if (filterIdentifier == T::FilterIdentifier() &&
            T::SupportedVersions().contains(version))
        {
            T* t = new T;
            t->setFilterVersion(version);
            return t;
        }

        return nullptr;
    }

    QString displayableName(const QString& filterIdentifier) override
    {
        if (filterIdentifier == T::FilterIdentifier())
        {
            return T::DisplayableName();
        }

        return QString();
    }
};

} // namespace Digikam

#endif // DIGIKAM_DIMG_FILTER_GENERATOR_H
