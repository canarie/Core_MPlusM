//--------------------------------------------------------------------------------------------------
//
//  File:       m+mKinectV2InputService.h
//
//  Project:    m+m
//
//  Contains:   The class declaration for the Kinect V2 input service.
//
//  Written by: Norman Jaffe
//
//  Copyright:  (c) 2014 by H Plus Technologies Ltd. and Simon Fraser University.
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
//  Created:    2014-09-15
//
//--------------------------------------------------------------------------------------------------

#if (! defined(MpMKinectV2InputService_H_))
# define MpMKinectV2InputService_H_ /* Header guard */

# include <m+m/m+mBaseInputService.h>

# if defined(__APPLE__)
#  pragma clang diagnostic push
#  pragma clang diagnostic ignored "-Wunknown-pragmas"
#  pragma clang diagnostic ignored "-Wdocumentation-unknown-command"
# endif // defined(__APPLE__)
/*! @file
 @brief The class declaration for the Kinect V2 input service. */

/*! @namespace MplusM::KinectV2
 @brief The classes that support reading from a Kinect V2 input device. */
# if defined(__APPLE__)
#  pragma clang diagnostic pop
# endif // defined(__APPLE__)

/*! @brief The base channel name to use for the service if not provided. */
# define DEFAULT_KINECTV2INPUT_SERVICE_NAME_ BUILD_NAME_(MpM_SERVICE_BASE_NAME_, \
                                                         BUILD_NAME_("input", "kinectv2"))

/*! @brief The description of the service. */
# define KINECTV2INPUT_SERVICE_DESCRIPTION_ T_("Kinect V2 input service")

namespace MplusM
{
    namespace KinectV2
    {
        class KinectV2EventThread;
        
        /*! @brief The Kinect V2 input service. */
        class KinectV2InputService : public Common::BaseInputService
        {
        public :
        
        protected :
        
        private :
            
            /*! @brief The class that this class is derived from. */
            typedef BaseInputService inherited;
            
        public :
            
            /*! @brief The constructor.
             @param argumentList Descriptions of the arguments to the executable.
             @param launchPath The command-line name used to launch the service.
             @param argc The number of arguments in 'argv'.
             @param argv The arguments passed to the executable used to launch the service.
             @param tag The modifier for the service name and port names.
             @param serviceEndpointName The YARP name to be assigned to the new service.
             @param servicePortNumber The port being used by the service. */
            KinectV2InputService(const Utilities::DescriptorVector & argumentList,
                                 const YarpString &                  launchPath,
                                 const int                           argc,
                                 char * *                            argv,
                                 const YarpString &                  tag,
                                 const YarpString &                  serviceEndpointName,
                                 const YarpString &                  servicePortNumber = "");
            
            /*! @brief The destructor. */
            virtual
            ~KinectV2InputService(void);
            
            DECLARE_CONFIGURE_;
            
            DECLARE_GETCONFIGURATION_;

            DECLARE_RESTARTSTREAMS_;
            
            DECLARE_SHUTDOWNOUTPUTSTREAMS_;
            
            DECLARE_STARTSERVICE_;
            
            DECLARE_STARTSTREAMS_;
            
            DECLARE_STOPSERVICE_;
            
            DECLARE_STOPSTREAMS_;
            
        protected :
            
        private :
            
            COPY_AND_ASSIGNMENT_(KinectV2InputService);

            DECLARE_SETUPSTREAMDESCRIPTIONS_;
            
        public :

        protected :

        private :

            /*! @brief The event thread to use. */
            KinectV2EventThread * _eventThread;

        }; // KinectV2InputService
        
    } // KinectV2
    
} // MplusM

#endif // ! defined(MpMKinectV2InputService_H_)
