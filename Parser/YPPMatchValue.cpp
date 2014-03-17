//--------------------------------------------------------------------------------------
//
//  File:       YPPMatchValue.cpp
//
//  Project:    YarpPlusPlus
//
//  Contains:   The class definition for a pattern matcher for simple values.
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
//  Created:    2014-03-07
//
//--------------------------------------------------------------------------------------

#include "YPPMatchValue.h"
//#define ENABLE_OD_SYSLOG /* */
#include "ODSyslog.h"
#include "YPPMatchConstraint.h"
#include "YPPMatchExpression.h"
#include "YPPMatchValueList.h"
#include <cctype>

using namespace YarpPlusPlusParser;

#if defined(__APPLE__)
# pragma mark Private structures, constants and variables
#endif // defined(__APPLE__)

/*! @brief The asterisk character, which is one of our wildcard characters. */
static const char kAsterisk = '*';
/*! @brief The double quote character. */
static const char kDoubleQuote = '"';
/*! @brief The 'escape' character - a backslash. */
static const char kEscapeCharacter = '\\';
/*! @brief The percent character, which is an SQL pattern character. */
static const char kPercent = '%';
/*! @brief The question mark character, which is one of our wildcard characters. */
static const char kQuestionMark = '?';
/*! @brief The single quote character. */
static const char kSingleQuote = '\'';
/*! @brief The underscore character, which is an SQL pattern character. */
static const char kUnderscore = '_';

#if defined(__APPLE__)
# pragma mark Local functions
#endif // defined(__APPLE__)

#if defined(__APPLE__)
# pragma mark Class methods
#endif // defined(__APPLE__)

MatchValue * MatchValue::CreateMatcher(const yarp::os::ConstString & inString,
                                       const int                     inLength,
                                       const int                     startPos,
                                       int &                         endPos)
{
    OD_SYSLOG_ENTER();//####
    OD_SYSLOG_S1("inString = ", inString.c_str());//####
    OD_SYSLOG_LL2("inLength = ", inLength, "startPos = ", startPos);//####
    int          workPos = SkipWhitespace(inString, inLength, startPos);
    MatchValue * result = NULL;
    
    if (workPos < inLength)
    {
        OD_SYSLOG("(workPos < inLength)");//####
        // Remember where we began.
        bool                  escapeNextChar = false;
        char                  delimiter;
        char                  scanChar = inString[workPos];
        const char            constraintSeparator = MatchConstraint::ConstraintSeparatorCharacter();
        const char            expressionSeparator = MatchExpression::ExpressionSeparatorCharacter();
        const char            listInitiator = MatchValueList::ListInitiatorCharacter();
        const char            listSeparator = MatchValueList::ListSeparatorCharacter();
        const char            listTerminator = MatchValueList::ListTerminatorCharacter();
        yarp::os::ConstString assembled;
        int                   startSubPos = workPos;
        
        // If we have a quote character, scan for the matching character. If we have an illegal starting character,
        // reject the string.
        if ((kDoubleQuote == scanChar) || (kSingleQuote == scanChar))
        {
            // A delimited string.
            delimiter = scanChar;
            ++startSubPos;
        }
        else if (listInitiator == scanChar)
        {
            // We've seen the start of a list - this is not a singular value!
            workPos = inLength;
            delimiter = '\1';
        }
        else if (kEscapeCharacter == scanChar)
        {
            // The first character needed to be escaped.
            delimiter = '\0';
            escapeNextChar = true;
        }
        else
        {
            // A normal character.
            delimiter = '\0';
            assembled += scanChar;
        }
        for (++workPos; workPos < inLength; ++workPos)
        {
            scanChar = inString[workPos];
            if (escapeNextChar)
            {
                escapeNextChar = false;
            }
            else if (delimiter)
            {
                if (delimiter == scanChar)
                {
                    break;
                }
                
            }
            else if (kEscapeCharacter == scanChar)
            {
                escapeNextChar = true;
                continue;
            }
            else if (isspace(scanChar) || (listSeparator == scanChar) || (listTerminator == scanChar) ||
                     (constraintSeparator == scanChar) || (expressionSeparator == scanChar))
            {
                break;
            }
            
            assembled += scanChar;
        }
        OD_SYSLOG_S1("assembled = ", assembled.c_str());//####
        // If we have a delimiter, then we must match before the end of the input string. If we don't have a delimiter,
        // we can match the rest of the input string.
        if (workPos < (inLength + (delimiter ? 0 : 1)))
        {
            OD_SYSLOG("(workPos < (inLength + (delimiter ? 0 : 1)))");//####
            // Either we stopped with a blank or the end of the string, or we saw a matching delimiter before the end.
            if (0 < (workPos - startSubPos))
            {
                // If we have a non-empty substring, we have success.
                result = new MatchValue(assembled);
            }
        }
        if (result)
        {
            endPos = (delimiter ? 1 : 0) + workPos;
        }        
    }
    OD_SYSLOG_EXIT_P(result);//####
    return result;
} // MatchValue::CreateMatcher

