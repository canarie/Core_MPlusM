//--------------------------------------------------------------------------------------------------
//
//  File:       m+mSendToMQOutputService.hpp
//
//  Project:    m+m
//
//  Contains:   The class declaration for the SendToMQ output service.
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
//  Created:    2015-07-26
//
//--------------------------------------------------------------------------------------------------

#if (! defined(MpMSendToMQOutputService_HPP_))
# define MpMSendToMQOutputService_HPP_ /* Header guard */

# include <m+m/m+mBaseOutputService.hpp>
# include <m+m/m+mUtilities.hpp>

/* Note that the following macro names are used for static integer class variables! */
# undef MIN_PRIORITY
# undef MAX_PRIORITY
# undef NORM_PRIORITY
# include <activemq/library/ActiveMQCPP.h>
# include <activemq/core/ActiveMQConnectionFactory.h>

# if defined(__APPLE__)
#  pragma clang diagnostic push
#  pragma clang diagnostic ignored "-Wunknown-pragmas"
#  pragma clang diagnostic ignored "-Wdocumentation-unknown-command"
# endif // defined(__APPLE__)
/*! @file
 @brief The class declaration for the %SendToMQ output service. */

/*! @namespace MplusM::SendToMQ
 @brief The classes that support writing to a %SendToMQ output system. */
# if defined(__APPLE__)
#  pragma clang diagnostic pop
# endif // defined(__APPLE__)

/*! @brief The base channel name to use for the service if not provided. */
# define DEFAULT_SENDTOMQOUTPUT_SERVICE_NAME_ BUILD_NAME_(MpM_SERVICE_BASE_NAME_, \
                                                          BUILD_NAME_("output", "sendtoMQ"))

/*! @brief The description of the service. */
# define SENDTOMQOUTPUT_SERVICE_DESCRIPTION_ T_("Send To MQ output service")

namespace MplusM
{
    namespace SendToMQ
    {
        class SendToMQOutputInputHandler;

        /*! @brief The %Blob output service. */
        class SendToMQOutputService : public Common::BaseOutputService
        {
        public :

        protected :

        private :

            /*! @brief The class that this class is derived from. */
            typedef BaseOutputService inherited;

        public :

            /*! @brief The constructor.
             @param[in] hostName The name of the MQ broker.
             @param[in] hostPort The port to connect to the MQ broker.
             @param[in] userName The user name for the MQ broker.
             @param[in] userPassword The user password for the MQ broker.
             @param[in] argumentList Descriptions of the arguments to the executable.
             @param[in] launchPath The command-line name used to launch the service.
             @param[in] argc The number of arguments in 'argv'.
             @param[in] argv The arguments passed to the executable used to launch the service.
             @param[in] tag The modifier for the service name and port names.
             @param[in] serviceEndpointName The YARP name to be assigned to the new service.
             @param[in] servicePortNumber The port being used by the service. */
            SendToMQOutputService(const YarpString &                  hostName,
                                  const int                           hostPort,
                                  const YarpString &                  userName,
                                  const YarpString &                  userPassword,
                                  const Utilities::DescriptorVector & argumentList,
                                  const YarpString &                  launchPath,
                                  const int                           argc,
                                  char * *                            argv,
                                  const YarpString &                  tag,
                                  const YarpString &                  serviceEndpointName,
                                  const YarpString &                  servicePortNumber = "");

            /*! @brief The destructor. */
            virtual
            ~SendToMQOutputService(void);

            /*! @brief Configure the input/output streams.
             @param[in] details The configuration information for the input/output streams.
             @returns @c true if the service was successfully configured and @c false otherwise. */
            virtual bool
            configure(const yarp::os::Bottle & details);

            /*! @brief Deactivate the network connection. */
            void
            deactivateConnection(void);

            /*! @brief Turn off the send / receive metrics collecting. */
            virtual void
            disableMetrics(void);

            /*! @brief Turn on the send / receive metrics collecting. */
            virtual void
            enableMetrics(void);

            /*! @brief Get the configuration of the input/output streams.
             @param[out] details The configuration information for the input/output streams.
             @returns @c true if the configuration was successfully retrieved and @c false
             otherwise. */
            virtual bool
            getConfiguration(yarp::os::Bottle & details);

            /*! @brief Send a message via ActiveMQ.
             @param[in] aMessage The message to send.
             @param[in] messageLength The length of the message. */
            void
            sendMessage(const std::string & aMessage,
                        const size_t        messageLength);

            /*! @brief Start the input / output streams. */
            virtual void
            startStreams(void);

            /*! @brief Stop the input / output streams. */
            virtual void
            stopStreams(void);

        protected :

        private :

            /*! @brief The copy constructor.
             @param[in] other The object to be copied. */
            SendToMQOutputService(const SendToMQOutputService & other);

            /*! @brief The assignment operator.
             @param[in] other The object to be copied.
             @returns The updated object. */
            SendToMQOutputService &
            operator =(const SendToMQOutputService & other);

            /*! @brief Set up the descriptions that will be used to construct the input / output
             streams.
             @returns @c true if the descriptions were set up and @c false otherwise. */
            virtual bool
            setUpStreamDescriptions(void);

        public :

        protected :

        private :

            /*! @brief The name of the MQ broker. */
            YarpString _hostName;

            /*! @brief The user password for the MQ broker. */
            YarpString _password;

            /*! @brief The user name for the MQ broker. */
            YarpString _userName;

            /*! @brief The topic or queue name to use. */
            YarpString _topicOrQueueName;

            /*! @brief The port to connect to the MQ broker. */
            int _hostPort;

            /*! @brief The handler for input data. */
            SendToMQOutputInputHandler * _inHandler;

            /*! @brief The connection generating factory. */
            std::auto_ptr<cms::ConnectionFactory> _connectionFactory;

            /*! @brief The active connection. */
            cms::Connection * _connection;

            /*! @brief The active session. */
            cms::Session * _session;

            /*! @brief The destination for the active session. */
            cms::Destination * _destination;

            /*! @brief The message producer that works with the active session. */
            cms::MessageProducer * _producer;

            /*! @brief If @c true, use a queue for transmission else use a topic. */
            bool _useQueue;

# if defined(__APPLE__)
#  pragma clang diagnostic push
#  pragma clang diagnostic ignored "-Wunused-private-field"
# endif // defined(__APPLE__)
            /*! @brief Filler to pad to alignment boundary */
            char _filler[7];
# if defined(__APPLE__)
#  pragma clang diagnostic pop
# endif // defined(__APPLE__)

        }; // SendToMQOutputService

    } // SendToMQ

} // MplusM

#endif // ! defined(MpMSendToMQOutputService_HPP_)
