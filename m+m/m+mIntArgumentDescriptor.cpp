//--------------------------------------------------------------------------------------------------
//
//  File:       m+m/m+mIntArgumentDescriptor.cpp
//
//  Project:    m+m
//
//  Contains:   The class definition for the minimal functionality required to represent an integer
//              command-line argument.
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
//  Created:    2015-05-15
//
//--------------------------------------------------------------------------------------------------

#include "m+mIntArgumentDescriptor.hpp"

//#include <odlEnable.h>
#include <odlInclude.h>

#if defined(__APPLE__)
# pragma clang diagnostic push
# pragma clang diagnostic ignored "-Wunknown-pragmas"
# pragma clang diagnostic ignored "-Wdocumentation-unknown-command"
#endif // defined(__APPLE__)
/*! @file
 @brief The definition for the minimal functionality required to represent an integer command-line
 argument. */
#if defined(__APPLE__)
# pragma clang diagnostic pop
#endif // defined(__APPLE__)

#if defined(__APPLE__)
# pragma mark Namespace references
#endif // defined(__APPLE__)

using namespace MplusM;
using namespace MplusM::Common;
using namespace MplusM::Utilities;

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

IntArgumentDescriptor::IntArgumentDescriptor(const YarpString & argName,
                                             const YarpString & argDescription,
                                             const ArgumentMode argMode,
                                             const int          defaultValue,
                                             const bool         hasMinimumValue,
                                             const int          minimumValue,
                                             const bool         hasMaximumValue,
                                             const int          maximumValue) :
    inherited(argName, argDescription, argMode), _defaultValue(defaultValue),
    _maximumValue(maximumValue), _minimumValue(minimumValue), _hasMaximumValue(hasMaximumValue),
    _hasMinimumValue(hasMinimumValue)
{
    ODL_ENTER(); //####
    ODL_S2s("argName = ", argName, "argDescription = ", argDescription); //####
    ODL_LL3("defaultValue = ", defaultValue, "minimumValue = ", minimumValue, //####
            "maximumValue = ", maximumValue); //####
    ODL_B2("hasMinimumValue = ", hasMinimumValue, "hasMaximumValue = ", hasMaximumValue); //####
    ODL_EXIT_P(this); //####
} // IntArgumentDescriptor::IntArgumentDescriptor

IntArgumentDescriptor::~IntArgumentDescriptor(void)
{
    ODL_OBJENTER(); //####
    ODL_OBJEXIT(); //####
} // IntArgumentDescriptor::~IntArgumentDescriptor

#if defined(__APPLE__)
# pragma mark Actions and Accessors
#endif // defined(__APPLE__)

void
IntArgumentDescriptor::addValueToBottle(yarp::os::Bottle & container)
{
    ODL_ENTER(); //####
    ODL_P1("container = ", &container); //####
    container.addInt(_currentValue);
    ODL_EXIT(); //####
} // IntArgumentDescriptor::addValueToBottle

BaseArgumentDescriptor *
IntArgumentDescriptor::clone(void)
{
    ODL_OBJENTER(); //####
    BaseArgumentDescriptor * result = new IntArgumentDescriptor(argumentName(),
                                                                argumentDescription(),
                                                                argumentMode(), _defaultValue,
                                                                _hasMinimumValue, _minimumValue,
                                                                _hasMaximumValue, _maximumValue);

    ODL_EXIT_P(result);
    return result;
} // IntArgumentDescriptor::clone

YarpString
IntArgumentDescriptor::getDefaultValue(void)
{
    ODL_OBJENTER(); //####
    YarpString        result;
    std::stringstream buff;

    buff << _defaultValue;
    result = buff.str();
    ODL_OBJEXIT_s(result); //####
    return result;
} // IntArgumentDescriptor::getDefaultValue

YarpString
IntArgumentDescriptor::getProcessedValue(void)
{
    ODL_OBJENTER(); //####
    YarpString        result;
    std::stringstream buff;

    buff << _currentValue;
    result = buff.str();
    ODL_OBJEXIT_s(result); //####
    return result;
} // IntArgumentDescriptor::getProcessedValue

