//--------------------------------------------------------------------------------------
//
//  File:       M+MBaseInputService.cpp
//
//  Project:    M+M
//
//  Contains:   The class definition for the minimal functionality required for an M+M
//              input service.
//
//  Written by: Norman Jaffe
//
//  Copyright:  (c) 2014 by HPlus Technologies Ltd. and Simon Fraser University.
//
//              All rights reserved. Redistribution and use in source and binary forms,
//              with or without modification, are permitted provided that the following
//              conditions are met:
//                * Redistributions of source code must retain the above copyright
//                  notice, this list of conditions and the following disclaimer.
//                * Redistributions in binary form must reproduce the above copyright
//                  notice, this list of conditions and the following disclaimer in the
//                  documentation and/or other materials provided with the
//                  distribution.
//                * Neither the name of the copyright holders nor the names of its
//                  contributors may be used to endorse or promote products derived
//                  from this software without specific prior written permission.
//
//              THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
//              "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
//              LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
//              PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
//              OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
//              SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
//              LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
//              DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
//              THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
//              (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
//              OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
//  Created:    2014-06-23
//
//--------------------------------------------------------------------------------------

#include "M+MBaseInputService.h"
#include "M+MRequests.h"

//#include "ODEnableLogging.h"
#include "ODLogging.h"

#if defined(__APPLE__)
# pragma clang diagnostic push
# pragma clang diagnostic ignored "-Wdocumentation-unknown-command"
#endif // defined(__APPLE__)
/*! @file
 
 @brief The class definition for the minimal functionality required for an M+M input service. */
#if defined(__APPLE__)
# pragma clang diagnostic pop
#endif // defined(__APPLE__)

using namespace MplusM;
using namespace MplusM::Common;

#if defined(__APPLE__)
# pragma mark Private structures, constants and variables
#endif // defined(__APPLE__)

#if defined(__APPLE__)
# pragma mark Local functions
#endif // defined(__APPLE__)

#if defined(__APPLE__)
# pragma mark Class methods
#endif // defined(__APPLE__)

#if defined(__APPLE__)
# pragma mark Constructors and destructors
#endif // defined(__APPLE__)

BaseInputService::BaseInputService(const char *                  launchPath,
                                   const bool                    useMultipleHandlers,
                                   const yarp::os::ConstString & canonicalName,
                                   const yarp::os::ConstString & description,
                                   const yarp::os::ConstString & requestsDescription,
                                   const yarp::os::ConstString & serviceEndpointName,
                                   const yarp::os::ConstString & serviceHostName,
                                   const yarp::os::ConstString & servicePortNumber) :
            inherited(kServiceKindInput, launchPath, useMultipleHandlers, canonicalName, description,
                      requestsDescription, serviceEndpointName, serviceHostName, servicePortNumber)
{
    OD_LOG_ENTER();//####
    OD_LOG_S1("launchPath = ", launchPath);//####
    OD_LOG_B1("useMultipleHandlers = ", useMultipleHandlers);//####
    OD_LOG_S4("canonicalName = ", canonicalName.c_str(), "description = ", description.c_str(),//####
              "requestsDescription = ", requestsDescription.c_str(), "serviceEndpointName = ",//####
              serviceEndpointName.c_str());//####
    OD_LOG_S2("serviceHostName = ", serviceHostName.c_str(), "servicePortNumber = ", servicePortNumber.c_str());//####
//    attachRequestHandlers();
    OD_LOG_EXIT_P(this);//####
} // BaseInputService::BaseInputService

BaseInputService::BaseInputService(const bool                    useMultipleHandlers,
                                   const yarp::os::ConstString & canonicalName,
                                   const yarp::os::ConstString & description,
                                   const yarp::os::ConstString & requestsDescription,
                                   const int                     argc,
                                   char * *                      argv) :
        inherited(kServiceKindInput, useMultipleHandlers, canonicalName, description, requestsDescription, argc, argv)
{
    OD_LOG_ENTER();//####
    OD_LOG_B1("useMultipleHandlers = ", useMultipleHandlers);//####
    OD_LOG_S3("canonicalName = ", canonicalName.c_str(), "description = ", description.c_str(),//####
              "requestsDescription = ", requestsDescription.c_str());//####
//    attachRequestHandlers();
    OD_LOG_EXIT_P(this);//####
} // BaseInputService::BaseInputService

BaseInputService::~BaseInputService(void)
{
    OD_LOG_OBJENTER();//####
//    detachRequestHandlers();
    OD_LOG_OBJEXIT();//####
} // BaseInputService::~BaseInputService

#if defined(__APPLE__)
# pragma mark Actions
#endif // defined(__APPLE__)

bool BaseInputService::setUpInputStreams(void)
{
    OD_LOG_OBJENTER();//####
    bool result = true; // always true, as there are no input streams
    
    OD_LOG_EXIT_B(result);//####
    return result;
} // BaseInputService::setUpInputStreams

bool BaseInputService::setUpOutputStreams(void)
{
    OD_LOG_OBJENTER();//####
    bool result = true;
    
    OD_LOG_EXIT_B(result);//####
    return result;
} // BaseInputService::setUpOutputStreams

bool BaseInputService::shutDownInputStreams(void)
{
    OD_LOG_OBJENTER();//####
    bool result = true; // always true, as there are no input streams
    
    OD_LOG_EXIT_B(result);//####
    return result;
} // BaseInputService::shutDownInputStreams

bool BaseInputService::shutDownOutputStreams(void)
{
    OD_LOG_OBJENTER();//####
    bool result = true;
    
    OD_LOG_EXIT_B(result);//####
    return result;
} // BaseInputService::shutDownOutputStreams

#if defined(__APPLE__)
# pragma mark Accessors
#endif // defined(__APPLE__)

#if defined(__APPLE__)
# pragma mark Global functions
#endif // defined(__APPLE__)
