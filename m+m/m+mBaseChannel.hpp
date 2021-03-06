//--------------------------------------------------------------------------------------------------
//
//  File:       m+m/m+mBaseChannel.hpp
//
//  Project:    m+m
//
//  Contains:   The class declaration for common resources for channels.
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
//  Created:    2014-10-08
//
//--------------------------------------------------------------------------------------------------

#if (! defined(MpMBaseChannel_HPP_))
# define MpMBaseChannel_HPP_ /* Header guard */

# include <m+m/m+mSendReceiveCounters.hpp>

# if defined(__APPLE__)
#  pragma clang diagnostic push
#  pragma clang diagnostic ignored "-Wunknown-pragmas"
#  pragma clang diagnostic ignored "-Wdocumentation-unknown-command"
# endif // defined(__APPLE__)
/*! @file
 @brief The class declaration for common resources for channels. */
# if defined(__APPLE__)
#  pragma clang diagnostic pop
# endif // defined(__APPLE__)

namespace MplusM
{
    namespace Common
    {
        /*! @brief A convenience class to provide common resources for channels. */
        class BaseChannel : public yarp::os::Port
        {
        public :

        protected :

        private :

            /*! @brief The class that this class is derived from. */
            typedef yarp::os::Port inherited;

        public :

            /*! @brief The constructor. */
            BaseChannel(void);

            /*! @brief The destructor. */
            virtual
            ~BaseChannel(void);

            /*! @brief Close the channel. */
            void
            close(void);

            /*! @brief Turn off the send / receive metrics collecting. */
            void
            disableMetrics(void);

            /*! @brief Turn on the send / receive metrics collecting. */
            void
            enableMetrics(void);

            /*! @brief Return the send / receive counters.
             @param[out] counters The send / receive counters. */
            void
            getSendReceiveCounters(SendReceiveCounters & counters);

            /*! @brief Return the state of the  send / receive metrics.
             @returns @c true if the send / receive metrics are being gathered and @c false
             otherwise. */
            inline bool
            metricsAreEnabled(void)
            const
            {
                return _metricsEnabled;
            } // metricsAreEnabled

            /*! @brief Returns the name associated with the channel.
             @returns The name associated with the channel. */
            inline const YarpString &
            name(void)
            const
            {
                return _name;
            } // name

            /*! @brief Open the channel, using a backoff strategy with retries.
             @param[in] theChannelName The name to be associated with the channel.
             @param[in] timeToWait The number of seconds allowed before a failure is considered.
             @returns @c true if the channel was opened and @c false if it could not be opened. */
            bool
            openWithRetries(const YarpString & theChannelName,
                            const double       timeToWait);

            /*! @brief Open the channel, using a backoff strategy with retries.
             @param[in,out] theContactInfo The connection information to be associated with the
             channel.
             @param[in] timeToWait The number of seconds allowed before a failure is considered.
             @returns @c true if the channel was opened and @c false if it could not be opened. */
            bool
            openWithRetries(yarp::os::Contact & theContactInfo,
                            const double        timeToWait);

            /*! @brief Release an allocated adapter channel.
             @param[in] theChannel A pointer to the channel to be released. */
            static void
            RelinquishChannel(BaseChannel * theChannel);

            /*! @brief Update the receive counters for the channel.
             @param[in] numBytes The number of bytes received. */
            void
            updateReceiveCounters(const size_t numBytes);

            /*! @brief Update the send counters for the channel.
             @param[in] numBytes The number of bytes sent. */
            void
            updateSendCounters(const size_t numBytes);

            /*! @brief Write a message to the port.
             @param[in] message The message to write.
             @returns @c true if the message was successfully sent and @c false otherwise. */
            bool
            writeBottle(yarp::os::Bottle & message);

            /*! @brief Write a message to the port, with a reply expected.
             @param[in] message The message to write.
             @param[in,out] reply Where to put the expected reply.
             @returns @c true if the message was successfully sent and @c false otherwise. */
            bool
            writeBottle(yarp::os::Bottle & message,
                        yarp::os::Bottle & reply);

        protected :

        private :

            /*! @brief The copy constructor.
             @param[in] other The object to be copied. */
            BaseChannel(const BaseChannel & other);

            /*! @brief The assignment operator.
             @param[in] other The object to be copied.
             @returns The updated object. */
            BaseChannel &
            operator =(const BaseChannel & other);

        public :

        protected :

        private :

            /*! @brief The name associated with the channel. */
            YarpString _name;

            /*! @brief The send / receive counters. */
            SendReceiveCounters _counters;

            /*! @brief @c true if metrics are enabled and @c false otherwise. */
            bool _metricsEnabled;

# if defined(__APPLE__)
#  pragma clang diagnostic push
#  pragma clang diagnostic ignored "-Wunused-private-field"
# endif // defined(__APPLE__)
            /*! @brief Filler to pad to alignment boundary */
            char _filler[7];
# if defined(__APPLE__)
#  pragma clang diagnostic pop
# endif // defined(__APPLE__)

        }; // BaseChannel

    } // Common

} // MplusM

#endif // ! defined(MpMBaseChannel_HPP_)
