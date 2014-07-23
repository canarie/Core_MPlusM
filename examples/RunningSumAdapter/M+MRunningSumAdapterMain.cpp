//--------------------------------------------------------------------------------------------------
//
//  File:       RunningSumAdapterMain.cpp
//
//  Project:    M+M
//
//  Contains:   The main application for the running sum adapter.
//
//  Written by: Norman Jaffe
//
//  Copyright:  (c) 2014 by HPlus Technologies Ltd. and Simon Fraser University.
//
//              All rights reserved. Redistribution and use in source and binary forms, with or
//              without modification, are permitted provided that the following conditions are met:
//                * Redistributions of source code must retain the above copyright notice, this list
//                  of conditions and the following disclaimer.
//                * Redistributions in binary form must reproduce the above copyright notice, this
//                  list of conditions and the following disclaimer in the documentation and/or
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
//  Created:    2014-03-18
//
//--------------------------------------------------------------------------------------------------

#include "M+MAdapterChannel.h"
#include "M+MChannelStatusReporter.h"
#include "M+MRunningSumAdapterData.h"
#include "M+MRunningSumClient.h"
#include "M+MRunningSumControlInputHandler.h"
#include "M+MRunningSumDataInputHandler.h"

//#include "ODEnableLogging.h"
#include "ODLogging.h"

#if defined(__APPLE__)
# pragma clang diagnostic push
# pragma clang diagnostic ignored "-Wc++11-extensions"
# pragma clang diagnostic ignored "-Wdocumentation"
# pragma clang diagnostic ignored "-Wdocumentation-unknown-command"
# pragma clang diagnostic ignored "-Wpadded"
# pragma clang diagnostic ignored "-Wshadow"
# pragma clang diagnostic ignored "-Wunused-parameter"
# pragma clang diagnostic ignored "-Wweak-vtables"
#endif // defined(__APPLE__)
#include <yarp/os/impl/Logger.h>
#if defined(__APPLE__)
# pragma clang diagnostic pop
#endif // defined(__APPLE__)

#if defined(__APPLE__)
# pragma clang diagnostic push
# pragma clang diagnostic ignored "-Wdocumentation-unknown-command"
#endif // defined(__APPLE__)
/*! @file
 
 @brief The main application for the running sum adapter. */

/*! @dir RunningSumAdapter
 @brief The set of files that implement the running sum adapter. */
#if defined(__APPLE__)
# pragma clang diagnostic pop
#endif // defined(__APPLE__)

using namespace MplusM;
using namespace MplusM::Common;
using namespace MplusM::Example;

#if defined(__APPLE__)
# pragma mark Private structures, constants and variables
#endif // defined(__APPLE__)

#if defined(__APPLE__)
# pragma mark Local functions
#endif // defined(__APPLE__)

#if defined(__APPLE__)
# pragma mark Global functions
#endif // defined(__APPLE__)

/*! @brief The entry point for creating the running sum adapter.
 
 The program creates three YARP ports: an output port and two input ports - a control port to
 receive commands and a data port to receive a sequence of numbers to be added. Commands are
 case-sensitive and will result in requests being sent to the service. The commands are:
 
 quit Ask the service to stop calculating the running sum and exit from the program.
 
 reset Ask the service to reset its running sum.
 
 start Ask the service to start calculating the running sum.
 
 stop Ask the service to stop calculating the running sum.
 The first, optional, argument is the port name to be used for the control port, the second,
 optional, argument is the name to be used for the data port and the third, optional, argument is
 the name to be used for the output port. If the first argument is missing, the control port will be
 named ADAPTER_PORT_NAME_BASE+control/runningsum, if the second argument is missing, the data port
 will be named ADAPTER_PORT_NAME_BASE+data/randomnumber and if the third argument is missing the
 output port will be named ADAPTER_PORT_NAME_BASE+output/runningsum.
 @param argc The number of arguments in 'argv'.
 @param argv The arguments to be used with the example client.
 @returns @c 0 on a successful test and @c 1 on failure. */
