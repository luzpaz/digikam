/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2011-05-23
 * Description : a tool to create panorama by fusion of several images.
 * Acknowledge : based on the expoblending tool
 *
 * Copyright (C) 2011-2016 by Benjamin Girault <benjamin dot girault at gmail dot com>
 * Copyright (C) 2009-2019 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General
 * Public License as published by the Free Software Foundation;
 * either version 2, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * ============================================================ */

#include "panowizard.h"

// KDE includes

#include <kconfig.h>
#include <klocalizedstring.h>

// Local includes.

#include "panomanager.h"
#include "panointropage.h"
#include "panoitemspage.h"
#include "panopreprocesspage.h"
#include "panooptimizepage.h"
#include "panopreviewpage.h"
#include "panolastpage.h"
#include "dxmlguiwindow.h"

namespace DigikamGenericPanoramaPlugin
{

class Q_DECL_HIDDEN PanoWizard::Private
{
public:

    explicit Private()
      : mngr(nullptr),
        introPage(nullptr),
        itemsPage(nullptr),
        preProcessingPage(nullptr),
        optimizePage(nullptr),
        previewPage(nullptr),
        lastPage(nullptr)
    {
    }

    PanoManager*           mngr;
    PanoIntroPage*         introPage;
    PanoItemsPage*         itemsPage;
    PanoPreProcessPage*    preProcessingPage;
    PanoOptimizePage*      optimizePage;
    PanoPreviewPage*       previewPage;
    PanoLastPage*          lastPage;
};

PanoWizard::PanoWizard(PanoManager* const mngr, QWidget* const parent)
    : DWizardDlg(parent, QLatin1String("Panorama Dialog")),
      d(new Private)
{
    setModal(false);
    setWindowTitle(i18nc("@title:window", "Panorama Creator Wizard"));

    d->mngr              = mngr;
    d->introPage         = new PanoIntroPage(d->mngr, this);
    d->itemsPage         = new PanoItemsPage(d->mngr, this);
    d->preProcessingPage = new PanoPreProcessPage(d->mngr, this);
    d->optimizePage      = new PanoOptimizePage(d->mngr, this);
    d->previewPage       = new PanoPreviewPage(d->mngr, this);
    d->lastPage          = new PanoLastPage(d->mngr, this);

    // ---------------------------------------------------------------

    connect(d->preProcessingPage, SIGNAL(signalPreProcessed(void)),
            this, SLOT(next(void)));

    connect(d->optimizePage, SIGNAL(signalOptimized(void)),
            this, SLOT(next(void)));

    connect(d->previewPage, SIGNAL(signalStitchingFinished(void)),
            this, SLOT(next(void)));

    connect(d->lastPage, SIGNAL(signalCopyFinished(void)),
            this, SLOT(accept(void)));
}

PanoWizard::~PanoWizard()
{
    delete d;
}

} // namespace DigikamGenericPanoramaPlugin
