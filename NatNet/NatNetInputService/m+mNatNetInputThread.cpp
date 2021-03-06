//--------------------------------------------------------------------------------------------------
//
//  File:       m+mNatNetInputThread.cpp
//
//  Project:    m+m
//
//  Contains:   The class definition for an output-generating thread for m+m.
//
//  Written by: Norman Jaffe
//
//  Copyright:  (c) 2015 by H Plus Technologies Ltd. and Simon Fraser University.
//
//              All rights reserved. Redistribution and use in source and binary forms, with or
//              without modification, are permitted provided that the following conditions are met:
//                * Redistributions of source code must retain the above copyright notice, this list
//                  of conditions and the following disclaimer.
//                * Redistributions in binary form must reproduce the above copyright notice, this
//                  list of conditions and the following disclaimer in the documentation and / or
//                  other materials provided with the distribution.
//                * Neither the name of the copyright holders nor the names of its contributors may
//                  be used to endorse or promote products derived from this software without
//                  specific prior written permission.
//
//              THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY
//              EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
//              OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT
//              SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
//              INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED
//              TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
//              BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
//              CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
//              ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
//              DAMAGE.
//
//  Created:    2015-04-13
//
//--------------------------------------------------------------------------------------------------

#include "m+mNatNetInputThread.hpp"

//#include <odlEnable.h>
#include <odlInclude.h>

#if defined(__APPLE__)
# pragma clang diagnostic push
# pragma clang diagnostic ignored "-Wunknown-pragmas"
# pragma clang diagnostic ignored "-Wdocumentation-unknown-command"
#endif // defined(__APPLE__)
/*! @file
 @brief The class definition for an output-generating thread for m+m. */
#if defined(__APPLE__)
# pragma clang diagnostic pop
#endif // defined(__APPLE__)

#if defined(__APPLE__)
# pragma mark Namespace references
#endif // defined(__APPLE__)

using namespace MplusM;
using namespace MplusM::Common;
using namespace MplusM::NatNet;

#if defined(__APPLE__)
# pragma mark Private structures, constants and variables
#endif // defined(__APPLE__)

//#define REPORT_NATNET_MESSAGES_ /* if defined, be verbose */

/*! @brief The connection mode for the client object. */
#define NATNET_CONNECTION_MODE_ 0 /* 0=multicast, 1=unicast */

#if defined(__APPLE__)
# pragma mark Global constants and variables
#endif // defined(__APPLE__)

#if defined(__APPLE__)
# pragma mark Local functions
#endif // defined(__APPLE__)

#if (! defined(MpM_BuildDummyServices))
/*! @brief Process a received frame of data.
@param[in] aFrame The data to be processed.
@param[in] userData The initially supplied data for this callback. */
static void __cdecl
dataReceived(sFrameOfMocapData * aFrame,
             void *              userData)
{
    ODL_ENTER(); //####
    ODL_P2("aFrame = ", aFrame, "userData = ", userData); //####
    NatNetInputThread * theThread = reinterpret_cast<NatNetInputThread *>(userData);

    if (aFrame && theThread)
    {
        int numSkeletons = aFrame->nSkeletons;
        int numRigid = aFrame->nRigidBodies;

        if (0 < (numRigid + numSkeletons))
        {
            yarp::os::Bottle   message;
            yarp::os::Bottle & rigidStuff = message.addList();

            // Put the 'rigid' data in first
            for (int ii = 0; numRigid > ii; ++ii)
            {
                sRigidBodyData & aBody = aFrame->RigidBodies[ii];

                if (aBody.params & 0x01)
                {
                    // Valid data
                    yarp::os::Property & bodyProps = rigidStuff.addDict();

                    bodyProps.put("id", aBody.ID);
                    bodyProps.put("x", aBody.x);
                    bodyProps.put("y", aBody.y);
                    bodyProps.put("z", aBody.z);
                    bodyProps.put("qx", aBody.qx);
                    bodyProps.put("qy", aBody.qy);
                    bodyProps.put("qz", aBody.qz);
                    bodyProps.put("qw", aBody.qw);
                }
            }
            yarp::os::Bottle & skelStuff = message.addList();

            // Now add the 'skeleton' data
            for (int ii = 0; numSkeletons > ii; ++ii)
            {
                yarp::os::Value      bonesStuff;
                sSkeletonData &      aSkel = aFrame->Skeletons[ii];
                yarp::os::Property & skelProps = skelStuff.addDict();
                int                  numBones = aSkel.nRigidBodies;
                yarp::os::Bottle *   bonesList = bonesStuff.asList();

                skelProps.put("id", aSkel.skeletonID);
                if (bonesList)
                {
                    for (int jj = 0; numBones > jj; ++jj)
                    {
                        sRigidBodyData &     aBone = aSkel.RigidBodyData[jj];
                        yarp::os::Property & boneProps = bonesList->addDict();

                        boneProps.put("id", aBone.ID);
                        boneProps.put("x", aBone.x);
                        boneProps.put("y", aBone.y);
                        boneProps.put("z", aBone.z);
                        boneProps.put("qx", aBone.qx);
                        boneProps.put("qy", aBone.qy);
                        boneProps.put("qz", aBone.qz);
                        boneProps.put("qw", aBone.qw);
                    }
                    skelProps.put("bones", bonesStuff);
                }
            }
            theThread->sendMessage(message);
        }
    }
    ODL_EXIT(); //####
} // dataReceived
#endif // ! defined(MpM_BuildDummyServices)

