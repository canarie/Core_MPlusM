//--------------------------------------------------------------------------------------------------
//
//  File:       mpm/M+MBaseAdapterService.cpp
//
//  Project:    M+M
//
//  Contains:   The class definition for the minimal functionality required for an M+M
//              adapter service.
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
//  Created:    2015-05-27
//
//--------------------------------------------------------------------------------------------------

#include "M+MBaseAdapterService.h"

#include <mpm/M+MRequests.h>
#include <mpm/M+MUtilities.h>

//#include <odl/ODEnableLogging.h>
#include <odl/ODLogging.h>

#if defined(__APPLE__)
# pragma clang diagnostic push
# pragma clang diagnostic ignored "-Wunknown-pragmas"
# pragma clang diagnostic ignored "-Wdocumentation-unknown-command"
#endif // defined(__APPLE__)
/*! @file
 @brief The class definition for the minimal functionality required for an M+M adapter service. */
#if defined(__APPLE__)
# pragma clang diagnostic pop
#endif // defined(__APPLE__)

#if defined(__APPLE__)
# pragma mark Namespace references
#endif // defined(__APPLE__)

using namespace MplusM;
using namespace MplusM::Common;

#if defined(__APPLE__)
# pragma mark Private structures, constants and variables
#endif // defined(__APPLE__)

#if defined(__APPLE__)
# pragma mark Global constants and variables
#endif // defined(__APPLE__)

#if defined(__APPLE__)
# pragma mark Local functions
#endif // defined(__APPLE__)

#if defined(__APPLE__)
# pragma mark Class methods
#endif // defined(__APPLE__)

#if defined(__APPLE__)
# pragma mark Constructors and Destructors
#endif // defined(__APPLE__)

BaseAdapterService::BaseAdapterService(const YarpString & launchPath,
                                       const int          argc,
                                       char * *           argv,
                                       const YarpString & tag,
                                       const bool         useMultipleHandlers,
                                       const YarpString & canonicalName,
                                       const YarpString & description,
                                       const YarpString & requestsDescription,
                                       const YarpString & serviceEndpointName,
                                       const YarpString & servicePortNumber) :
    inherited(kServiceKindAdapter, launchPath, argc, argv, tag, useMultipleHandlers, canonicalName,
              description, requestsDescription, serviceEndpointName, servicePortNumber)
{
    OD_LOG_ENTER(); //####
    OD_LOG_S4s("launchPath = ", launchPath, "tag = ", tag, "canonicalName = ", canonicalName, //####
               "description = ", description); //####
    OD_LOG_S3s("requestsDescription = ", requestsDescription, "serviceEndpointName = ", //####
               serviceEndpointName, "servicePortNumber = ", servicePortNumber); //####
    OD_LOG_LL1("argc = ", argc); //####
    OD_LOG_P1("argv = ", argv); //####
    OD_LOG_B1("useMultipleHandlers = ", useMultipleHandlers); //####
    OD_LOG_EXIT_P(this); //####
} // BaseAdapterService::BaseAdapterService

BaseAdapterService::~BaseAdapterService(void)
{
    OD_LOG_OBJENTER(); //####
    OD_LOG_OBJEXIT(); //####
} // BaseAdapterService::~BaseAdapterService

#if defined(__APPLE__)
# pragma mark Actions and Accessors
#endif // defined(__APPLE__)

bool BaseAdapterService::setUpClientStreams(void)
{
    OD_LOG_OBJENTER(); //####
    bool result = inherited::setUpClientStreams();
    
    if (result)
    {
        result = addClientStreamsFromDescriptions(_clientDescriptions);
    }
    OD_LOG_EXIT_B(result); //####
    return result;
} // BaseAdapterService::setUpClientStreams

bool BaseAdapterService::setUpInputStreams(void)
{
    OD_LOG_OBJENTER(); //####
    bool result = inherited::setUpInputStreams();
    
    if (result)
    {
        result = addInStreamsFromDescriptions(_inDescriptions);
    }
    OD_LOG_EXIT_B(result); //####
    return result;
} // BaseAdapterService::setUpInputStreams

bool BaseAdapterService::setUpOutputStreams(void)
{
    OD_LOG_OBJENTER(); //####
    bool result = inherited::setUpOutputStreams();
    
    if (result)
    {
        result = addOutStreamsFromDescriptions(_outDescriptions);
    }
    OD_LOG_EXIT_B(result); //####
    return result;
} // BaseAdapterService::setUpOutputStreams

bool BaseAdapterService::shutDownClientStreams(void)
{
    OD_LOG_OBJENTER(); //####
    bool result = inherited::shutDownClientStreams();
    
    OD_LOG_EXIT_B(result); //####
    return result;
} // BaseAdapterService::shutDownClientStreams

bool BaseAdapterService::shutDownInputStreams(void)
{
    OD_LOG_OBJENTER(); //####
    bool result = inherited::shutDownInputStreams();
    
    OD_LOG_EXIT_B(result); //####
    return result;
} // BaseAdapterService::shutDownInputStreams

bool BaseAdapterService::shutDownOutputStreams(void)
{
    OD_LOG_OBJENTER(); //####
    bool result = inherited::shutDownOutputStreams();
    
    OD_LOG_EXIT_B(result); //####
    return result;
} // BaseAdapterService::shutDownOutputStreams

#if defined(__APPLE__)
# pragma mark Global functions
#endif // defined(__APPLE__)
