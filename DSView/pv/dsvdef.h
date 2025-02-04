/*
 * This file is part of the DSView project.
 * DSView is based on PulseView.
 *
 * Copyright (C) 2012 Joel Holdsworth <joel@airwebreathe.org.uk>
 * Copyright (C) 2013 DreamSourceLab <support@dreamsourcelab.com>
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

#pragma once

#include "../config.h"
#include <QString>


#ifdef DS_DEBUG_TRACE
    void ds_print(const char *s);
    
    #define ds_debug(x) ds_print((x))
#else
    #define ds_debug(x)
#endif

class QWidget;
class QTextStream;

#define DESTROY_OBJECT(p) if((p)){delete (p); p = NULL;} 
#define DESTROY_QT_OBJECT(p) if((p)){((p))->deleteLater(); p = NULL;}
#define DESTROY_QT_LATER(p) ((p))->deleteLater();

#define RELEASE_ARRAY(a)   for (auto ptr : (a)){delete ptr;} (a).clear();

#define ABS_VAL(x) (x>0?x:-x)

namespace DecoderDataFormat
{
    enum _data_format
    {
        hex=0,
        dec=1,       
        oct=2,
        bin=3,
        ascii=4
    };

    int Parse(const char *name);       
}

namespace app
{
    QWidget* get_app_window_instance(QWidget *ins, bool bSet);

    bool is_app_top_window(QWidget* w);

    void set_utf8(QTextStream &stream);
}

