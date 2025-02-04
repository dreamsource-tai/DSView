/*
 * This file is part of the PulseView project.
 *
 * Copyright (C) 2013 Joel Holdsworth <joel@airwebreathe.org.uk>
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

#ifndef DSVIEW_PV_WIDGETS_DECODERGROUPBOX_H
#define DSVIEW_PV_WIDGETS_DECODERGROUPBOX_H

 
#include <QPushButton>
#include <QGridLayout>
#include <QToolBar>
#include <QScrollArea> 

namespace pv {

namespace data{
class DecoderStack;
namespace decode{
class Decoder;
}
}

namespace widgets {

class DecoderGroupBox : public QScrollArea
{
	Q_OBJECT

public:
    DecoderGroupBox(pv::data::DecoderStack *decoder_stack,
                    data::decode::Decoder *dec, QLayout *dec_layout,
                    QWidget *parent = NULL);
    ~DecoderGroupBox();
    bool eventFilter(QObject *o, QEvent *e);

signals:
	void show_hide_decoder();
    void show_hide_row();
    void del_stack(data::decode::Decoder *_dec);

private slots:
    void tog_icon();
    void on_del_stack();

private:
    QWidget *_widget;

    pv::data::DecoderStack  *_decoder_stack;
    data::decode::Decoder   *_dec;
    int _index;

    QGridLayout *_layout;
    QPushButton *_del_button;
    QPushButton *_show_button;
    std::list <QPushButton *> _row_show_button;
};

} // widgets
} // pv

#endif // DSVIEW_PV_WIDGETS_DECODERGROUPBOX_H