#if (! MAC_OR_LINUX_)
# pragma warning(push)
# pragma warning(disable: 4100)
#endif // ! MAC_OR_LINUX_
static void __cdecl
messageReceived(int    messageType,
                char * message)
{
#if ((! defined(ODL_ENABLE_LOGGING_)) || (! defined(REPORT_NATNET_MESSAGES_)))
# if MAC_OR_LINUX_
#  pragma unused(messageType,message)
# endif // MAC_OR_LINUX_
#endif // (! defined(ODL_ENABLE_LOGGING_)) || (! defined(REPORT_NATNET_MESSAGES_))
    ODL_ENTER(); //####
    ODL_LL1("messageType = ", messageType); //####
    ODL_S1("message = ", message); //####
#if defined(REPORT_NATNET_MESSAGES_)
    std::cerr << messageType << ": " << message << std::endl;
#endif // defined(REPORT_NATNET_MESSAGES_)
    ODL_EXIT(); //####
} // messageReceived
#if (! MAC_OR_LINUX_)
# pragma warning(pop)
#endif // ! MAC_OR_LINUX_

#if defined(__APPLE__)
# pragma mark Class methods
#endif // defined(__APPLE__)

#if defined(__APPLE__)
# pragma mark Constructors and Destructors
#endif // defined(__APPLE__)

NatNetInputThread::NatNetInputThread(Common::GeneralChannel * outChannel,
                                     const YarpString &       name,
                                     const int                commandPort,
                                     const int                dataPort) :
    inherited(), _outChannel(outChannel), _address(name), _commandPort(commandPort),
    _dataPort(dataPort)
#if (! defined(MpM_BuildDummyServices))
    , _client(NULL)
#endif // ! defined(MpM_BuildDummyServices)
{
    ODL_ENTER(); //####
    ODL_P1("outChannel = ", outChannel); //####
    ODL_S1s("name = ", name); //####
    ODL_LL2("commandPort = ", commandPort, "dataPort = ", dataPort); //####
#if defined(MpM_BuildDummyServices)
    strncpy(_clientIPAddress, "", sizeof(_clientIPAddress) - 1);
    strncpy(_serverIPAddress, "", sizeof(_serverIPAddress) - 1);
#else // ! defined(MpM_BuildDummyServices)
    strcpy_s(_clientIPAddress, sizeof(_clientIPAddress) - 1, "");
    strcpy_s(_serverIPAddress, sizeof(_serverIPAddress) - 1, "");
#endif // ! defined(MpM_BuildDummyServices)
    ODL_EXIT_P(this); //####
} // NatNetInputThread::NatNetInputThread

