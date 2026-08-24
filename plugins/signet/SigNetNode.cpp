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
 * SigNetNode.cpp
 * A self contained object for sending and receiving SigNet messages.
 * Copyright (C) 2026 Peter Newman
 */

#ifdef _WIN32
#include <ola/win/CleanWinSock2.h>
#endif  // _WIN32

#include <ola/Callback.h>
#include <ola/Constants.h>
#include <ola/ExportMap.h>
#include <ola/Logging.h>
#include <ola/network/InterfacePicker.h>
#include <ola/network/NetworkUtils.h>
#include <ola/rdm/UID.h>
#include <ola/stl/STLUtils.h>
#include <ola/strings/Format.h>
#include <ola/util/Utils.h>
#include <netdb.h>
#include <algorithm>
#include <string>
#include <utility>
#include <vector>
#include "plugins/signet/SigNetEnums.h"
#include "plugins/signet/SigNetNode.h"
#include "plugins/signet/SigNetTIDs.h"

namespace ola {
namespace plugin {
namespace signet {

#ifdef _WIN32
class UnmanagedSocketDescriptor : public ola::io::UnmanagedFileDescriptor {
 public:
  explicit UnmanagedSocketDescriptor(int fd) :
      ola::io::UnmanagedFileDescriptor(fd) {
    m_handle.m_type = ola::io::SOCKET_DESCRIPTOR;
    // Set socket to nonblocking to enable WSAEventSelect
    u_long mode = 1;
    ioctlsocket(fd, FIONBIO, &mode);
  }
 private:
  DISALLOW_COPY_AND_ASSIGN(UnmanagedSocketDescriptor);
};
#endif  // _WIN32

using ola::io::SelectServerInterface;
using ola::network::HostToNetwork;
using ola::network::IPV4Address;
using ola::network::IPV4SocketAddress;
using ola::network::NetworkToHost;
using ola::rdm::UID;
using ola::strings::IntToString;
using ola::utils::JoinUInt8;
using std::auto_ptr;
using std::make_pair;
using std::max;
using std::min;
using std::string;
using std::vector;

const char SigNetNode::SIGNET_PORT_VARIABLE[] = "signet-listen-port";
const vector<string> SigNetNode::SIGNET_LEVEL_URI({
    "sig-net", "v1", "local", "level"});

// TODO(Peter): Convert this into a SocketAddress?
coap_context_t* SigNetNode::GetCoapContext(const IPV4Address ip,
                                           const uint16_t port) {
  coap_context_t *ctx = NULL;
  struct addrinfo hints;
  struct addrinfo *result, *rp;

  memset(&hints, 0, sizeof(struct addrinfo));
  hints.ai_family = AF_INET;  // Allow IPv4
  hints.ai_socktype = SOCK_DGRAM;  // Coap is UDP based
  hints.ai_flags = AI_PASSIVE | AI_NUMERICHOST;

  int status = getaddrinfo(ip.ToString().c_str(), IntToString(port).c_str(),
                           &hints, &result);
  if ( status != 0 ) {
    OLA_WARN << "Failed to getaddrinfo: " << gai_strerror(status);
    return NULL;
  }

  // Loop through the results until we've found a suitable address
  for (rp = result; rp != NULL; rp = rp->ai_next) {
    coap_address_t addr;

    if (rp->ai_addrlen <= sizeof(addr.addr)) {
      coap_address_init(&addr);
      addr.size = rp->ai_addrlen;
      memcpy(&addr.addr, rp->ai_addr, rp->ai_addrlen);

      ctx = coap_new_context(&addr);
      if (ctx) {
        break;
      }
    }
  }

  if (!ctx) {
    OLA_FATAL << "Failed to find a context for interface " << ip.ToString();
  }

  freeaddrinfo(result);
  return ctx;
}

void SigNetUniverseHandler(OLA_UNUSED coap_context_t *ctx,
                           struct coap_resource_t *resource,
                           OLA_UNUSED const coap_endpoint_t *local_interface,
                           OLA_UNUSED coap_address_t *peer,
                           coap_pdu_t *request,
                           OLA_UNUSED str *token,
                           OLA_UNUSED coap_pdu_t *response) {
  string uri = string(reinterpret_cast<char*>(resource->uri.s),
                      resource->uri.length);
  OLA_INFO << "POST Called! " << uri;

  SigNetNode *node = NULL;

  string *rname = new string("ola-sig-net-node-ptr");

  coap_attr_t *attr;
  attr = coap_find_attr(resource,
                        (unsigned char *)rname->c_str(),
                        rname->length());

  if (attr) {
    OLA_DEBUG << "Found our attr";

    string val = string(reinterpret_cast<char*>(attr->value.s),
                        attr->value.length);
    OLA_INFO << "Attr val: " << val;

    node = reinterpret_cast<SigNetNode*>(attr->value.s);
    if (node) {
          OLA_DEBUG << "Still listening on " << node->ListeningPort();
    }
  } else {
    OLA_DEBUG << "Didn't find our attr";
  }

  coap_opt_iterator_t opt_iter;
  coap_opt_t *option;

  string uri_part;
  unsigned int uri_part_i = 0;
  uint16_t universe = 0;
  int signet_security_mode = -1;
  int signet_mfg_code = -1;
  int signet_session_id = -1;
  int signet_seq_num = -1;
  UID signet_sender_id_tuid(0, 0);
  uint16_t signet_sender_id_endpoint = 0;
  std::ostringstream format_data_str;

  coap_option_iterator_init((coap_pdu_t *)request, &opt_iter, COAP_OPT_ALL);

  while ((option = coap_option_next(&opt_iter))) {
    switch (opt_iter.type) {
      case COAP_OPTION_URI_PATH:
        uri_part = string(reinterpret_cast<char*>(coap_opt_value(option)),
                          coap_opt_length(option));
        if (uri_part_i < SigNetNode::SIGNET_LEVEL_URI.size()) {
          OLA_INFO << "Got COAP URI part " << uri_part << " expected "
                   << SigNetNode::SIGNET_LEVEL_URI[uri_part_i];
        } else {
          if (uri_part_i == SigNetNode::SIGNET_LEVEL_URI.size()) {
            OLA_INFO << "Got COAP URI part " << uri_part;
            if (ola::StringToInt(uri_part, &universe, true)) {
              OLA_DEBUG << "Got universe from COAP URI part: " << universe;
            } else {
              universe = 0;
              OLA_WARN << "Failed to parse universe from URI part";
            }
          } else {
            OLA_WARN << "Got too many extra URI parts";
          }
        }

        uri_part_i++;
        break;
      case SIGNET_SECURITY_MODE:
        signet_security_mode = static_cast<int>(coap_decode_var_bytes(
            coap_opt_value(option), coap_opt_length(option)));
        OLA_INFO << "Got Sig-Net-Security-Mode " << signet_security_mode;
        break;
      case SIGNET_SENDER_ID:
        OLA_INFO << "Got Sig-Net-Sender-ID";
        signet_sender_id_tuid = UID(coap_opt_value(option));
        // TODO(Peter): Check length of UID/TUID before use!
        OLA_INFO << "Got Sig-Net-Sender-ID (TUID) "
                 << signet_sender_id_tuid.ToString();
        signet_sender_id_endpoint = static_cast<int>(coap_decode_var_bytes(
            coap_opt_value(option) + UID::LENGTH,
            sizeof(signet_sender_id_endpoint)));
        OLA_INFO << "Got Sig-Net-Sender-ID (Endpoint) "
                 << signet_sender_id_endpoint;
        break;
      case SIGNET_MFG_CODE:
        signet_mfg_code = static_cast<int>(coap_decode_var_bytes(
            coap_opt_value(option), coap_opt_length(option)));
        OLA_INFO << "Got Sig-Net-Mfg-Code " << signet_mfg_code;
        break;
      case SIGNET_SESSION_ID:
        signet_session_id = static_cast<int>(coap_decode_var_bytes(
            coap_opt_value(option), coap_opt_length(option)));
        OLA_INFO << "Got Sig-Net-Session-ID " << signet_session_id;
        break;
      case SIGNET_SEQ_NUM:
        signet_seq_num = static_cast<int>(coap_decode_var_bytes(
            coap_opt_value(option), coap_opt_length(option)));
        OLA_INFO << "Got Sig-Net-Seq-Num " << signet_seq_num;
        break;
      case SIGNET_AUTH:
        format_data_str.str("");
        FormatData(&format_data_str,
                   coap_opt_value(option),
                   coap_opt_length(option));
        OLA_INFO << "Got Sig-Net-Auth\n" << format_data_str.str();
        break;
      default:
        OLA_WARN << "Unknown COAP option " << opt_iter.type;
    }
  }

  size_t data_len;
  unsigned char *data;

  if (coap_get_data(request, &data_len, &data)) {
    OLA_DEBUG << "Raw Length: " << data_len;
    if (data_len >= 4) {
      uint16_t tid_data_type = JoinUInt8(data[0], data[1]);
      uint16_t tid_data_len = JoinUInt8(data[2], data[3]);
      data = data + 4;
      data_len = data_len - 4;
      OLA_DEBUG << "Got TID " << tid_data_type << " and length "
                << tid_data_len;
      if (tid_data_len < data_len) {
        OLA_WARN << "Not enough data, " << data_len
                 << " remaining, but TLV claims " << tid_data_len;
      } else {
        if (tid_data_type == TID_LEVEL) {
          OLA_INFO << "DMX Length: " << tid_data_len;
          DmxBuffer buffer = DmxBuffer(data, tid_data_len);
          OLA_INFO << "DMX: " << buffer.ToString();

          if (node) {
            unsigned int size = min(static_cast<uint32_t>(DMX_UNIVERSE_SIZE),
                                    static_cast<uint32_t>(tid_data_len));
            node->SetUniverse(universe, static_cast<uint8_t*>(data), size);
          } else {
            OLA_WARN << "No node to run SetUniverse on";
          }
        } else {
          OLA_INFO << "Unrecognised/unhandled TID " << tid_data_type;
        }
      }
    }
  }
}

/*
 * The Error handler for the SigNet server.
 */
void SigNetErrorHandler(int error_code, const char *msg, const char *stack) {
  string msg_str, stack_str;
  if (msg) {
    msg_str.assign(msg);
  }
  if (stack) {
    stack_str.assign(stack);
  }
  OLA_WARN << "SigNet Error. Code " << error_code << ", " << msg_str << ", "
           << stack_str;
}


/**
 * Called when liblo receives data.
 * @param signet_address the SigNet Address this data was sent to
 * @param types the SigNet data type for the data
 * @param argv the data itself
 * @param argc the number of data blocks
 * @param user_data a pointer to the SigNetNode object.
 */
/*int SigNetDataHandler(const char *signet_address, const char *types, lo_arg **argv,
                   int argc, lo_message, void *user_data) {
  OLA_DEBUG << "Got SigNet message for " << signet_address << ", types are " << types;

  SigNetNode *node = reinterpret_cast<SigNetNode*>(user_data);
  const string type(types);
  uint16_t slot;

      unsigned int size = min(static_cast<uint32_t>(DMX_UNIVERSE_SIZE),
                              lo_blob_datasize(blob));
      node->SetUniverse(
          signet_address, static_cast<uint8_t*>(lo_blob_dataptr(blob)), size);
      return 0;
  OLA_WARN << "Unknown SigNet message type " << type;
  return 0;
}*/


/*
 * Calculate the IP that corresponds to a universe.
 * @param universe the universe id
 * @param addr where to store the address
 * @return true if this is a valid Sig-Net universe, false otherwise
 */
bool SigNetNode::UniverseIP(uint16_t universe, IPV4Address *addr) {
  uint8_t universe_high = 0;
  uint8_t universe_low = 0;
  universe_low = ((universe-1) % 109) + 1;
  *addr = IPV4Address(
      HostToNetwork(ola::utils::JoinUInt8(239,
                                          254,
                                          universe_high,
                                          universe_low)));
  if (universe && (universe != 0xFFFF)) {
    return true;
  }

  OLA_WARN << "Universe " << universe << " isn't a valid Sig-Net universe";
  return false;
}


bool SigNetNode::UniverseURI(uint16_t universe, string *uri) {
  uri->assign("sig-net/v1/local/level/" + ola::strings::IntToString(universe));

  if (universe && (universe != 0xFFFF)) {
    return true;
  }

  OLA_WARN << "Universe " << universe << " isn't a valid Sig-Net universe";
  return false;
}


/**
 * Create a new SigNetNode.
 * @param ss the SelectServer to use
 * @param export_map a pointer to an ExportMap (may be NULL)
 * @param options the SigNetNodeOptions
 */
SigNetNode::SigNetNode(SelectServerInterface *ss,
                 ExportMap *export_map,
                 const SigNetNodeOptions &options)
    : m_ss(ss),
      m_listen_port(options.listen_port) {
  if (export_map) {
    // export the SigNet listening port if we have an export map
    ola::IntegerVariable *signet_port_var =
      export_map->GetIntegerVar(SIGNET_PORT_VARIABLE);
    signet_port_var->Set(options.listen_port);
  }
}


/**
 * Cleanup
 */
SigNetNode::~SigNetNode() {
  Stop();
}


/*
 * Setup the Node
 */
bool SigNetNode::Init() {
  OLA_DEBUG << "Listening on " << ListeningPort();

  auto_ptr<ola::network::InterfacePicker> picker(
    ola::network::InterfacePicker::NewPicker());
  // TODO(Peter): Use our preferred interface...
  if (!picker->ChooseInterface(&m_interface, "")) {
    OLA_INFO << "Failed to find an interface";
    return false;
  }

  if (!m_socket.Init()) {
    return false;
  }

  if (!m_socket.Bind(IPV4SocketAddress(IPV4Address::WildCard(),
                                       m_listen_port))) {
    return false;
  }

  if (!m_socket.EnableBroadcast()) {
    return false;
  }

//  m_socket.SetMulticastInterface(m_interface.ip_address);

//  m_socket.SetOnData(NewCallback(&m_incoming_udp_transport,
//                                 &IncomingUDPTransport::Receive));

  coap_set_log_level(LOG_DEBUG);

  m_coap_context = SigNetNode::GetCoapContext(IPV4Address::WildCard(),
                                              m_listen_port);
  if (!m_coap_context) {
    OLA_FATAL << "No COAP context!";
    return false;
  }

  // Get the socket descriptor that libcoap is using, create a
  // UnmanagedFileDescriptor, assign a callback and register with the
  // SelectServer.
  int fd = m_coap_context->sockfd;
#ifdef _WIN32
  m_descriptor.reset(new UnmanagedSocketDescriptor(fd));
#else
  m_descriptor.reset(new ola::io::UnmanagedFileDescriptor(fd));
#endif  // _WIN32
  m_descriptor->SetOnData(NewCallback(this, &SigNetNode::DescriptorReady));
  m_ss->AddReadDescriptor(m_descriptor.get());

  return true;
}


/**
 * Stop this node. This removes all registrations.
 */
void SigNetNode::Stop() {
  // TODO(Peter): Clean up m_coap_context

  // Clean up the m_output_map map
  OutputGroupMap::iterator group_iter = m_output_map.begin();
  for (; group_iter != m_output_map.end(); ++group_iter) {
    delete group_iter->second;
  }
  m_output_map.clear();

  // Delete all the RX callbacks.
  STLDeleteValues(&m_input_map);

  if (m_descriptor.get()) {
    // if there was an UnmanagedFileDescriptor, de-register from the
    // SelectServer and delete it.
    m_ss->RemoveReadDescriptor(m_descriptor.get());
    m_descriptor.reset();
  }

  // TODO(Peter): Free m_coap_context
}


bool SigNetNode::SetHandler(uint16_t universe,
                            OLA_UNUSED DmxBuffer *buffer,
                            OLA_UNUSED uint8_t *priority,
                            DMXCallback *callback) {
  IPV4Address addr;
  if (!UniverseIP(universe, &addr)) {
    OLA_WARN << "Unable to determine multicast group for universe "
             << universe;
    return false;
  }

  OLA_DEBUG << "Joining via handler " << addr;
  if (!m_socket.JoinMulticast(m_interface.ip_address, addr)) {
    OLA_WARN << "Failed to join multicast group " << addr;
    return false;
  }

  coap_resource_t *r;

  string *uri1 = new string();
  UniverseURI(universe, uri1);

//  OLA_DEBUG << "Universe URI: " << *uri1;

  r = coap_resource_init((unsigned char *)uri1->c_str(), uri1->length(),
                         COAP_RESOURCE_FLAGS_NOTIFY_NON);

  string *rname = new string("ola-sig-net-node-ptr");

  string val = string(reinterpret_cast<char*>(this), 4);
  OLA_INFO << "Orig Attr val: " << val;

  coap_add_attr(r,
                (unsigned char *)rname->c_str(),
                rname->length(),
                (unsigned char *)this,
                4,
                0);

  coap_register_handler(r, COAP_REQUEST_POST, SigNetUniverseHandler);

  coap_add_resource(m_coap_context, r);

  if (callback) {
    // register
    SigNetInputGroup *universe_data = STLFindOrNull(m_input_map, universe);
    if (universe_data) {
      OLA_WARN << "Attempt to register a second callback for " << universe;
      // delete the callback, since we have ownership of it
      delete callback;
      return false;
    } else {
      // This is a new registration, insert into the CallbackMap.
      m_input_map.insert(
          make_pair(universe, new SigNetInputGroup(callback)));
    }
  } else {
    // deregister
    STLRemoveAndDelete(&m_input_map, universe);
  }

  return true;
}

bool SigNetNode::RemoveHandler(uint16_t universe) {
  IPV4Address addr;
  if (!UniverseIP(universe, &addr)) {
    OLA_WARN << "Unable to determine multicast group for universe " <<
      universe;
    return false;
  }

  OLA_DEBUG << "Leaving via handler" << addr;
  if (!m_socket.LeaveMulticast(m_interface.ip_address, addr)) {
    OLA_WARN << "Failed to leave multicast group " << addr;
    return false;
  }

  return true;
}


/**
 * Send the DMX data to all targets registered for this group.
 * @param group the group to send the data to
 * @param dmx_data the DmxBuffer to send
 * @returns true if successfully sent, false if any error occurred.
 */
bool SigNetNode::SendData(unsigned int group,
                          OLA_UNUSED const ola::DmxBuffer &dmx_data) {
  SigNetOutputGroup *output_group = STLFindOrNull(m_output_map, group);
  if (!output_group) {
    OLA_WARN << "failed to find " << group;
    // group doesn't exist, just return
    return false;
  }

  // TODO(Peter): Fixme!
  return true;
}


/**
 * Called by SigNetDataHandler when there is new data.
 * @param signet_address the SigNet address this data arrived on
 * @param data the DmxBuffer containing the data.
 * @param size the number of slots.
 */
void SigNetNode::SetUniverse(const uint16_t universe, const uint8_t *data,
                             unsigned int size) {
  SigNetInputGroup *universe_data = STLFindOrNull(m_input_map, universe);
  if (!universe_data)
    return;

  universe_data->dmx.Set(data, size);
  if (universe_data->callback.get()) {
    universe_data->callback->Run(universe_data->dmx);
  }
}


/**
 * Return the listening port. Will be 0 if the node isn't setup.
 */
uint16_t SigNetNode::ListeningPort() const {
  return m_listen_port;
}


/**
 * Called when the SigNet FD is readable.
 */
void SigNetNode::DescriptorReady() {
  // Call into libcoap to read the received data
  coap_read(m_coap_context);
}


/**
 * Send an lo_message to each target in a set.
 * @param message the lo_message to send.
 * @param targets the list of targets to send the message to.
 */
/*bool SigNetNode::SendMessageToTargets(lo_message message,
                                   const SigNetTargetVector &targets) {
  bool ok = true;
  SigNetTargetVector::const_iterator target_iter = targets.begin();
  for (; target_iter != targets.end(); ++target_iter) {
    int ret = lo_send_message_from(
        (*target_iter)->liblo_address,
        m_signet_server,
        (*target_iter)->signet_address.c_str(),
        message);
    ok &= (ret > 0);
  }
  return ok;
}*/
}  // namespace signet
}  // namespace plugin
}  // namespace ola