#if defined(__APPLE__)
# pragma mark Constructors and destructors
#endif // defined(__APPLE__)

MatchValue::MatchValue(const yarp::os::ConstString & inString) :
        inherited(), _matchingString(inString), _hasSingleQuotes(false), _hasWildcards(false), _needsEscaping(false)
{
    OD_SYSLOG_ENTER();//####
    OD_SYSLOG_S1("inString = ", inString.c_str());//####
    // Mark if the string will need escaping.
    for (int ii = 0, len = inString.length(); ii < len; ++ii)
    {
        char walker = inString[ii];
        
        // If there are SQL special characters present, flag this as needing to be escaped.
        if ((kUnderscore == walker) || (kPercent == walker) || (kEscapeCharacter == walker))
        {
            _needsEscaping = true;
        }
        else if ((kAsterisk == walker) || (kQuestionMark == walker))
        {
            // If there are wildcard characters present, flag this.
            _hasWildcards = true;
        }
        else if (kSingleQuote == walker)
        {
            // If there are single quote characters present, flag this.
            _hasSingleQuotes = true;
        }
    }
    OD_SYSLOG_EXIT_P(this);//####
} // MatchValue::MatchValue

MatchValue::~MatchValue(void)
{
    OD_SYSLOG_ENTER();//####
    OD_SYSLOG_EXIT();//####
} // MatchValue::~MatchValue

#if defined(__APPLE__)
# pragma mark Actions
#endif // defined(__APPLE__)

yarp::os::ConstString MatchValue::asSQLString(void)
const
{
    OD_SYSLOG_ENTER();//####
    yarp::os::ConstString converted;
    
    converted += kSingleQuote;
    if (_hasSingleQuotes || _hasWildcards || _needsEscaping)
    {
        OD_SYSLOG("(_hasWildcards || _needsEscaping)");//####
        for (int ii = 0, len = _matchingString.length(); ii < len; ++ii)
        {
            char walker = _matchingString[ii];
            
            // If there are SQL special characters present, escape them.
            if ((kUnderscore == walker) || (kPercent == walker) || (kEscapeCharacter == walker))
            {
                converted += kEscapeCharacter;
                converted += walker;
            }
            else if (kAsterisk == walker)
            {
                // Substitute the corresponding SQL wildcard character.
                converted += kPercent;
            }
            else if (kQuestionMark == walker)
            {
                // Substitute the corresponding SQL wildcard character.
                converted += kUnderscore;
            }
            else if (kSingleQuote == walker)
            {
                converted += kSingleQuote;
                converted += walker;
            }
            else
            {
                converted += walker;
            }
        }
        if (_needsEscaping)
        {
            converted += kSingleQuote;
            converted += " ESCAPE ";
            converted += kSingleQuote;
            converted += kEscapeCharacter;
        }
    }
    else
    {
        converted += _matchingString;
    }
    converted += kSingleQuote;
    OD_SYSLOG_EXIT_S(converted.c_str());//####
    return converted;
} // MatchValue::asSQLString

yarp::os::ConstString MatchValue::asString(void)
const
{
    bool                  sawDoubleQuote = false;
    bool                  sawSingleQuote = false;
    bool                  sawWhitespace = false;
    yarp::os::ConstString converted;
    int                   len = _matchingString.length();

    // First, check if there are blanks or quotes in the string:
    for (int ii = 0; ii < len; ++ii)
    {
        char walker = _matchingString[ii];
        
        if (isspace(walker))
        {
            sawWhitespace = true;
        }
        else if (kDoubleQuote == walker)
        {
            sawDoubleQuote = true;
        }
        else if (kSingleQuote == walker)
        {
            sawSingleQuote = true;
        }
    }
    if (sawWhitespace || sawDoubleQuote || sawSingleQuote)
    {
        if (sawDoubleQuote && sawSingleQuote)
        {
            // If both quotes are present, use double quotes and escape any double quotes that we find.
            converted += kDoubleQuote;
            for (int ii = 0; ii < len; ++ii)
            {
                char walker = _matchingString[ii];
                
                if (kDoubleQuote == walker)
                {
                    converted += kEscapeCharacter;
                }
                converted += walker;
            }
            converted += kDoubleQuote;
        }
        else if (sawDoubleQuote)
        {
            // If only one type of quote is present, use the other quote.
            converted += kSingleQuote;
            converted += _matchingString;
            converted += kSingleQuote;
        }
        else if (sawSingleQuote)
        {
            // If only one type of quote is present, use the other quote.
            converted += kDoubleQuote;
            converted += _matchingString;
            converted += kDoubleQuote;
        }
        else
        {
            // If no quotes are present, just use double quotes.
            converted += kDoubleQuote;
            converted += _matchingString;
            converted += kDoubleQuote;
        }
    }
    else
    {
        // If neither blanks nor quotes are in the string, just return the string.
        converted = _matchingString;
    }
    return converted;
} // MatchValue::asString

#if defined(__APPLE__)
# pragma mark Accessors
#endif // defined(__APPLE__)

#if defined(__APPLE__)
# pragma mark Global functions
#endif // defined(__APPLE__)
