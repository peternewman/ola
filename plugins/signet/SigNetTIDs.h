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
 * SigNetTIDs.h
 * TIDs used in Sig-Net packets
 * Copyright (C) 2026 Peter Newman
 */

#ifndef PLUGINS_SIGNET_SIGNETTIDS_H_
#define PLUGINS_SIGNET_SIGNETTIDS_H_

/**
 * @addtogroup signet
 * @{
 * @file SigNetTIDs.h
 * @brief Sig-Net TID values.
 * @}
 */

#include <stdint.h>

namespace ola {
namespace plugin {
namespace signet {

/**
 * @addtogroup signet
 * @{
 */

// Section 17, TID Dictionary
/**
 * @brief Sig-Net TIDs used at the root level.
 */
enum RootTID {
  TID_LEVEL = 0x0101,  /**< TID Level */
  TID_PRIORITY = 0x0102,  /**< TID Priority */
};
/**
 * @}
 */
}  // namespace signet
}  // namespace plugin
}  // namespace ola
#endif  // PLUGINS_SIGNET_SIGNETTIDS_H_
