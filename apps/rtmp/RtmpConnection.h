/*
 * Copyright (C) 2011 Raphael Coeffic
 *
 * This file is part of SEMS, a free SIP media server.
 *
 * SEMS is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version. This program is released under
 * the GPL with the additional exemption that compiling, linking,
 * and/or using OpenSSL is allowed.
 *
 * For a license to use the SEMS software under conditions
 * other than those described here, or to purchase support for this
 * software, please contact iptel.org by e-mail at the following addresses:
 *    info@iptel.org
 *
 * SEMS is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License 
 * along with this program; if not, write to the Free Software 
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#ifndef _RtmpConnection_h_
#define _RtmpConnection_h_

#include "AmThread.h"
#include "librtmp/rtmp.h"

#include "Rtmp.h"

#include <string>
using std::string;

// call states for the RTMP client
#define RTMP_CALL_NOT_CONNECTED 0
#define RTMP_CALL_IN_PROGRESS   1
#define RTMP_CALL_CONNECTED     2
#define RTMP_CALL_DISCONNECTING 3

// request the client to connect the streams
#define RTMP_CALL_CONNECT_STREAMS 4


class RtmpSession;
class RtmpSender;
class RtmpAudio;

class RtmpConnection
  : public AmThread
{
  enum MediaFlags {
    AudioSend=0x01,
    AudioRecv=0x02,
    VideoSend=0x04,
    VideoRecv=0x08
  };

  RTMP     rtmp;

  // Transaction number for server->client requests
  int      out_txn;

  // Previous stream ID sent to the client for createStream
  int      prev_stream_id;

  // Stream ID with play() invoke
  unsigned int play_stream_id;

  // Stream ID with publish() invoke
  unsigned int publish_stream_id;
  
  // Owned by the connection
  // used also by the session
  //
  // Note: do not destroy before
  //       the session released its ptrs.
  RtmpSender*  sender;

  // Owned by the session
  RtmpSession* session;
  AmMutex      m_session;

  // Identity of the connection
  string       ident;

  // Is the connection registered in RtmpFactory?
  bool         registered;

  const RtmpConfig& rtmp_cfg;

public:
  RtmpConnection(int fd);
  ~RtmpConnection();

  void setSessionPtr(RtmpSession* s);
  RtmpSender* getSenderPtr() { return sender; }

  int SendPlayStart();
  int SendPlayStop();
  int SendStreamBegin();
  int SendStreamEOF();

  int SendCallStatus(int status);
  int NotifyIncomingCall(const string& uri);

protected:
  void run();
  void on_stop();

private:
  RtmpSession* startSession(const char* uri);
  void disconnectSession();
  void detachSession();

  void stopStream(unsigned int stream_id);

  int processPacket(RTMPPacket *packet);
  int invoke(RTMPPacket *packet, unsigned int offset);

  void rxAudio(RTMPPacket *packet);
  void rxVideo(RTMPPacket *packet) {/*NYI*/}

  void HandleCtrl(const RTMPPacket *packet);

  int SendCtrl(short nType, unsigned int nObject, unsigned int nTime);
  int SendResultNumber(double txn, double ID);
  int SendConnectResult(double txn);
  int SendRegisterResult(double txn, const char* str);
  int SendErrorResult(double txn, const char* str);
  int SendPause(int DoPause, int iTime);
  int SendChangeChunkSize();
};

#endif