int main(int     argc,
         char ** argv)
{
    OD_LOG_INIT(*argv, kODLoggingOptionIncludeProcessID | kODLoggingOptionIncludeThreadID | //####
                kODLoggingOptionEnableThreadSupport | kODLoggingOptionWriteToStderr); //####
    OD_LOG_ENTER(); //####
    MplusM::Common::SetUpLogger(*argv);
    try
    {
#if CheckNetworkWorks_
        if (yarp::os::Network::checkNetwork())
#endif // CheckNetworkWorks_
        {
            yarp::os::Network yarp; // This is necessary to establish any connection to the YARP
                                    // infrastructure
            
            MplusM::Common::Initialize(*argv);
            RunningSumClient * stuff = new RunningSumClient;
            
            if (stuff)
            {
                MplusM::StartRunning();
                MplusM::Common::SetSignalHandlers(MplusM::SignalRunningStop);
                if (stuff->findService("Name RunningSum"))
                {
#if defined(MpM_ReportOnConnections)
                    stuff->setReporter(ChannelStatusReporter::gReporter, true);
#endif // defined(MpM_ReportOnConnections)
                    if (stuff->connectToService())
                    {
                        MplusM::Common::AdapterChannel * controlChannel =
                                                        new MplusM::Common::AdapterChannel(false);
                        MplusM::Common::AdapterChannel * dataChannel =
                                                        new MplusM::Common::AdapterChannel(false);
                        MplusM::Common::AdapterChannel * outputChannel =
                                                        new MplusM::Common::AdapterChannel(true);
                        RunningSumAdapterData            sharedData(stuff, outputChannel);
                        RunningSumControlInputHandler *  controlHandler =
                                                    new RunningSumControlInputHandler(sharedData);
                        RunningSumDataInputHandler *     dataHandler =
                                                        new RunningSumDataInputHandler(sharedData);
                        
                        if (controlChannel && dataChannel && outputChannel && controlHandler &&
                            dataHandler)
                        {
                            yarp::os::ConstString controlName(T_(ADAPTER_PORT_NAME_BASE
                                                                 "control/runningsum"));
                            yarp::os::ConstString dataName(T_(ADAPTER_PORT_NAME_BASE
                                                              "data/runningsum"));
                            yarp::os::ConstString outputName(T_(ADAPTER_PORT_NAME_BASE
                                                                "output/runningsum"));
                            
                            if (argc > 1)
                            {
                                controlName = argv[1];
                                if (argc > 2)
                                {
                                    dataName = argv[2];
                                    if (argc > 3)
                                    {
                                        outputName = argv[3];
                                    }
                                }
                            }
#if defined(MpM_ReportOnConnections)
                            controlChannel->setReporter(ChannelStatusReporter::gReporter);
                            controlChannel->getReport(ChannelStatusReporter::gReporter);
                            dataChannel->setReporter(ChannelStatusReporter::gReporter);
                            dataChannel->getReport(ChannelStatusReporter::gReporter);
                            outputChannel->setReporter(ChannelStatusReporter::gReporter);
                            outputChannel->getReport(ChannelStatusReporter::gReporter);
#endif // defined(MpM_ReportOnConnections)
                            if (controlChannel->openWithRetries(controlName, STANDARD_WAIT_TIME) &&
                                dataChannel->openWithRetries(dataName, STANDARD_WAIT_TIME) &&
                                outputChannel->openWithRetries(outputName, STANDARD_WAIT_TIME))
                            {
                                stuff->addAssociatedChannel(controlChannel);
                                stuff->addAssociatedChannel(dataChannel);
                                stuff->addAssociatedChannel(outputChannel);
                                sharedData.activate();
                                controlChannel->setReader(*controlHandler);
                                dataChannel->setReader(*dataHandler);
                                for ( ; MplusM::IsRunning() && sharedData.isActive(); )
                                {
#if defined(MpM_MainDoesDelayNotYield)
                                    yarp::os::Time::delay(ONE_SECOND_DELAY);
#else // ! defined(MpM_MainDoesDelayNotYield)
                                    yarp::os::Time::yield();
#endif // ! defined(MpM_MainDoesDelayNotYield)
                                    if (! MplusM::IsRunning())
                                    {
                                        sharedData.deactivate();
                                    }
                                }
                                stuff->removeAssociatedChannels();
                            }
                            else
                            {
                                OD_LOG("! (controlChannel->openWithRetries(controlName, " //####
                                       "STANDARD_WAIT_TIME) && " //####
                                       "dataChannel->openWithRetries(dataName, " //####
                                       "STANDARD_WAIT_TIME) && " //####
                                       "outputChannel->openWithRetries(outputName, " //####
                                       "STANDARD_WAIT_TIME))"); //####
                                MplusM::Common::GetLogger().fail("Problem opening a channel.");
                            }
#if defined(MpM_DoExplicitClose)
                            controlChannel->close();
                            dataChannel->close();
                            outputChannel->close();
#endif // defined(MpM_DoExplicitClose)
                        }
                        else
                        {
                            OD_LOG("! (controlChannel && dataChannel && outputChannel && " //####
                                   "controlHandler && dataHandler)"); //####
                            MplusM::Common::GetLogger().fail("Problem creating a channel.");
                        }
                        MplusM::Common::AdapterChannel::RelinquishChannel(controlChannel);
                        MplusM::Common::AdapterChannel::RelinquishChannel(dataChannel);
                        MplusM::Common::AdapterChannel::RelinquishChannel(outputChannel);
                        if (! stuff->disconnectFromService())
                        {
                            OD_LOG("(! stuff->disconnectFromService())"); //####
                            MplusM::Common::GetLogger().fail("Problem disconnecting from the "
                                                             "service.");
                        }
                    }
                    else
                    {
                        OD_LOG("! (stuff->connectToService())"); //####
                        MplusM::Common::GetLogger().fail("Problem connecting to the service.");
                    }
                }
                else
                {
                    OD_LOG("! (stuff->findService(\"Name RunningSum\"))"); //####
                    MplusM::Common::GetLogger().fail("Problem locating the service.");
                }
                delete stuff;
            }
            else
            {
                OD_LOG("! (stuff)"); //####
            }
        }
#if CheckNetworkWorks_
        else
        {
            OD_LOG("! (yarp::os::Network::checkNetwork())"); //####
            MplusM::Common::GetLogger().fail("YARP network not running.");
        }
#endif // CheckNetworkWorks_
    }
    catch (...)
    {
        OD_LOG("Exception caught"); //####
    }
    yarp::os::Network::fini();
    OD_LOG_EXIT_L(0); //####
    return 0;
} // main
