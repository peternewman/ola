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
 * NicolaudiePlugin.h
 * The Nicolaudie usb chipset DMX plugin for ola
 * Copyright (C) 2011 Rui Barreiros
 */

#ifndef PLUGINS_NICOLAUDIE_NICOLAUDIEPLUGIN_H_
#define PLUGINS_NICOLAUDIE_NICOLAUDIEPLUGIN_H_

#include <set>
#include <string>
#include <vector>

#include "olad/Plugin.h"
#include "ola/plugin_id.h"

#include "plugins/nicolaudie/NicolaudieDevice.h"

namespace ola {
namespace plugin {
namespace nicolaudie {


class NicolaudiePlugin : public Plugin {
 public:
  explicit NicolaudiePlugin(ola::PluginAdaptor *plugin_adaptor)
      : Plugin(plugin_adaptor) {
  }

  ola_plugin_id Id() const { return OLA_PLUGIN_NICOLAUDIE; }
  std::string Name() const { return PLUGIN_NAME; }
  std::string PluginPrefix() const { return PLUGIN_PREFIX; }
  bool DefaultMode() const { return true; }

  std::string Description() const;

 private:
  typedef std::vector<NicolaudieDevice*> NicolaudieDeviceVector;
  NicolaudieDeviceVector m_devices;

  void AddDevice(NicolaudieDevice *device);
  bool StartHook();
  bool StopHook();
  bool SetDefaultPreferences();

  static const char PLUGIN_NAME[];
  static const char PLUGIN_PREFIX[];
};
}  // namespace nicolaudie
}  // namespace plugin
}  // namespace ola
#endif  // PLUGINS_NICOLAUDIE_NICOLAUDIEPLUGIN_H_
