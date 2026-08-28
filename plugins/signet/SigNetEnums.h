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
 * SigNetEnums.h
 * General enums used in Sig-Net packets
 * Copyright (C) 2026 Peter Newman
 */

#ifndef PLUGINS_SIGNET_SIGNETENUMS_H_
#define PLUGINS_SIGNET_SIGNETENUMS_H_
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

/**
 * @brief A set of values representing Sig-Net COAP Options.
 * @note See section 8.3 of Sig-Net for more information.
 */
typedef enum {
  SIGNET_SECURITY_MODE = 2076, /**< Sig-Net-Security-Mode */
  SIGNET_SENDER_ID = 2108, /**< Sig-Net-Sender-ID */
  SIGNET_MFG_CODE = 2140, /**< Sig-Net-Mfg-Code */
  SIGNET_SESSION_ID = 2172, /**< Sig-Net-Session-ID */
  SIGNET_SEQ_NUM = 2204, /**< Sig-Net-Seq-Num */
  SIGNET_AUTH = 2236, /**<  */
} signet_coap_options;
/**
 * @}
 */
}  // namespace signet
}  // namespace plugin
}  // namespace ola
#endif  // PLUGINS_SIGNET_SIGNETENUMS_H_
