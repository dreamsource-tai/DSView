/*
 * This file is part of the DSView project.
 * DSView is based on PulseView.
 *
 * Copyright (C) 2016 DreamSourceLab <support@dreamsourcelab.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301 USA
 */


#include "dsdialog.h"
#include "shadow.h"

#include <QObject>
#include <QEvent>
#include <QMouseEvent>
#include <QVBoxLayout>
#include <QAbstractButton>
#include "../dsvdef.h"

namespace pv {
namespace dialogs {

DSDialog::DSDialog() : DSDialog(NULL, false, false)
{
}

DSDialog::DSDialog(QWidget *parent): DSDialog(parent, false, false)
{
}

DSDialog::DSDialog(QWidget *parent, bool hasClose): DSDialog(parent, hasClose, false)
{
}

DSDialog::DSDialog(QWidget *parent, bool hasClose, bool bBaseButton) :
    QDialog(NULL), //must be null, otherwise window can not able to move
    m_bBaseButton(bBaseButton)
{
    (void)parent;

    _base_layout = NULL;
    _main_layout = NULL;
    _main_widget = NULL;
    _titlebar = NULL;
    _shadow = NULL; 
    _base_button = NULL;

    m_callback = NULL; 
    _clickYes = false;
    
    setWindowFlags(Qt::Dialog | Qt::FramelessWindowHint | Qt::WindowSystemMenuHint);
    setAttribute(Qt::WA_TranslucentBackground);

    build_base(hasClose); 
}

DSDialog::~DSDialog()
{ 
    DESTROY_QT_OBJECT(_base_layout);
    DESTROY_QT_OBJECT(_main_layout);
    DESTROY_QT_OBJECT(_main_widget);
    DESTROY_QT_OBJECT(_titlebar);
    DESTROY_QT_OBJECT(_shadow);
    DESTROY_QT_OBJECT(_base_button);
}

void DSDialog::accept()
{  
    _clickYes = true;
    if (m_callback){
        m_callback->OnDlgResult(true);
    }


    QDialog::accept();
}

void DSDialog::reject()
{ 
    _clickYes = false;

    if (m_callback){
        m_callback->OnDlgResult(false);
    } 

    QDialog::reject();
}
  
void DSDialog::setTitle(QString title)
{
    if (_titlebar){
         _titlebar->setTitle(title);
    } 
}

void DSDialog::reload()
{
    show();
}

int DSDialog::exec()
{ 
      //ok,cancel
    if (m_bBaseButton){
        _base_button = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel,Qt::Horizontal, this);
         _main_layout->addWidget(_base_button);//, 5, 1, 1, 1, Qt::AlignHCenter | Qt::AlignBottom);
        //_main_layout->addWidget(_base_button,0, Qt::AlignHCenter | Qt::AlignBottom);
        connect(_base_button, SIGNAL(rejected()), this, SLOT(reject()));
        connect(_base_button, SIGNAL(accepted()), this, SLOT(accept()));
    }
 
    return QDialog::exec();
}


void DSDialog::build_base(bool hasClose)
{    
    _main_widget = new QWidget(this);
    _main_layout = new QVBoxLayout(_main_widget);
    _main_widget->setLayout(_main_layout);

    _shadow  = new Shadow(this);
    _shadow->setBlurRadius(10.0);
    _shadow->setDistance(3.0);
    _shadow->setColor(QColor(0, 0, 0, 80));
    _main_widget->setAutoFillBackground(true); 
    this->setGraphicsEffect(_shadow);

    _titlebar = new toolbars::TitleBar(false, this, hasClose);
     _main_layout->addWidget(_titlebar);

     QWidget *space = new QWidget(this);
     space->setFixedHeight(15);
    _main_layout->addWidget(space);

    _base_layout = new QVBoxLayout(this);   
    _base_layout->addWidget(_main_widget);
    setLayout(_base_layout); 

    _main_layout->setAlignment(Qt::AlignCenter | Qt::AlignTop);
    _main_layout->setContentsMargins(10,5,10,10);   
} 

} // namespace dialogs
} // namespace pv
