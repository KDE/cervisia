/* 
 *  Copyright (C) 2001 Colin MacLeod
 *                     colin.macleod@ivata.com
 *
 * This program may be distributed under the terms of the Q Public
 * License as defined by Trolltech AS of Norway and appearing in the
 * file LICENSE.QPL included in the packaging of this file.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 */

#include "repositorysettingsdlg.h"

#include <qhbuttongroup.h>
#include <qpushbutton.h>
#include <qradiobutton.h>
#include <qlayout.h>
#include <qvariant.h>
#include <qtooltip.h>
#include <qwhatsthis.h>

#include <kapplication.h>
#include <kconfig.h>
#include <klocale.h>
#include <kbuttonbox.h>

#include "cervisiapart.h"
#include "repositorysettingsdlg.moc"

RepositorySettingsDialog::RepositorySettingsDialog(QWidget* parent,  const char* name)
    : QDialog(parent, name, true)
{
    if (!name)
      setName("RepositorySettingsDialog");

    setCaption(i18n("Repository Settings"));

    RepositorySettingsDialogLayout = new QVBoxLayout(this);     
    RepositorySettingsDialogLayout->setSpacing(6);
    RepositorySettingsDialogLayout->setMargin(11);

    QVBoxLayout* Layout1 = new QVBoxLayout; 
    Layout1->setSpacing(6);
    Layout1->setMargin(0);

    // set up a group for all the radio buttons
    groupCompression = new QHButtonGroup(i18n("Compression Level"), this);
    groupCompression->insert(radioCompressionDefault = new QRadioButton(i18n("Default"), groupCompression), -2);		
    groupCompression->insert(radioCompression0 = new QRadioButton("0", groupCompression), 0);		
    groupCompression->insert(radioCompression1 = new QRadioButton("1", groupCompression), 1);
    groupCompression->insert(radioCompression2 = new QRadioButton("2", groupCompression), 2);
    groupCompression->insert(radioCompression3 = new QRadioButton("3", groupCompression), 3);

    // add the group to the layout and the layout to the form
    Layout1->addWidget(groupCompression);
    Layout1->addStretch(  );
    RepositorySettingsDialogLayout->addLayout(Layout1);

    // new button group for ok, cancel and friends...
    KButtonBox* box = new KButtonBox(this, KButtonBox::Horizontal);
    box->addStretch();
    buttonOk = box->addButton(i18n("OK"));
    buttonCancel = box->addButton(i18n("&Cancel"));
    box->layout();
    RepositorySettingsDialogLayout->addWidget(box, 0);

    // signals and slots connections
    connect(buttonOk, SIGNAL(clicked()), this, SLOT(slotOkClicked()));
    buttonOk->setDefault(true);
    connect(buttonCancel, SIGNAL(clicked()), this, SLOT(reject()));
    
    resize(sizeHint());
}

/*  
 *  Destroys the object and frees any allocated resources
 */
RepositorySettingsDialog::~RepositorySettingsDialog()
{
    // no need to delete child widgets, Qt does it all for us
}

void RepositorySettingsDialog::slotOkClicked()
{
  // save off the compression level and close the dialog
  KConfig* config = CervisiaPart::config();
  
  // set the compression level
  config->setGroup(getConfigGroup(sRepository));
  // check it is not the default setting; if it is, leave empty
  if(groupCompression->selected() == radioCompressionDefault) {
    config->writeEntry("Compression", -1);
  // it's not the default
  } else { 
    config->writeEntry("Compression", groupCompression->id(groupCompression->selected()));
  }

  // just do the standard stuff when ok is pressed now
  accept();
}


// accessor methods
void RepositorySettingsDialog::setRepository(QString sNew) {
  sRepository = sNew;

  KConfig* config = CervisiaPart::config();
  
  groupCompression->id(radioCompressionDefault);
  // get the compression level
  config->setGroup(getConfigGroup(sRepository));
  int nCompression = config->readNumEntry("Compression", -1);

  setCaption(i18n("Repsitory Settings for ") + sNew);

  // if it is the default set the default radio
 if(nCompression < 0) {
    groupCompression->setButton(groupCompression->id(radioCompressionDefault));
  // it's not the default
  } else {
    groupCompression->setButton(nCompression);
  }
}