BaseArgumentDescriptor *
IntArgumentDescriptor::parseArgString(const YarpString & inString)
{
    ODL_ENTER(); //####
    ODL_S1s("inString = ", inString); //####
    BaseArgumentDescriptor * result = NULL;
    YarpStringVector         inVector;

    if (partitionString(inString, 5, inVector))
    {
        ArgumentMode argMode;
        bool         okSoFar = true;
        int          defaultValue;
        int          maxValue;
        int          minValue;
        YarpString   name(inVector[0]);
        YarpString   typeTag(inVector[1]);
        YarpString   modeString(inVector[2]);
        YarpString   minValString(inVector[3]);
        YarpString   maxValString(inVector[4]);
        YarpString   defaultString(inVector[5]);
        YarpString   description(inVector[6]);

        if (typeTag != "I")
        {
            okSoFar = false;
        }
        if (okSoFar)
        {
            argMode = ModeFromString(modeString);
            okSoFar = (kArgModeUnknown != argMode);
        }
        else
        {
            argMode = kArgModeUnknown;
        }
        if (okSoFar && (0 < defaultString.length()))
        {
            const char * startPtr = defaultString.c_str();
            char *       endPtr;

            defaultValue = strtol(startPtr, &endPtr, 10);
            if ((startPtr == endPtr) || *endPtr)
            {
                okSoFar = false;
            }
        }
        if (okSoFar && (0 < minValString.length()))
        {
            const char * startPtr = minValString.c_str();
            char *       endPtr;

            minValue = strtol(startPtr, &endPtr, 10);
            if ((startPtr == endPtr) || *endPtr)
            {
                okSoFar = false;
            }
        }
        if (okSoFar && (0 < maxValString.length()))
        {
            const char * startPtr = maxValString.c_str();
            char *       endPtr;

            maxValue = strtol(startPtr, &endPtr, 10);
            if ((startPtr == endPtr) || *endPtr)
            {
                okSoFar = false;
            }
        }
        if (okSoFar)
        {
            bool hasMaximumValue = (0 < maxValString.length());
            bool hasMinimumValue = (0 < minValString.length());

            result = new IntArgumentDescriptor(name, description, argMode, defaultValue,
                                               hasMinimumValue, hasMinimumValue ? minValue : 0,
                                               hasMaximumValue, hasMaximumValue ? maxValue : 0);
        }
    }
    ODL_EXIT_P(result); //####
    return result;
} // IntArgumentDescriptor::parseArgString

void
IntArgumentDescriptor::setToDefaultValue(void)
{
    ODL_OBJENTER(); //####
    _currentValue = _defaultValue;
    ODL_LL1("_currentValue <- ", _currentValue); //####
    ODL_OBJEXIT(); //####
} // IntArgumentDescriptor::setToDefaultValue

YarpString
IntArgumentDescriptor::toString(void)
{
    ODL_OBJENTER(); //####
    YarpString result(prefixFields("I"));

    result += _parameterSeparator;
    if (_hasMinimumValue)
    {
        std::stringstream buff;

        buff << _minimumValue;
        result += buff.str();
    }
    result += _parameterSeparator;
    if (_hasMaximumValue)
    {
        std::stringstream buff;

        buff << _maximumValue;
        result += buff.str();
    }
    result += suffixFields(getDefaultValue());
    ODL_OBJEXIT_s(result); //####
    return result;
} // IntArgumentDescriptor::toString

bool
IntArgumentDescriptor::validate(const YarpString & value)
{
    ODL_OBJENTER(); //####
    const char * startPtr = value.c_str();
    char *       endPtr;
    int          intValue = strtol(startPtr, &endPtr, 10);

    if ((startPtr != endPtr) && (! *endPtr))
    {
        _valid = true;
        ODL_B1("_valid <- ", _valid); //####
        if (_hasMinimumValue && (intValue < _minimumValue))
        {
            _valid = false;
            ODL_B1("_valid <- ", _valid); //####
        }
        if (_hasMaximumValue && (intValue > _maximumValue))
        {
            _valid = false;
            ODL_B1("_valid <- ", _valid); //####
        }
    }
    else
    {
        _valid = false;
        ODL_B1("_valid <- ", _valid); //####
    }
    if (_valid)
    {
        _currentValue = intValue;
        ODL_LL1("_currentValue <- ", _currentValue); //####
    }
    ODL_OBJEXIT_B(_valid); //####
    return _valid;
} // IntArgumentDescriptor::validate

#if defined(__APPLE__)
# pragma mark Global functions
#endif // defined(__APPLE__)
