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


#ifndef DSVIEW_PV_SIGNAL_H
#define DSVIEW_PV_SIGNAL_H
 

#include <QColor>
#include <QPainter>
#include <QPen>
#include <QRect>
#include <QString>

#include <stdint.h>
#include <list>

#include <libsigrok4DSL/libsigrok.h>
#include "trace.h"

namespace pv {

namespace data {
class SignalData;
}

namespace device {
class DevInst;
}

using namespace pv::device;

namespace view {

//draw signal trace base class
class Signal : public Trace
{
    Q_OBJECT

private:


protected:
    Signal(DevInst* dev_inst,sr_channel * const probe);

    /**
     * Copy constructor
     */
    Signal(const Signal &s, sr_channel * const probe);

public:
    virtual pv::data::SignalData* data() = 0;

    /**
     * Returns true if the trace is visible and enabled.
     */
    bool enabled();

    /**
     * Sets the name of the signal.
     */
    void set_name(QString name);

	/**
	 * Paints the signal label into a QGLWidget.
	 * @param p the QPainter to paint into.
	 * @param right the x-coordinate of the right edge of the header
	 * 	area.
	 * @param hover true if the label is being hovered over by the mouse.
     * @param action mouse position for hover
	 */
    //virtual void paint_label(QPainter &p, int right, bool hover, int action);

    DevInst* get_device();

protected:
    DevInst* _dev_inst;
    sr_channel *const _probe;
};

} // namespace view
} // namespace pv

#endif // DSVIEW_PV_SIGNAL_H