NatNetInputThread::~NatNetInputThread(void)
{
    ODL_OBJENTER(); //####
#if (! defined(MpM_BuildDummyServices))
    if (_client)
    {
        _client->Uninitialize();
        delete _client;
    }
#endif // ! defined(MpM_BuildDummyServices)
    ODL_OBJEXIT(); //####
} // NatNetInputThread::~NatNetInputThread

#if defined(__APPLE__)
# pragma mark Actions and Accessors
#endif // defined(__APPLE__)

void
NatNetInputThread::clearOutputChannel(void)
{
    ODL_OBJENTER(); //####
    _outChannel = NULL;
    ODL_OBJEXIT(); //####
} // NatNetInputThread::clearOutputChannel

void
NatNetInputThread::run(void)
{
    ODL_OBJENTER(); //####
    try
    {
        for ( ; ! isStopping(); )
        {
            ConsumeSomeTime();
        }
#if (! defined(MpM_BuildDummyServices))
        if (_client)
        {
            _client->SetDataCallback(NULL, NULL);
        }
#endif // ! defined(MpM_BuildDummyServices)
    }
    catch (...)
    {
        ODL_LOG("Exception caught"); //####
        throw;
    }
    ODL_OBJEXIT(); //####
} // NatNetInputThread::run

void
NatNetInputThread::sendMessage(yarp::os::Bottle & message)
{
    ODL_OBJENTER(); //####
    ODL_P1("message = ", &message); //####

    if (_outChannel)
    {
        if (! _outChannel->write(message))
        {
            ODL_LOG("(! _outChannel->write(message))"); //####
#if defined(MpM_StallOnSendProblem)
            Stall();
#endif // defined(MpM_StallOnSendProblem)
        }
    }
    ODL_OBJEXIT(); //####
} // NatNetInputThread::sendMessage

bool
NatNetInputThread::threadInit(void)
{
    ODL_OBJENTER(); //####
    bool result = true;

    try
    {
#if (! defined(MpM_BuildDummyServices))
        _client = new NatNetClient(NATNET_CONNECTION_MODE_);
        ODL_P1("_client <- ", _client); //####
        if (_client)
        {
            const char * theServerAddress = _address.c_str();

            if (theServerAddress && (IPADDRESS_BUFFER_SIZE > (strlen(theServerAddress) + 5)))
            {
                strcpy_s(_serverIPAddress, sizeof(_serverIPAddress) - 1, theServerAddress);
            }
            else
            {
                strcpy_s(_serverIPAddress, sizeof(_serverIPAddress) - 1, "");
            }
            _client->SetMessageCallback(messageReceived);
            _client->SetVerbosityLevel(Verbosity_Debug);
            _client->SetDataCallback(dataReceived, this);
            int retCode = _client->Initialize(_clientIPAddress, _serverIPAddress, _commandPort,
                                              _dataPort);

            if (ErrorCode_OK == retCode)
            {
                sServerDescription theServer;

                memset(&theServer, 0, sizeof(theServer));
                _client->GetServerDescription(&theServer);
                if (! theServer.HostPresent)
                {
                    ODL_LOG("(! theServer.HostPresent)"); //####
                    std::cerr << "Natural Point NatNet service host not present." << std::endl;
                    result = false;
                }
            }
            else
            {
                ODL_LOG("(_client->Initialize(NULL, const_cast<char *>(_address.c_str()), " //####
                        "_commandPort, _dataPort))"); //####
                std::cerr << "Initialization problem with Natural Point NatNet service: ";
                switch (retCode)
                {
                    case ErrorCode_External :
                        std::cerr << "external issue.";
                        break;

                    case ErrorCode_Internal :
                        std::cerr << "internal issue.";
                        break;

                    case ErrorCode_Network :
                        std::cerr << "network issue.";
                        break;

                    default :
                        std::cerr << "unknown issue.";
                        break;

                }
                std::cerr << std::endl;
                result = false;
            }
        }
        else
        {
            result = false;
        }
#endif // ! defined(MpM_BuildDummyServices)
    }
    catch (...)
    {
        ODL_LOG("Exception caught"); //####
        result = false;
    }
    ODL_OBJEXIT_B(result); //####
    return result;
} // NatNetInputThread::threadInit

#if defined(__APPLE__)
# pragma mark Global functions
#endif // defined(__APPLE__)
