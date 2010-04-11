/* <!-- copyright */
/*
 * aria2 - The high speed download utility
 *
 * Copyright (C) 2006 Tatsuhiro Tsujikawa
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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 *
 * In addition, as a special exception, the copyright holders give
 * permission to link the code of portions of this program with the
 * OpenSSL library under certain conditions as described in each
 * individual source file, and distribute linked combinations
 * including the two.
 * You must obey the GNU General Public License in all respects
 * for all of the code used other than OpenSSL.  If you modify
 * file(s) with this exception, you may extend this exception to your
 * version of the file(s), but you are not obligated to do so.  If you
 * do not wish to do so, delete this exception statement from your
 * version.  If you delete this exception statement from all source
 * files in the program, then also delete it here.
 */
/* copyright --> */
#include "Peer.h"

#include <cstring>
#include <cassert>

#include "util.h"
#include "a2functional.h"
#include "PeerSessionResource.h"
#include "BtMessageDispatcher.h"
#include "wallclock.h"

namespace aria2 {

#define BAD_CONDITION_INTERVAL 10

Peer::Peer(std::string ipaddr, uint16_t port, bool incoming):
  ipaddr(ipaddr),
  port(port),
  _firstContactTime(global::wallclock),
  _badConditionStartTime(0),
  _seeder(false),
  _res(0),
  _incoming(incoming),
  _localPeer(false)
{
  memset(_peerId, 0, PEER_ID_LENGTH);
  resetStatus();
  id = ipaddr;
  strappend(id, A2STR::COLON_C, util::uitos(port));
}

Peer::~Peer()
{
  releaseSessionResource();
}

void Peer::usedBy(cuid_t cuid)
{
  _cuid = cuid;
}

void Peer::allocateSessionResource(size_t pieceLength, uint64_t totalLength)
{
  delete _res;
  _res = new PeerSessionResource(pieceLength, totalLength);
  _res->getPeerStat().downloadStart();
}

void Peer::releaseSessionResource()
{
  delete _res;
  _res = 0;
}

void Peer::setPeerId(const unsigned char* peerId)
{
  memcpy(_peerId, peerId, PEER_ID_LENGTH);
}

void Peer::resetStatus() {
  _cuid = 0;
}

bool Peer::amChoking() const
{
  assert(_res);
  return _res->amChoking();
}

void Peer::amChoking(bool b) const
{
  assert(_res);
  _res->amChoking(b);
}

// localhost is interested in this peer
bool Peer::amInterested() const
{
  assert(_res);
  return _res->amInterested();
}

void Peer::amInterested(bool b) const
{
  assert(_res);
  _res->amInterested(b);
}

// this peer is choking localhost
bool Peer::peerChoking() const
{
  assert(_res);
  return _res->peerChoking();
}

void Peer::peerChoking(bool b) const
{
  assert(_res);
  _res->peerChoking(b);
}

// this peer is interested in localhost
bool Peer::peerInterested() const
{
  assert(_res);
  return _res->peerInterested();
}

void Peer::peerInterested(bool b)
{
  assert(_res);
  _res->peerInterested(b);
}
  
// this peer should be choked
bool Peer::chokingRequired() const
{
  assert(_res);
  return _res->chokingRequired();
}

void Peer::chokingRequired(bool b)
{
  assert(_res);
  _res->chokingRequired(b);
}

// this peer is eligible for unchoking optionally.
bool Peer::optUnchoking() const
{
  assert(_res);
  return _res->optUnchoking();
}

void Peer::optUnchoking(bool b)
{
  assert(_res);
  _res->optUnchoking(b);
}

// this peer is snubbing.
bool Peer::snubbing() const
{
  assert(_res);
  return _res->snubbing();
}

void Peer::snubbing(bool b)
{
  assert(_res);
  _res->snubbing(b);
}

void Peer::updateUploadLength(size_t bytes)
{
  assert(_res);
  _res->updateUploadLength(bytes);
}

void Peer::updateDownloadLength(size_t bytes)
{
  assert(_res);
  _res->updateDownloadLength(bytes);
}

void Peer::updateSeeder()
{
  assert(_res);
  if(_res->hasAllPieces()) {
    _seeder = true;
  }  
}

void Peer::updateBitfield(size_t index, int operation) {
  assert(_res);
  _res->updateBitfield(index, operation);
  updateSeeder();
}

unsigned int Peer::calculateUploadSpeed()
{
  assert(_res);
  return _res->getPeerStat().calculateUploadSpeed();
}

unsigned int Peer::calculateDownloadSpeed()
{
  assert(_res);
  return _res->getPeerStat().calculateDownloadSpeed();
}

uint64_t Peer::getSessionUploadLength() const
{
  assert(_res);
  return _res->uploadLength();
}

uint64_t Peer::getSessionDownloadLength() const
{
  assert(_res);
  return _res->downloadLength();
}

void Peer::setBitfield(const unsigned char* bitfield, size_t bitfieldLength)
{
  assert(_res);
  _res->setBitfield(bitfield, bitfieldLength);
  updateSeeder();
}

const unsigned char* Peer::getBitfield() const
{
  assert(_res);
  return _res->getBitfield();
}

size_t Peer::getBitfieldLength() const
{
  assert(_res);
  return _res->getBitfieldLength();
}

bool Peer::shouldBeChoking() const {
  assert(_res);
  return _res->shouldBeChoking();
}

bool Peer::hasPiece(size_t index) const {
  assert(_res);
  return _res->hasPiece(index);
}

void Peer::setFastExtensionEnabled(bool enabled)
{
  assert(_res);
  return _res->fastExtensionEnabled(enabled);
}

bool Peer::isFastExtensionEnabled() const
{
  assert(_res);
  return _res->fastExtensionEnabled();
}

size_t Peer::countPeerAllowedIndexSet() const
{
  assert(_res);
  return _res->peerAllowedIndexSet().size();
}

const std::vector<size_t>& Peer::getPeerAllowedIndexSet() const
{
  assert(_res);
  return _res->peerAllowedIndexSet();
}

bool Peer::isInPeerAllowedIndexSet(size_t index) const
{
  assert(_res);
  return _res->peerAllowedIndexSetContains(index);
}

void Peer::addPeerAllowedIndex(size_t index)
{
  assert(_res);
  _res->addPeerAllowedIndex(index);
}

bool Peer::isInAmAllowedIndexSet(size_t index) const
{
  assert(_res);
  return _res->amAllowedIndexSetContains(index);
}

void Peer::addAmAllowedIndex(size_t index)
{
  assert(_res);
  _res->addAmAllowedIndex(index);
}

void Peer::setAllBitfield() {
  assert(_res);
  _res->markSeeder();
  _seeder = true;
}

void Peer::startBadCondition()
{
  _badConditionStartTime = global::wallclock;
}

bool Peer::isGood() const
{
  return _badConditionStartTime.
    difference(global::wallclock) >= BAD_CONDITION_INTERVAL;
}

uint8_t Peer::getExtensionMessageID(const std::string& name) const
{
  assert(_res);
  return _res->getExtensionMessageID(name);
}

std::string Peer::getExtensionName(uint8_t id) const
{
  assert(_res);
  return _res->getExtensionName(id);
}

void Peer::setExtension(const std::string& name, uint8_t id)
{
  assert(_res);
  _res->addExtension(name, id);
}

void Peer::setExtendedMessagingEnabled(bool enabled)
{
  assert(_res);
  _res->extendedMessagingEnabled(enabled);
}

bool Peer::isExtendedMessagingEnabled() const
{
  assert(_res);
  return _res->extendedMessagingEnabled();
}

void Peer::setDHTEnabled(bool enabled)
{
  assert(_res);
  _res->dhtEnabled(enabled);
}

bool Peer::isDHTEnabled() const
{
  assert(_res);
  return _res->dhtEnabled();
}

const Timer& Peer::getLastDownloadUpdate() const
{
  assert(_res);
  return _res->getLastDownloadUpdate();
}

const Timer& Peer::getLastAmUnchoking() const
{
  assert(_res);
  return _res->getLastAmUnchoking();
}

uint64_t Peer::getCompletedLength() const
{
  assert(_res);
  return _res->getCompletedLength();
}

void Peer::setIncomingPeer(bool incoming)
{
  _incoming = incoming;
}

void Peer::setFirstContactTime(const Timer& time)
{
  _firstContactTime = time;
}

void Peer::setBtMessageDispatcher(const WeakHandle<BtMessageDispatcher>& dpt)
{
  assert(_res);
  _res->setBtMessageDispatcher(dpt);
}

size_t Peer::countOutstandingUpload() const
{
  assert(_res);
  return _res->countOutstandingUpload();
}

} // namespace aria2
