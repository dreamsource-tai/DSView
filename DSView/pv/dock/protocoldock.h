/*
 * This file is part of the DSView project.
 * DSView is based on PulseView.
 *
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


#ifndef DSVIEW_PV_PROTOCOLDOCK_H
#define DSVIEW_PV_PROTOCOLDOCK_H

#include <libsigrokdecode4DSL/libsigrokdecode.h>

#include <QDockWidget>
#include <QPushButton>
#include <QLabel>
#include <QVector>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QScrollArea>
#include <QSplitter>
#include <QTableView>
#include <QSortFilterProxyModel>

#include <vector>
#include <mutex>

#include "../data/decodermodel.h"
#include "protocolitemlayer.h"
#include "../ui/dscombobox.h"


namespace pv {

class SigSession;
  
namespace data {
class DecoderModel;
}

namespace view {
class View;
}

namespace dock {
  
class ProtocolDock : public QScrollArea,public IProtocolItemLayerCallback
{
    Q_OBJECT

public:
    static const uint64_t ProgressRows = 100000;

public:
    ProtocolDock(QWidget *parent, view::View &view, SigSession *session);
    ~ProtocolDock();

    void del_all_protocol();
    bool sel_protocol(QString name);
    void add_protocol(bool silent);

private:
    void changeEvent(QEvent *event);
    void retranslateUi();
    void reStyle();

protected:
    void paintEvent(QPaintEvent *);
    void resizeEvent(QResizeEvent *);

private:
    //IProtocolItemLayerCallback
    void OnProtocolSetting(void *handle);
    void OnProtocolDelete(void *handle);
    void OnProtocolFormatChanged(QString format, void *handle);

signals:
    void protocol_updated();

public slots:
    void update_model();

private slots:
    void on_add_protocol(); 
    void on_del_all_protocol();
    void decoded_progress(int progress);
    void set_model();   
    void export_table_view();
    void nav_table_view();
    void item_clicked(const QModelIndex &index);
    void column_resize(int index, int old_size, int new_size);
    void search_pre();
    void search_nxt();
    void search_done();
    void search_changed();
    void search_update();

private:
    static int decoder_name_cmp(const void *a, const void *b);
    void resize_table_view(data::DecoderModel *decoder_model);

private:
    SigSession *_session;
    view::View &_view;
    QSortFilterProxyModel _model_proxy;
    int _cur_search_index;
    QStringList _str_list;

    QSplitter *_split_widget;
    QWidget *_up_widget;
    QWidget *_dn_widget;
    QTableView *_table_view;
    QPushButton *_pre_button;
    QPushButton *_nxt_button;
    QLineEdit *_search_edit;
    QHBoxLayout *_dn_search_layout;
    QVBoxLayout *_dn_layout;
    QLabel *_matchs_label;
    QLabel *_matchs_title_label;
    QLabel *_dn_title_label;

    QPushButton *_add_button;
    QPushButton *_del_all_button;
    DsComboBox *_protocol_combobox; 
    QVector <int > _protocol_index_list; 
    QVBoxLayout *_up_layout;
    QVector <ProtocolItemLayer*> _protocol_items; //protocol item layers

    QPushButton *_dn_set_button;
    QPushButton *_dn_save_button;
    QPushButton *_dn_nav_button;
    QPushButton *_search_button;

    mutable std::mutex _search_mutex;
    bool _search_edited;
    bool _searching;

    bool _add_silent;
};

} // namespace dock
} // namespace pv

#endif // DSVIEW_PV_PROTOCOLDOCK_H
