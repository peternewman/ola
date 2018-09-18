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
 * NicolaudiePlugin.cpp
 * The NICOLAUDIE usb chipset DMX plugin for ola
 * Copyright (C) 2011 Rui Barreiros
 *
 * Additional modifications to enable support for multiple outputs and
 * additional device ids did change the original structure.
 *
 * by E.S. Rosenberg a.k.a. Keeper of the Keys 5774/2014
 */

#include <vector>
#include <string>

#include "ola/StringUtils.h"
#include "olad/Preferences.h"
#include "olad/PluginAdaptor.h"
#include "plugins/nicolaudie/NicolaudiePlugin.h"
#include "plugins/nicolaudie/NicolaudiePluginDescription.h"
//#include "plugins/nicolaudie/NicolaudieDevice.h"
//#include "plugins/nicolaudie/NicolaudieWidget.h"

#include <LsHardDevKit.h>

namespace ola {
namespace plugin {
namespace nicolaudie {

using std::string;
using std::vector;

const char NicolaudiePlugin::PLUGIN_NAME[] = "NICOLAUDIE USB DMX";
const char NicolaudiePlugin::PLUGIN_PREFIX[] = "nicolaudie";

/**
 * @brief Attempt to start a device and, if successful, register it
 *
 * Ownership of the NicolaudieDevice is transferred to us here.
 */
void NicolaudiePlugin::AddDevice(NicolaudieDevice *device) {
  if (device->Start()) {
      m_devices.push_back(device);
      m_plugin_adaptor->RegisterDevice(device);
  } else {
    OLA_WARN << "Failed to start Nicolaudie device " << device->Description();
    delete device;
  }
}


/**
 * @brief Fetch a list of all Nicolaudie widgets and create a new device for each of them.
 */
bool NicolaudiePlugin::StartHook() {
  DHDK_init(DHDK_DASUSB|DHDK_DASNET, "OLAd");

/*  typedef vector<NicolaudieWidgetInfo> NicolaudieWidgetInfoVector;
  NicolaudieWidgetInfoVector widgets;
  NicolaudieWidget::Widgets(&widgets);

  NicolaudieWidgetInfoVector::const_iterator iter;
  for (iter = widgets.begin(); iter != widgets.end(); ++iter) {
    AddDevice(new NicolaudieDevice(this, *iter));
  }*/

  if (DHDK_enumerate()) {
    DHDK_UINT device_count = DHDK_getDeviceCount();
    OLA_INFO << "Found " << device_count << " devices";
    for (DHDK_UINT i = 0; i < device_count; i++) {
      DHDK_DEVICE_HANDLE device_handle = DHDK_getDevice(i);
      if (device_handle != DHDK_INVALID_HANDLE) {
        OLA_WARN << "Got invalid handle for device " << i;
        AddDevice(new NicolaudieDevice(this, i));
      } else {
     }
    }
  } else {
    OLA_WARN << "Failed to enumerate devices";
  }
  return true;
}


/**
 * @brief Stop all the devices.
 */
bool NicolaudiePlugin::StopHook() {
  NicolaudieDeviceVector::iterator iter;
  for (iter = m_devices.begin(); iter != m_devices.end(); ++iter) {
    m_plugin_adaptor->UnregisterDevice(*iter);
//    (*iter)->Stop();
    delete (*iter);
  }
  m_devices.clear();
  DHDK_deinit();
  return true;
}


/**
 * @brief Return a description for this plugin.
 */
string NicolaudiePlugin::Description() const {
  return plugin_description;
}


/**
 * @brief Set the default preferences
 */
bool NicolaudiePlugin::SetDefaultPreferences() {
  if (!m_preferences) {
    return false;
  }

  return true;
}
}  // namespace nicolaudie
}  // namespace plugin
}  // namespace ola
