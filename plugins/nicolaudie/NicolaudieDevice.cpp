/*
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Library General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 * NicolaudieDevice.cpp
 * The Nicolaudie usb chipset DMX plugin for ola
 * Copyright (C) 2011 Rui Barreiros
 */

#include <string>
#include <memory>
#include "ola/Logging.h"
#include "ola/base/Macro.h"
#include "plugins/nicolaudie/NicolaudieDevice.h"
//#include "plugins/nicolaudie/NicolaudiePort.h"

namespace ola {
namespace plugin {
namespace nicolaudie {

using std::string;

NicolaudieDevice::NicolaudieDevice(AbstractPlugin *owner,
                                   OLA_UNUSED DHDK_DEVICE_HANDLE device_handle)
    : Device(owner, 
//widget_info.Description()
             "foobar"
) {
/*      m_widget_info(widget_info),
  m_widget = new NicolaudieWidget(widget_info.Serial(),
                            widget_info.Name(),
                            widget_info.Id(),
                            widget_info.Vid(),
                            widget_info.Pid());*/
}

NicolaudieDevice::~NicolaudieDevice() {
  DeleteAllPorts();
//  delete m_widget;
}

bool NicolaudieDevice::StartHook() {
/*  unsigned int interface_count = m_widget->GetInterfaceCount();*/
  unsigned int successfully_added = 0;

/*  OLA_INFO << "Widget " << m_widget->Name() << " has " << interface_count
           << " interfaces.";

  for (unsigned int i = 1; i <= interface_count; i++) {
    NicolaudieInterface *port = new NicolaudieInterface(m_widget,
                                            static_cast<ftdi_interface>(i));
    if (port->SetupOutput()) {
      AddPort(new NicolaudieOutputPort(this, port, i));
      successfully_added += 1;
    } else {
      OLA_WARN << "Failed to add interface: " << i;
      delete port;
    }
  }*/
  if (successfully_added > 0) {
    OLA_INFO << "Successfully added " << successfully_added << "/"
//             << interface_count << " interfaces."
;
  } else {
    OLA_INFO << "Removing widget since no ports were added.";
    return false;
  }

  return true;
}
}  // namespace nicolaudie
}  // namespace plugin
}  // namespace ola
