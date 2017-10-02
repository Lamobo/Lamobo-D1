/**********
This library is free software; you can redistribute it and/or modify it under
the terms of the GNU Lesser General Public License as published by the
Free Software Foundation; either version 2.1 of the License, or (at your
option) any later version. (See <http://www.gnu.org/copyleft/lesser.html>.)

This library is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for
more details.

You should have received a copy of the GNU Lesser General Public License
along with this library; if not, write to the Free Software Foundation, Inc.,
51 Franklin Street, Fifth Floor, Boston, MA 02110-1301  USA
**********/
// "liveMedia"
// Copyright (c) 1996-2013 Live Networks, Inc.  All rights reserved.
// A 'ServerMediaSubsession' object that creates new, unicast, "RTPSink"s
// on demand, from an AAC audio file in ADTS format
// C++ header

#ifndef _AKIPC_AACAUDIO_ONDEMAND_MEDIA_SUBSESSION_HH
#define _AKIPC_AACAUDIO_ONDEMAND_MEDIA_SUBSESSION_HH

#ifndef _ON_DEMAND_SERVER_MEDIA_SUBSESSION_HH
#include "OnDemandServerMediaSubsession.hh"
#endif

#include "AKIPCAACAudioFramedSource.hh"

class AKIPCAACAudioOnDemandMediaSubsession: public OnDemandServerMediaSubsession{
public:
  static AKIPCAACAudioOnDemandMediaSubsession*
  createNew(UsageEnvironment& env, /*char const* fileName, */Boolean reuseFirstSource, GetAACFrameFunc func, int vsIndex);

protected:
  AKIPCAACAudioOnDemandMediaSubsession(UsageEnvironment& env,
				      /*char const* fileName, */Boolean reuseFirstSource, GetAACFrameFunc func, int index);
      // called only by createNew();
  virtual ~AKIPCAACAudioOnDemandMediaSubsession();

protected: // redefined virtual functions
  virtual FramedSource* createNewStreamSource(unsigned clientSessionId,
					      unsigned& estBitrate);
  virtual RTPSink* createNewRTPSink(Groupsock* rtpGroupsock,
                                    unsigned char rtpPayloadTypeIfDynamic,
				    FramedSource* inputSource);
private:
	GetAACFrameFunc fGetAACFrameFunc;
	int findex;
};

#endif
