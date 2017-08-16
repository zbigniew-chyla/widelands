/*
 * Copyright (C) 2008-2017 by the Widelands Development Team
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 *
 */

#ifndef WL_NETWORK_NETRELAYCONNECTION_H
#define WL_NETWORK_NETRELAYCONNECTION_H

#include <memory>

#include "network/network.h"
#include "network/relay_protocol.h"

/**
 * A wrapper around a network connection to the netrelay.
 * Does not contain logic but provides a buffer to read
 * uint8_t / std::string / SendPacket from the network stream.
 *
 * Additionally, the peek_* methods allow to check the contents of the buffer. They will not remove any bytes,
 * but will check whether the required value can be read. After checking, an internal peek-pointer will be
 * incremented. The next call of a peek_* method will then start reading after the previous value.
 * Calling peek_reset() will restore the pointer to the start of the buffer. Only a successful peek will
 * move the pointer.
 * Note that a successful peek does not mean that the received bytes really are of the requested type, it only
 * means they could be interpreted that way. Whether the type matches is in the responsibility of the caller.
 * The idea of this methods is that the caller can check whether a complete relay command can be received
 * before starting to remove data from the buffer. Otherwise, the caller would have to maintain an own buffer.
 */
class NetRelayConnection {
public:
	/**
	 * Tries to establish a connection to the given relay.
	 * \param host The host to connect to.
	 * \return A pointer to a connected \c NetRelayConnection object or a \c nullptr.
	 */
	static std::unique_ptr<NetRelayConnection> connect(const NetAddress& host);

	/**
	 * Closes the connection.
	 * If you want to send a goodbye-message to the relay, do so before freeing the object.
	 */
	~NetRelayConnection();

	/**
	 * Returns whether the relay is connected.
	 * \return \c true if the connection is open, \c false otherwise.
	 */
	bool is_connected() const;

	/**
	 * Closes the connection.
	 * If you want to send a goodbye-message to the relay, do so before calling this.
	 */
	void close();

	/**
	 * Checks whether a relay command can be read from the buffer.
	 * This method does not modify the buffer contents but increases the peek-pointer.
	 * \param cmd The command that will be returned next. It will not be removed from the input queue!
	 *            Can be nullptr.
	 * \return \c True if the value can be read, \c false if not enough data has been received.
	 */
	bool peek_cmd(RelayCommand *cmd = nullptr);

	/**
	 * Checks whether an uint8_t can be read from the buffer.
	 * This method does not modify the buffer contents but increases the peek-pointer.
	 * \return \c True if the value can be read, \c false if not enough data has been received.
	 */
	bool peek_uint8_t();

	/**
	 * Checks whether a std::string can be read from the buffer.
	 * This method does not modify the buffer contents but increases the peek-pointer.
	 * \return \c True if the value can be read, \c false if not enough data has been received.
	 */
	bool peek_string();

	/**
	 * Checks whether a RecvPacket can be read from the buffer.
	 * This method does not modify the buffer contents but increases the peek-pointer.
	 * \return \c True if the value can be read, \c false if not enough data has been received.
	 */
	bool peek_recvpacket();

	/**
	 * Resets the peek pointer to the beginning of the buffer.
	 */
	void peek_reset();

	/**
	 * Receive a command.
	 * \warning Calling this method is only safe when peek_cmd() returned \c true.
	 *          Otherwise the behavior of this method is undefined.
	 *
	 * Calling this methods resets the peek-pointer.
	 * \param out The variable to write the value to.
	 */
	void receive(RelayCommand *out);

	/**
	 * Receive an uint8_t.
	 * \warning Calling this method is only safe when peek_uint8_t() returned \c true.
	 *          Otherwise the behavior of this method is undefined.
	 *
	 * Calling this methods resets the peek-pointer.
	 * \param out The variable to write the value to.
	 */
	void receive(uint8_t *out);

	/**
	 * Receive a string.
	 * \warning Calling this method is only safe when peek_string() returned \c true.
	 *          Otherwise the behavior of this method is undefined.
	 *
	 * Calling this methods resets the peek-pointer.
	 * \param out The variable to write the value to.
	 */
	void receive(std::string *out);

	/**
	 * Receive a RecvPacket.
	 * \warning Calling this method is only safe when peek_recvpacket() returned \c true.
	 *          Otherwise the behavior of this method is undefined.
	 *
	 * Calling this methods resets the peek-pointer.
	 * \param out The variable to write the value to.
	 */
	void receive(RecvPacket *out);

	/**
	 * Sends a relay command.
	 * Calling this on a closed connection will silently fail.
	 * \param data The data to send.
	 */
	void send(RelayCommand data);

	/**
	 * Sends an uint8_t.
	 * Calling this on a closed connection will silently fail.
	 * \param data The data to send.
	 */
	void send(uint8_t data);

	/**
	 * Sends a string.
	 * Calling this on a closed connection will silently fail.
	 * \param data The data to send.
	 */
	void send(const std::string& data);

	/**
	 * Sends a packet.
	 * Calling this on a closed connection will silently fail.
	 * \param data The data to send.
	 */
	void send(const SendPacket& data);

private:
	/**
	 * Tries to establish a connection to the given host.
	 * If the connection attempt failed, is_connected() will return \c false.
	 * \param host The host to connect to.
	 */
	explicit NetRelayConnection(const NetAddress& host);

	/**
	 * Reads data from network.
	 * \return \c False if an error occurred.
	 */
	bool try_network_receive();

	/// An io_service needed by boost.asio. Primarily needed for asynchronous operations.
	boost::asio::io_service io_service_;

	/// The socket that connects us to the relay.
	boost::asio::ip::tcp::socket socket_;

	/// Buffer for arriving data. We need to store it until we have enough to return the required type.
	std::deque<unsigned char> buffer_;

	/// The position of the next peek.
	size_t peek_pointer_;
};

#endif  // end of include guard: WL_NETWORK_NETRELAYCONNECTION_H
