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
 * NicolaudieDevice.h
 * The Nicolaudie usb chipset DMX plugin for ola
 * Copyright (C) 2011 Rui Barreiros
 */

#ifndef PLUGINS_NICOLAUDIE_NICOLAUDIEDEVICE_H_
#define PLUGINS_NICOLAUDIE_NICOLAUDIEDEVICE_H_

#include <string>
#include <memory>
#include "ola/DmxBuffer.h"
#include "olad/Device.h"
#include "olad/Preferences.h"
//#include "plugins/nicolaudie/NicolaudieWidget.h"

#include <LsHardDevKit.h>

namespace ola {
namespace plugin {
namespace nicolaudie {

class NicolaudieDevice : public Device {
 public:
  NicolaudieDevice(AbstractPlugin *owner,
                   DHDK_DEVICE_HANDLE device_handle);
  ~NicolaudieDevice();

  std::string DeviceId() const { return "42"; // TODO(Peter): Fixme
//return m_widget->Serial();
 }
  std::string Description() const { return "foobar"; }
//return m_widget_info.Description(); }
//  NicolaudieWidget* GetDevice() { return m_widget; }*/

  // We can send the same universe to multiple ports, or patch port 2 before
  // port 1
  bool AllowMultiPortPatching() const { return true; }

 protected:
  bool StartHook();

 private:
//  NicolaudieWidget *m_widget;
//  const NicolaudieWidgetInfo m_widget_info;
};
}  // namespace nicolaudie
}  // namespace plugin
}  // namespace ola
#endif  // PLUGINS_NICOLAUDIE_NICOLAUDIEDEVICE_H_
