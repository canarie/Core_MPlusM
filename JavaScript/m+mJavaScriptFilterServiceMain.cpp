//--------------------------------------------------------------------------------------------------
//
//  File:       m+mJavaScriptFilterServiceMain.cpp
//
//  Project:    m+m
//
//  Contains:   The main application for the JavaScript filter service.
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
//  Created:    2015-01-05
//
//--------------------------------------------------------------------------------------------------

#include "m+mJavaScriptFilterService.hpp"

#include <m+m/m+mEndpoint.hpp>
#include <m+m/m+mExtraArgumentDescriptor.hpp>
#include <m+m/m+mFilePathArgumentDescriptor.hpp>
#include <m+m/m+mUtilities.hpp>

//#include <odlEnable.h>
#include <odlInclude.h>

#if defined(__APPLE__)
# pragma clang diagnostic push
# pragma clang diagnostic ignored "-Wunknown-pragmas"
# pragma clang diagnostic ignored "-Wdocumentation-unknown-command"
#endif // defined(__APPLE__)
/*! @file
 @brief The main application for the %JavaScript filter service. */

/*! @dir JavaScript
 @brief The set of files that implement the %JavaScript filter service. */
#if defined(__APPLE__)
# pragma clang diagnostic pop
#endif // defined(__APPLE__)

#if defined(__APPLE__)
# pragma mark Namespace references
#endif // defined(__APPLE__)

using namespace MplusM;
using namespace MplusM::Common;
using namespace MplusM::JavaScript;
using std::cin;
using std::cout;
using std::endl;
using std::cerr;

#if defined(__APPLE__)
# pragma mark Private structures, constants and variables
#endif // defined(__APPLE__)

/*! @brief The number of megabytes before the %JavaScript engine triggers a garbage collection. */
#define JAVASCRIPT_GC_SIZE_ 16

/*! @brief The number of bytes for each %JavaScript 'stack chunk'. */
#define JAVASCRIPT_STACKCHUNK_SIZE_ 8192

/*! @brief The class of the global object. */
static JSClass lGlobalClass =
{
    "global",            // name
    JSCLASS_GLOBAL_FLAGS // flags
}; // lGlobalClass

#if defined(__APPLE__)
# pragma mark Global constants and variables
#endif // defined(__APPLE__)

#if defined(__APPLE__)
# pragma mark Local functions
#endif // defined(__APPLE__)

/*! @brief The error reporter callback for the %JavaScript engine.
 @param[in] cx The context in which the error happened.
 @param[in] message An error message.
 @param[in] report An error report record containing additional details about the error. */
static void
reportJavaScriptError(JSContext *     cx,
                      const char *    message,
                      JSErrorReport * report)
{
    // Note that, since this is a callback for the JavaScript engine, it must NOT throw any C++
    // exceptions!
    try
    {
        YarpString        errMessage(report->filename ? report->filename : "[no filename]");
        std::stringstream buff;

        buff << report->lineno << ":" << message;
        errMessage += buff.str();
        MpM_FAIL_(errMessage.c_str());
    }
    catch (...)
    {
        // Suppress any C++ exception caused by this function.
    }
} // reportJavaScriptError

/*! @brief A C-callback function for %JavaScript to write out an object.
 @param[in] jct The context in which the native function is being called.
 @param[in] argc The number of arguments supplied to the function by the caller.
 @param[in] vp The arguments to the function.
 @returns @c true on success and @c false otherwise. */
static bool
dumpObjectToStdoutForJs(JSContext * jct,
                        unsigned    argc,
                        JS::Value * vp)
{
    ODL_ENTER(); //####
    ODL_P2("jct = ", jct, "vp = ", vp); //####
    ODL_L1("argc = ", argc); //####
    bool         result;
    JS::CallArgs args = JS::CallArgsFromVp(argc, vp);

    if (2 == args.length())
    {
        if (args[0].isString())
        {
            if (args[1].isObject())
            {
                JS::RootedObject asObject(jct);

                if (JS_ValueToObject(jct, args[1], &asObject))
                {
                    JSString * asString = args[0].toString();
                    char *     asChars = JS_EncodeString(jct, asString);

                    cout << asChars << endl;
                    JS_free(jct, asChars);
                    PrintJavaScriptObject(cout, jct, asObject, 1);
                    cout.flush();
                    result = true;
                }
            }
            else
            {
                JS_ReportError(jct, "Non-object argument to dumpObjectToStdout");
                result = false;
            }
        }
        else
        {
            JS_ReportError(jct, "Non-string argument to dumpObjectToStdout");
            result = false;
        }
    }
    else if (2 < args.length())
    {
        JS_ReportError(jct, "Extra arguments to dumpObjectToStdout");
        result = false;
    }
    else
    {
        JS_ReportError(jct, "Missing arguments to dumpObjectToStdout");
        result = false;
    }
    ODL_EXIT_B(result); //####
    return result;
} // dumpObjectToStdoutForJs

/*! @brief A C-callback function for %JavaScript to get the current time in seconds.
 @param[in] jct The context in which the native function is being called.
 @param[in] argc The number of arguments supplied to the function by the caller.
 @param[in] vp The arguments to the function.
 @returns @c true on success and @c false otherwise. */
static bool
getTimeNowForJs(JSContext * jct,
                unsigned    argc,
                JS::Value * vp)
{
    ODL_ENTER(); //####
    ODL_P2("jct = ", jct, "vp = ", vp); //####
    ODL_L1("argc = ", argc); //####
    bool         result;
    JS::CallArgs args = JS::CallArgsFromVp(argc, vp);

    if (0 < args.length())
    {
        JS_ReportError(jct, "Extra arguments to requestStop");
        result = false;
    }
    else
    {
        args.rval().setDouble(yarp::os::Time::now());
        result = true;
    }
    ODL_EXIT_B(result); //####
    return result;
} // getTimeNowForJs

/*! @brief A C-callback function for %JavaScript to stop the service.
 @param[in] jct The context in which the native function is being called.
 @param[in] argc The number of arguments supplied to the function by the caller.
 @param[in] vp The arguments to the function.
 @returns @c true on success and @c false otherwise. */
static bool
requestStopForJs(JSContext * jct,
                 unsigned    argc,
                 JS::Value * vp)
{
    ODL_ENTER(); //####
    ODL_P2("jct = ", jct, "vp = ", vp); //####
    ODL_L1("argc = ", argc); //####
    bool         result;
    JS::CallArgs args = JS::CallArgsFromVp(argc, vp);

    if (0 < args.length())
    {
        JS_ReportError(jct, "Extra arguments to requestStop");
        result = false;
    }
    else
    {
        JavaScriptFilterService * theService =
                            reinterpret_cast<JavaScriptFilterService *>(JS_GetContextPrivate(jct));

        if (theService)
        {
            theService->requestServiceStop();
        }
        result = true;
    }
    ODL_EXIT_B(result); //####
    return result;
} // requestStopForJs

/*! @brief A C-callback function for %JavaScript to send an object to a channel.
 @param[in] jct The context in which the native function is being called.
 @param[in] argc The number of arguments supplied to the function by the caller.
 @param[in] vp The arguments to the function.
 @returns @c true on success and @c false otherwise. */
static bool
sendToChannelForJs(JSContext * jct,
                   unsigned    argc,
                   JS::Value * vp)
{
    ODL_ENTER(); //####
    ODL_P2("jct = ", jct, "vp = ", vp); //####
    ODL_L1("argc = ", argc); //####
    bool         result = false;
    JS::CallArgs args = JS::CallArgsFromVp(argc, vp);

    if (2 == args.length())
    {
        // Check that the first argument is a valid integer.
        if (args[0].isInt32())
        {
            int32_t                   channelSlot = args[0].toInt32();
            JavaScriptFilterService * theService =
                            reinterpret_cast<JavaScriptFilterService *>(JS_GetContextPrivate(jct));

            if (theService)
            {
                result = theService->sendToChannel(channelSlot, args[1]);
            }
        }
    }
    else if (2 < args.length())
    {
        JS_ReportError(jct, "Extra arguments to sendToChannel");
    }
    else
    {
        JS_ReportError(jct, "Missing argument(s) to sendToChannel");
    }
    ODL_EXIT_B(result); //####
    return result;
} // sendToChannelForJs

/*! @brief A C-callback function for %JavaScript to write out a string.
 @param[in] jct The context in which the native function is being called.
 @param[in] argc The number of arguments supplied to the function by the caller.
 @param[in] vp The arguments to the function.
 @returns @c true on success and @c false otherwise. */
static bool
writeLineToStdoutForJs(JSContext * jct,
                       unsigned    argc,
                       JS::Value * vp)
{
    ODL_ENTER(); //####
    ODL_P2("jct = ", jct, "vp = ", vp); //####
    ODL_L1("argc = ", argc); //####
    bool         result;
    JS::CallArgs args = JS::CallArgsFromVp(argc, vp);

    if (0 == args.length())
    {
        JS_ReportError(jct, "Missing argument to writeLineToStdout");
        result = false;
    }
    else if (1 < args.length())
    {
        JS_ReportError(jct, "Extra arguments to writeLineToStdout");
        result = false;
    }
    else if (args[0].isString())
    {
        JSString * asString = args[0].toString();
        char *     asChars = JS_EncodeString(jct, asString);

        cout << asChars << endl;
        JS_free(jct, asChars);
        cout.flush();
        result = true;
    }
    else
    {
        JS_ReportError(jct, "Non-string argument to writeLineToStdout");
        result = false;
    }
    ODL_EXIT_B(result); //####
    return result;
} // writeLineToStdoutForJs

/*! @brief The table of supplied functions for the service. */
static const JSFunctionSpec lServiceFunctions[] =
{
    // name, call, nargs, flags
    JS_FS("dumpObjectToStdout", dumpObjectToStdoutForJs, 2, JSPROP_ENUMERATE),
    JS_FS("getTimeNow", getTimeNowForJs, 0, JSPROP_ENUMERATE),
    JS_FS("requestStop", requestStopForJs, 0, JSPROP_ENUMERATE),
    JS_FS("sendToChannel", sendToChannelForJs, 2, JSPROP_ENUMERATE),
    JS_FS("writeLineToStdout", writeLineToStdoutForJs, 1, JSPROP_ENUMERATE),
    JS_FS_END
}; // lServiceFunctions

/*! @brief Add custom functions to the %JavaScript environment.
 @param[in] jct The %JavaScript engine context.
 @param[in,out] global The %JavaScript global object.
 @returns @c true if the custom functions were addeded successfully and @c false otherwise. */
static bool
addCustomFunctions(JSContext *        jct,
                   JS::RootedObject & global)
{
    ODL_ENTER(); //####
    ODL_P2("jct = ", jct, "global = ", &global); //####
    bool okSoFar = JS_DefineFunctions(jct, global, lServiceFunctions);

    ODL_EXIT_B(okSoFar); //####
    return okSoFar;
} // addCustomFunctions

// The following forward reference is needed since lStreamClass refers to this function and the
// function uses lStreamClass.
/*! @brief Release resources used by a Stream object.
 @param[in] freeOp The environment in which this is being performed.
 @param[in] obj The Stream object being released. */
static void
cleanupStreamObject(JSFreeOp * freeOp,
                    JSObject * obj);

/*! @brief The function pointers for the stream class. */
static JSClassOps lStreamOps =
{
	NULL,               // addProperty
	NULL,               // delProperty
	NULL,               // getProperty
	NULL,               // setProperty
	NULL,               // enumerate
	NULL,               // resolve
    NULL,               // mayResolve
	cleanupStreamObject // finalize
}; // lStreamOps

/*! @brief The class of the stream class. */
static JSClass lStreamClass =
{
    "Stream",            // name
    JSCLASS_HAS_PRIVATE, // flags
	&lStreamOps
}; // lStreamClass

static void
cleanupStreamObject(JSFreeOp * freeOp,
                    JSObject * obj)
{
    ODL_ENTER(); //####
    if (&lStreamClass == JS_GetClass(obj))
    {
        FILE * aFile = reinterpret_cast<FILE *>(JS_GetPrivate(obj));

        if (aFile)
        {
            fclose(aFile);
            JS_SetPrivate(obj, NULL);
        }
    }
    ODL_EXIT(); //####
} // cleanupStreamObject

/*! @brief A C-callback function for %JavaScript to create a Stream object.
 @param[in] jct The context in which the native function is being called.
 @param[in] argc The number of arguments supplied to the function by the caller.
 @param[in] vp The arguments to the function.
 @returns @c true on success and @c false otherwise. */
static bool
createStreamObject(JSContext * jct,
                   unsigned    argc,
                   JS::Value * vp)
{
    bool         result = false;
    JS::CallArgs args = JS::CallArgsFromVp(argc, vp);

    if (args.length())
    {
        cerr << "Extra arguments to Stream constructor" << endl;
    }
    else
    {
        JSObject * obj = JS_NewObjectForConstructor(jct, &lStreamClass, args);

        if (obj)
        {
            JS_SetPrivate(obj, NULL);
            args.rval().setObject(*obj);
            result = true;
        }
    }
    return result;
} // createStreamObject

/*! @brief A C-callback function for %JavaScript to check if a Stream object is at EOF.
 @param[in] jct The context in which the native function is being called.
 @param[in] argc The number of arguments supplied to the function by the caller.
 @param[in] vp The arguments to the function.
 @returns @c true on success and @c false otherwise. */
static bool
streamAtEofForJs(JSContext * jct,
                 unsigned    argc,
                 JS::Value * vp)
{
    ODL_ENTER(); //####
    ODL_P2("jct = ", jct, "vp = ", vp); //####
    ODL_L1("argc = ", argc); //####
    bool         result = false;
    JS::CallArgs args = JS::CallArgsFromVp(argc, vp);
    JSObject &   theThis = args.thisv().toObject();

    if (args.length())
    {
        JS_ReportError(jct, "Extra arguments to Stream.atEof");
    }
    else
    {
        FILE * aFile = reinterpret_cast<FILE *>(JS_GetPrivate(&theThis));

        if (aFile)
        {
            args.rval().setBoolean(0 != feof(aFile));
        }
        else
        {
            // If it isn't open, treat it as if it was at EOF
            args.rval().setBoolean(true);
        }
        result = true;
    }
    ODL_EXIT_B(result); //####
    return result;
} // streamAtEofForJs

/*! @brief A C-callback function for %JavaScript to clear the error state of a Stream object.
 @param[in] jct The context in which the native function is being called.
 @param[in] argc The number of arguments supplied to the function by the caller.
 @param[in] vp The arguments to the function.
 @returns @c true on success and @c false otherwise. */
static bool
streamClearErrorForJs(JSContext * jct,
                      unsigned    argc,
                      JS::Value * vp)
{
    ODL_ENTER(); //####
    ODL_P2("jct = ", jct, "vp = ", vp); //####
    ODL_L1("argc = ", argc); //####
    bool         result = false;
    JS::CallArgs args = JS::CallArgsFromVp(argc, vp);
    JSObject &   theThis = args.thisv().toObject();

    if (args.length())
    {
        JS_ReportError(jct, "Extra arguments to Stream.clearError");
    }
    else
    {
        FILE * aFile = reinterpret_cast<FILE *>(JS_GetPrivate(&theThis));

        if (aFile)
        {
            clearerr(aFile);
        }
        result = true;
    }
    ODL_EXIT_B(result); //####
    return result;
} // streamClearErrorForJs

/*! @brief A C-callback function for %JavaScript to close a Stream object.
 @param[in] jct The context in which the native function is being called.
 @param[in] argc The number of arguments supplied to the function by the caller.
 @param[in] vp The arguments to the function.
 @returns @c true on success and @c false otherwise. */
static bool
streamCloseForJs(JSContext * jct,
                 unsigned    argc,
                 JS::Value * vp)
{
    ODL_ENTER(); //####
    ODL_P2("jct = ", jct, "vp = ", vp); //####
    ODL_L1("argc = ", argc); //####
    bool         result = false;
    JS::CallArgs args = JS::CallArgsFromVp(argc, vp);
    JSObject &   theThis = args.thisv().toObject();

    if (args.length())
    {
        JS_ReportError(jct, "Extra arguments to Stream.close");
    }
    else
    {
        FILE * aFile = reinterpret_cast<FILE *>(JS_GetPrivate(&theThis));

        if (aFile)
        {
            fclose(aFile);
            JS_SetPrivate(&theThis, NULL);
        }
        result = true;
    }
    ODL_EXIT_B(result); //####
    return result;
} // streamCloseForJs

/*! @brief A C-callback function for %JavaScript to check if a Stream object is in an error state.
 @param[in] jct The context in which the native function is being called.
 @param[in] argc The number of arguments supplied to the function by the caller.
 @param[in] vp The arguments to the function.
 @returns @c true on success and @c false otherwise. */
static bool
streamHasErrorForJs(JSContext * jct,
                    unsigned    argc,
                    JS::Value * vp)
{
    ODL_ENTER(); //####
    ODL_P2("jct = ", jct, "vp = ", vp); //####
    ODL_L1("argc = ", argc); //####
    bool         result = false;
    JS::CallArgs args = JS::CallArgsFromVp(argc, vp);
    JSObject &   theThis = args.thisv().toObject();

    if (args.length())
    {
        JS_ReportError(jct, "Extra arguments to Stream.hasError");
    }
    else
    {
        FILE * aFile = reinterpret_cast<FILE *>(JS_GetPrivate(&theThis));

        if (aFile)
        {
            args.rval().setBoolean(0 != feof(aFile));
        }
        else
        {
            // If it isn't open, treat it as if it was at EOF
            args.rval().setBoolean(true);
        }
        result = true;
    }
    ODL_EXIT_B(result); //####
    return result;
} // streamHasErrorForJs

/*! @brief A C-callback function for %JavaScript to check if a Stream object is open.
 @param[in] jct The context in which the native function is being called.
 @param[in] argc The number of arguments supplied to the function by the caller.
 @param[in] vp The arguments to the function.
 @returns @c true on success and @c false otherwise. */
static bool
streamIsOpenForJs(JSContext * jct,
                  unsigned    argc,
                  JS::Value * vp)
{
    ODL_ENTER(); //####
    ODL_P2("jct = ", jct, "vp = ", vp); //####
    ODL_L1("argc = ", argc); //####
    bool         result = false;
    JS::CallArgs args = JS::CallArgsFromVp(argc, vp);
    JSObject &   theThis = args.thisv().toObject();

    if (args.length())
    {
        JS_ReportError(jct, "Extra arguments to Stream.isOpen");
    }
    else
    {
        FILE * aFile = reinterpret_cast<FILE *>(JS_GetPrivate(&theThis));

        args.rval().setBoolean(NULL != aFile);
        result = true;
    }
    ODL_EXIT_B(result); //####
    return result;
} // streamIsOpenForJs

/*! @brief A C-callback function for %JavaScript to open a Stream object.
 @param[in] jct The context in which the native function is being called.
 @param[in] argc The number of arguments supplied to the function by the caller.
 @param[in] vp The arguments to the function.
 @returns @c true on success and @c false otherwise. */
static bool
streamOpenForJs(JSContext * jct,
                unsigned    argc,
                JS::Value * vp)
{
    ODL_ENTER(); //####
    ODL_P2("jct = ", jct, "vp = ", vp); //####
    ODL_L1("argc = ", argc); //####
    bool         result = false;
    JS::CallArgs args = JS::CallArgsFromVp(argc, vp);
    JSObject &   theThis = args.thisv().toObject();

    if (2 == args.length())
    {
        // Check if the Stream is already open and close it.
        FILE * aFile = reinterpret_cast<FILE *>(JS_GetPrivate(&theThis));

        if (aFile)
        {
            fclose(aFile);
            JS_SetPrivate(&theThis, NULL);
        }
        if (args[0].isString() && args[1].isString())
        {
            JSString * asString1 = args[0].toString();
            char *     asChars1 = JS_EncodeString(jct, asString1);
            JSString * asString2 = args[1].toString();
            char *     asChars2 = JS_EncodeString(jct, asString2);

            aFile = fopen(asChars1, asChars2);
            JS_free(jct, asChars1);
            JS_free(jct, asChars2);
            if (aFile)
            {
                JS_SetPrivate(&theThis, aFile);
                result = true;
            }
        }
    }
    else if (2 < args.length())
    {
        JS_ReportError(jct, "Extra arguments to Stream.open");
    }
    else
    {
        JS_ReportError(jct, "Missing argument(s) to Stream.open");
    }
    ODL_EXIT_B(result); //####
    return result;
} // streamOpenForJs

/*! @brief A C-callback function for %JavaScript to read the next non-blank character from a Stream
 object.
 @param[in] jct The context in which the native function is being called.
 @param[in] argc The number of arguments supplied to the function by the caller.
 @param[in] vp The arguments to the function.
 @returns @c true on success and @c false otherwise. */
static bool
streamReadCharacterForJs(JSContext * jct,
                         unsigned    argc,
                         JS::Value * vp)
{
    ODL_ENTER(); //####
    ODL_P2("jct = ", jct, "vp = ", vp); //####
    ODL_L1("argc = ", argc); //####
    bool         result = false;
    JS::CallArgs args = JS::CallArgsFromVp(argc, vp);
    JSObject &   theThis = args.thisv().toObject();

    if (args.length())
    {
        JS_ReportError(jct, "Extra arguments to Stream.readCharacter");
    }
    else
    {
        FILE * aFile = reinterpret_cast<FILE *>(JS_GetPrivate(&theThis));

        if (aFile)
        {
            char aChar = '\0';

            if (1 == fscanf(aFile, " %c", &aChar))
            {
                JSString * outString = JS_NewStringCopyN(jct, &aChar, 1);

                args.rval().setString(outString);
            }
            result = true;
        }
    }
    ODL_EXIT_B(result); //####
    return result;
} // streamReadCharacterForJs

/*! @brief A C-callback function for %JavaScript to read a line from a Stream object.
 @param[in] jct The context in which the native function is being called.
 @param[in] argc The number of arguments supplied to the function by the caller.
 @param[in] vp The arguments to the function.
 @returns @c true on success and @c false otherwise. */
static bool
streamReadLineForJs(JSContext * jct,
                    unsigned    argc,
                    JS::Value * vp)
{
    ODL_ENTER(); //####
    ODL_P2("jct = ", jct, "vp = ", vp); //####
    ODL_L1("argc = ", argc); //####
    bool         result = false;
    JS::CallArgs args = JS::CallArgsFromVp(argc, vp);
    JSObject &   theThis = args.thisv().toObject();

    if (args.length())
    {
        JS_ReportError(jct, "Extra arguments to Stream.readLine");
    }
    else
    {
        FILE * aFile = reinterpret_cast<FILE *>(JS_GetPrivate(&theThis));

        if (aFile)
        {
            bool       keepGoing = true;
            char       inBuffer[200];
            JSString * outString = JS_NewStringCopyZ(jct, "");

            for ( ; keepGoing; )
            {
                char * inPtr = fgets(inBuffer, sizeof(inBuffer), aFile);

                if (inPtr)
                {
                    JSString * thisChunk;
                    size_t     len = strlen(inBuffer);

                    if ('\n' == inBuffer[len - 1])
                    {
                        // We don't want to copy the newline into the buffer.
                        thisChunk = JS_NewStringCopyN(jct, inBuffer, len - 1);
                        keepGoing = false;
                    }
                    else
                    {
                        thisChunk = JS_NewStringCopyZ(jct, inBuffer);
                    }
                    JS::RootedString leftString(jct);
                    JS::RootedString rightString(jct);

                    leftString = outString;
                    rightString = thisChunk;
                    outString = JS_ConcatStrings(jct, leftString, rightString);
                }
                else
                {
                    keepGoing = false;
                }
            }
            args.rval().setString(outString);
            result = true;
        }
    }
    ODL_EXIT_B(result); //####
    return result;
} // streamReadLineForJs

/*! @brief A C-callback function for %JavaScript to read the next number from a Stream object.
 @param[in] jct The context in which the native function is being called.
 @param[in] argc The number of arguments supplied to the function by the caller.
 @param[in] vp The arguments to the function.
 @returns @c true on success and @c false otherwise. */
static bool
streamReadNumberForJs(JSContext * jct,
                      unsigned    argc,
                      JS::Value * vp)
{
    ODL_ENTER(); //####
    ODL_P2("jct = ", jct, "vp = ", vp); //####
    ODL_L1("argc = ", argc); //####
    bool         result = false;
    JS::CallArgs args = JS::CallArgsFromVp(argc, vp);
    JSObject &   theThis = args.thisv().toObject();

    if (args.length())
    {
        JS_ReportError(jct, "Extra arguments to Stream.readNumber");
    }
    else
    {
        FILE * aFile = reinterpret_cast<FILE *>(JS_GetPrivate(&theThis));

        if (aFile)
        {
            double aDouble = 0;

            if (1 == fscanf(aFile, " %lg", &aDouble))
            {
                args.rval().setDouble(aDouble);
            }
            result = true;
        }
    }
    ODL_EXIT_B(result); //####
    return result;
} // streamReadNumberForJs

/*! @brief A C-callback function for %JavaScript to read a string from a Stream object.
 @param[in] jct The context in which the native function is being called.
 @param[in] argc The number of arguments supplied to the function by the caller.
 @param[in] vp The arguments to the function.
 @returns @c true on success and @c false otherwise. */
static bool
streamReadStringForJs(JSContext * jct,
                      unsigned    argc,
                      JS::Value * vp)
{
    ODL_ENTER(); //####
    ODL_P2("jct = ", jct, "vp = ", vp); //####
    ODL_L1("argc = ", argc); //####
    bool         result = false;
    JS::CallArgs args = JS::CallArgsFromVp(argc, vp);
    JSObject &   theThis = args.thisv().toObject();

    if (args.length())
    {
        JS_ReportError(jct, "Extra arguments to Stream.readString");
    }
    else
    {
        FILE * aFile = reinterpret_cast<FILE *>(JS_GetPrivate(&theThis));

        if (aFile)
        {
            char aChar = '\0';

            if (1 == fscanf(aFile, " %c", &aChar))
            {
                bool       keepGoing = true;
                int        outLen = 0;
                char       outBuff[100];
                char       matchChar;
                JSString * outString = NULL;
                JSString * thisChunk = NULL;

                if (('"' == aChar) || ('\'' == aChar))
                {
                    matchChar = aChar;
                }
                else
                {
                    matchChar = '\0';
                    outBuff[0] = aChar;
                    outLen = 1;
                }
                for ( ; keepGoing; )
                {
                    aChar = fgetc(aFile);
                    if (EOF == aChar)
                    {
                        // Something happened.
                        keepGoing = false;
                    }
                    else if (matchChar == aChar)
                    {
                        // Reached the end-of-string.
                        keepGoing = false;
                    }
                    else if (outLen < sizeof(outBuff))
                    {
                        outBuff[outLen++] = aChar;
                    }
                    else
                    {
                        // The buffer is full.
                        if (outString)
                        {
                            // We need to concenate
                            thisChunk = JS_NewStringCopyN(jct, outBuff, outLen);
                            JS::RootedString leftString(jct);
                            JS::RootedString rightString(jct);

                            leftString = outString;
                            rightString = thisChunk;
                            outString = JS_ConcatStrings(jct, leftString, rightString);
                        }
                        else
                        {
                            // This is the first chunk
                            outString = JS_NewStringCopyN(jct, outBuff, outLen);
                        }
                        outLen = 0;
                    }
                }
                // Add the remaining characters to the string.
                if (outLen)
                {
                    if (outString)
                    {
                        // We need to concenate
                        thisChunk = JS_NewStringCopyN(jct, outBuff, outLen);
                        JS::RootedString leftString(jct);
                        JS::RootedString rightString(jct);

                        leftString = outString;
                        rightString = thisChunk;
                        outString = JS_ConcatStrings(jct, leftString, rightString);
                    }
                    else
                    {
                        // This is the first chunk
                        outString = JS_NewStringCopyN(jct, outBuff, outLen);
                    }
                }
                else if (! outString)
                {
                    outString = JS_NewStringCopyZ(jct, "");
                }
                args.rval().setString(outString);
            }
            result = true;
        }
    }
    ODL_EXIT_B(result); //####
    return result;
} // streamReadStringForJs

/*! @brief A C-callback function for %JavaScript to reposition a Stream object to its beginning.
 @param[in] jct The context in which the native function is being called.
 @param[in] argc The number of arguments supplied to the function by the caller.
 @param[in] vp The arguments to the function.
 @returns @c true on success and @c false otherwise. */
static bool
streamRewindForJs(JSContext * jct,
                  unsigned    argc,
                  JS::Value * vp)
{
    ODL_ENTER(); //####
    ODL_P2("jct = ", jct, "vp = ", vp); //####
    ODL_L1("argc = ", argc); //####
    bool         result = false;
    JS::CallArgs args = JS::CallArgsFromVp(argc, vp);
    JSObject &   theThis = args.thisv().toObject();

    if (args.length())
    {
        JS_ReportError(jct, "Extra arguments to Stream.close");
    }
    else
    {
        FILE * aFile = reinterpret_cast<FILE *>(JS_GetPrivate(&theThis));

        if (aFile)
        {
            rewind(aFile);
        }
        result = true;
    }
    ODL_EXIT_B(result); //####
    return result;
} // streamRewindForJs

/*! @brief A C-callback function for %JavaScript to write a value to a Stream object.
 @param[in] jct The context in which the native function is being called.
 @param[in] argc The number of arguments supplied to the function by the caller.
 @param[in] vp The arguments to the function.
 @returns @c true on success and @c false otherwise. */
static bool
streamWriteForJs(JSContext * jct,
                 unsigned    argc,
                 JS::Value * vp)
{
    ODL_ENTER(); //####
    ODL_P2("jct = ", jct, "vp = ", vp); //####
    ODL_L1("argc = ", argc); //####
    bool         result;
    JS::CallArgs args = JS::CallArgsFromVp(argc, vp);
    JSObject &   theThis = args.thisv().toObject();
    FILE *       aFile = reinterpret_cast<FILE *>(JS_GetPrivate(&theThis));

    if (aFile)
    {
        for (int ii = 0, mm = args.length(); mm > ii; ++ii)
        {
            JSString * asString = JS::ToString(jct, args[ii]);

            if (asString && JS_GetStringLength(asString))
            {
                char * asChars = JS_EncodeString(jct, asString);

                fputs(asChars, aFile);
                JS_free(jct, asChars);
            }
        }
        result = true;
    }
    else
    {
        result = false;
    }
    ODL_EXIT_B(result); //####
    return result;
} // streamWriteForJs

/*! @brief A C-callback function for %JavaScript to write a value to a Stream object,
 followed by a newline.
 @param[in] jct The context in which the native function is being called.
 @param[in] argc The number of arguments supplied to the function by the caller.
 @param[in] vp The arguments to the function.
 @returns @c true on success and @c false otherwise. */
static bool
streamWriteLineForJs(JSContext * jct,
                     unsigned    argc,
                     JS::Value * vp)
{
    ODL_ENTER(); //####
    ODL_P2("jct = ", jct, "vp = ", vp); //####
    ODL_L1("argc = ", argc); //####
    bool         result;
    JS::CallArgs args = JS::CallArgsFromVp(argc, vp);
    JSObject &   theThis = args.thisv().toObject();
    FILE *       aFile = reinterpret_cast<FILE *>(JS_GetPrivate(&theThis));

    if (aFile)
    {
        for (int ii = 0, mm = args.length(); mm > ii; ++ii)
        {
            JSString * asString = JS::ToString(jct, args[ii]);

            if (asString && JS_GetStringLength(asString))
            {
                char * asChars = JS_EncodeString(jct, asString);

                fputs(asChars, aFile);
                JS_free(jct, asChars);
            }
        }
        fputc('\n', aFile);
        fflush(aFile);
        result = true;
    }
    else
    {
        result = false;
    }
    ODL_EXIT_B(result); //####
    return result;
} // streamWriteLineForJs

/*! @brief The table of supplied functions for the %Stream class. */
static const JSFunctionSpec lStreamFunctions[] =
{
    JS_FS("atEof", streamAtEofForJs, 0, JSPROP_ENUMERATE),
    JS_FS("clearError", streamClearErrorForJs, 0, JSPROP_ENUMERATE),
    JS_FS("close", streamCloseForJs, 0, JSPROP_ENUMERATE),
    JS_FS("hasError", streamHasErrorForJs, 0, JSPROP_ENUMERATE),
    JS_FS("isOpen", streamIsOpenForJs, 0, JSPROP_ENUMERATE),
    JS_FS("open", streamOpenForJs, 2, JSPROP_ENUMERATE),
    JS_FS("readCharacter", streamReadCharacterForJs, 0, JSPROP_ENUMERATE),
    JS_FS("readLine", streamReadLineForJs, 0, JSPROP_ENUMERATE),
    JS_FS("readNumber", streamReadNumberForJs, 0, JSPROP_ENUMERATE),
    JS_FS("readString", streamReadStringForJs, 0, JSPROP_ENUMERATE),
    JS_FS("rewind", streamRewindForJs, 0, JSPROP_ENUMERATE),
    JS_FS("write", streamWriteForJs, 1, JSPROP_ENUMERATE),
    JS_FS("writeLine", streamWriteLineForJs, 1, JSPROP_ENUMERATE),
    JS_FS_END
}; // lStreamFunctions

/*! @brief Add custom classes to the %JavaScript environment.
 @param[in] jct The %JavaScript engine context.
 @param[in,out] global The %JavaScript global object.
 @returns @c true if the custom classes were addeded successfully and @c false otherwise. */
static bool
addCustomClasses(JSContext *        jct,
                 JS::RootedObject & global)
{
    ODL_ENTER(); //####
    ODL_P2("jct = ", jct, "global = ", &global); //####
    bool okSoFar = false;

#if (40 < MOZJS_MAJOR_VERSION)
    if (JS_InitClass(jct, global, NULL, &lStreamClass, createStreamObject, 0, NULL,
                     lStreamFunctions, NULL, NULL))
#else // 40 >= MOZJS_MAJOR_VERSION
    if (JS_InitClass(jct, global, JS::NullPtr(), &lStreamClass, createStreamObject, 0, NULL,
                     lStreamFunctions, NULL, NULL))
#endif // 40 >= MOZJS_MAJOR_VERSION
    {
        okSoFar = true;
    }
    ODL_EXIT_B(okSoFar); //####
    return okSoFar;
} // addCustomClasses

/*! @brief Add an array containing the command-line arguments to the %JavaScript environment.
 @param[in] jct The %JavaScript engine context.
 @param[in,out] global The %JavaScript global object.
 @param[in] argv The arguments to be used with the %JavaScript filter service.
 @returns @c true if the arrays wss addeded successfully and @c false otherwise. */
static bool
addArgvObject(JSContext *              jct,
              JS::RootedObject &       global,
              const YarpStringVector & argv)
{
    ODL_ENTER(); //####
    ODL_P3("jct = ", jct, "global = ", &global, "argv = ", &argv); //####
    bool       okSoFar = true;
    JSObject * argArray = JS_NewArrayObject(jct, 0);

    if (argArray)
    {
        JS::RootedObject argObject(jct);
        JS::RootedValue  argValue(jct);

        argObject = argArray;
        argValue.setObject(*argArray);
        if (JS_SetProperty(jct, global, "argv", argValue))
        {
            char *          endPtr;
            size_t          argc = argv.size();
            int32_t         tempInt;
            JS::RootedValue anElement(jct);
            JS::RootedId    aRootedId(jct);

            for (size_t ii = 0; okSoFar && (argc > ii); ++ii)
            {
                const char * anArg = argv[ii].c_str();

                // Check for an integer value
                tempInt = static_cast<int32_t>(strtol(anArg, &endPtr, 10));
                if ((anArg == endPtr) || *endPtr)
                {
                    // Check for an floating-point value
                    double tempDouble = strtod(anArg, &endPtr);

                    if ((anArg == endPtr) || *endPtr)
                    {
                        // Otherwise, treat as a string
                        JSString * aString = JS_NewStringCopyZ(jct, anArg);

                        if (aString)
                        {
                            anElement.setString(aString);
                        }
                        else
                        {
                            okSoFar = false;
                        }
                    }
                    else
                    {
                        anElement.setDouble(tempDouble);
                    }
                }
                else
                {
                    anElement.setInt32(tempInt);
                }
                if (okSoFar)
                {
                    if (JS_IndexToId(jct, static_cast<uint32_t>(ii), &aRootedId))
                    {
                        JS_SetPropertyById(jct, argObject, aRootedId, anElement);
                    }
                    else
                    {
                        okSoFar = false;
                    }
                }
            }
        }
        else
        {
            okSoFar = false;
        }
    }
    else
    {
        okSoFar = false;
    }
    ODL_EXIT_B(okSoFar); //####
    return okSoFar;
} // addArgvObject

/*! @brief Add a custom string object to the %JavaScript environment.
 @param[in] jct The %JavaScript engine context.
 @param[in,out] global The %JavaScript global object.
 @param[in] tag The modifier for the service name and port names.
 @returns @c true if the custom string object was addeded successfully and @c false otherwise. */
static bool
addScriptTagObject(JSContext *        jct,
                   JS::RootedObject & global,
                   const YarpString & tag)
{
    ODL_ENTER(); //####
    ODL_P2("jct = ", jct, "global = ", &global); //####
    ODL_S1s("tag = ", tag); //####
    bool       okSoFar = true;
    JSString * aString = JS_NewStringCopyZ(jct, tag.c_str());

    if (aString)
    {
        JS::RootedValue argValue(jct);

        argValue.setString(aString);
        if (! JS_SetProperty(jct, global, "scriptTag", argValue))
        {
            okSoFar = false;
        }
    }
    else
    {
        okSoFar = false;
    }
    ODL_EXIT_B(okSoFar); //####
    return okSoFar;
} // addScriptTagObject

/*! @brief Add custom classes, functions and variables to the %JavaScript environment.
 @param[in] jct The %JavaScript engine context.
 @param[in,out] global The %JavaScript global object.
 @param[in] tag The modifier for the service name and port names.
 @param[in] argv The arguments to be used with the %JavaScript filter service.
 @returns @c true if the custom objects were addeded successfully and @c false otherwise. */
static bool
addCustomObjects(JSContext *              jct,
                 JS::RootedObject &       global,
                 const YarpString &       tag,
                 const YarpStringVector & argv)
{
    ODL_ENTER(); //####
    ODL_P3("jct = ", jct, "global = ", &global, "argv = ", &argv); //####
    ODL_S1s("tag = ", tag); //####
    bool okSoFar = addCustomFunctions(jct, global);

    if (okSoFar)
    {
        okSoFar = addCustomClasses(jct, global);
    }
    if (okSoFar)
    {
        okSoFar = addArgvObject(jct, global, argv);
    }
    if (okSoFar)
    {
        okSoFar = addScriptTagObject(jct, global, tag);
    }
    ODL_EXIT_B(okSoFar); //####
    return okSoFar;
} // addCustomObjects

/*! @brief Load a script into the %JavaScript environment.
 @param[in] jct The %JavaScript engine context.
 @param[in,out] options The compile options used to retain the compiled script.
 @param[in] script The %JavaScript source code to be executed.
 @param[in] scriptPath The path to the script file.
 @returns @c true on success and @c false otherwise. */
static bool
loadScript(JSContext *                jct,
           JS::OwningCompileOptions & options,
           const YarpString &         script,
           const YarpString &         scriptPath)
{
    ODL_ENTER();
    ODL_P1("jct = ", jct); //####
    ODL_S1s("scriptPath = ", scriptPath); //####
    bool            okSoFar;
    JS::RootedValue result(jct);

    options.setFileAndLine(jct, scriptPath.c_str(), 1);
    // We can ignore the returned result, since we are only interested in setting up the functions
    // and variables in the environment. The documentation states that NULL can be passed as the
    // last argument, but compiles fail if this is done.
    okSoFar = JS::Evaluate(jct, options, script.c_str(), script.size(), &result);
    ODL_EXIT_B(okSoFar);
    return okSoFar;
} // loadScript

/*! @brief Check an object for a specific numeric property.
 @param[in] jct The %JavaScript engine context.
 @param[in] anObject The object to check.
 @param[in] propertyName The name of the property being searched for.
 @param[in] canBeFunction @c true if the property can be a function rather than a string and
 @c false if the property must be a string.
 @param[in] isOptional @c true if the property does not have to be present.
 @param[out] result The value of the number, if located.
 @returns @c true on success and @c false otherwise. */
static bool
getLoadedDouble(JSContext *        jct,
                JS::RootedObject & anObject,
                const char *       propertyName,
                const bool         canBeFunction,
                const bool         isOptional,
                double &           result)
{
    ODL_ENTER(); //####
    ODL_P3("jct = ", jct, "anObject = ", &anObject, "result = ", &result); //####
    ODL_S1("propertyName = ", propertyName); //####
    ODL_B2("canBeFunction = ", canBeFunction, "isOptional = ", isOptional); //####
    bool found = false;
    bool okSoFar;

    result = 0;
    if (JS_HasProperty(jct, anObject, propertyName, &found))
    {
        okSoFar = true;
    }
    else if (isOptional)
    {
        okSoFar = true;
    }
    else
    {
        ODL_LOG("! (JS_HasProperty(jct, anObject, propertyName, &found))"); //####
        okSoFar = false;
        MpM_FAIL_("Problem searching for a property.");
    }
    if (okSoFar && found)
    {
        JS::RootedValue value(jct);

        if (JS_GetProperty(jct, anObject, propertyName, &value))
        {
            okSoFar = false;
            if (value.isNumber())
            {
                result = value.toNumber();
                okSoFar = true;
            }
            else if (canBeFunction)
            {
                if (value.isObject())
                {
                    JS::RootedObject asObject(jct);

                    if (JS_ValueToObject(jct, value, &asObject))
                    {
                        if (JS_ObjectIsFunction(jct, asObject))
                        {
                            JS::HandleValueArray funcArgs(JS::HandleValueArray::empty());
                            JS::RootedValue      funcResult(jct);

                            JS_BeginRequest(jct);
                            if (JS_CallFunctionValue(jct, anObject, value, funcArgs, &funcResult))
                            {
                                if (funcResult.isNumber())
                                {
                                    result = funcResult.toNumber();
                                    okSoFar = true;
                                }
                            }
                            else
                            {
                                ODL_LOG("! (JS_CallFunctionValue(jct, anObject, value, " //####
                                        "funcArgs, &funcResult))"); //####
                                JS::RootedValue exc(jct);

                                if (JS_GetPendingException(jct, &exc))
                                {
                                    JS_ClearPendingException(jct);
                                    YarpString message("Exception occurred while executing "
                                                       "function for Property '");

                                    message += propertyName;
                                    message += "'.";
                                    MpM_FAIL_(message.c_str());
                                }
                            }
                            JS_EndRequest(jct);
                        }
                    }
                }
            }
            if (! okSoFar)
            {
                ODL_LOG("! (okSoFar)"); //####
                okSoFar = false;
                YarpString message("Property '");

                message += propertyName;
                message += "' has the wrong type.";
                MpM_FAIL_(message.c_str());
            }
        }
        else
        {
            ODL_LOG("! (JS_GetProperty(jct, anObject, propertyName, &value))"); //####
            okSoFar = false;
            MpM_FAIL_("Problem retrieving a property.");
        }
    }
    ODL_EXIT_B(okSoFar); //####
    return okSoFar;
} // getLoadedDouble

/*! @brief Check an object for a specific string property.
 @param[in] jct The %JavaScript engine context.
 @param[in] anObject The object to check.
 @param[in] propertyName The name of the property being searched for.
 @param[in] canBeFunction @c true if the property can be a function rather than a string and
 @c false if the property must be a string.
 @param[in] isOptional @c true if the property does not have to be present.
 @param[out] result The value of the string, if located.
 @returns @c true on success and @c false otherwise. */
static bool
getLoadedString(JSContext *        jct,
                JS::RootedObject & anObject,
                const char *       propertyName,
                const bool         canBeFunction,
                const bool         isOptional,
                YarpString &       result)
{
    ODL_ENTER(); //####
    ODL_P3("jct = ", jct, "anObject = ", &anObject, "result = ", &result); //####
    ODL_S1("propertyName = ", propertyName); //####
    ODL_B2("canBeFunction = ", canBeFunction, "isOptional = ", isOptional); //####
    bool found = false;
    bool okSoFar;

    result = "";
    if (JS_HasProperty(jct, anObject, propertyName, &found))
    {
        okSoFar = true;
    }
    else if (isOptional)
    {
        okSoFar = true;
    }
    else
    {
        ODL_LOG("! (JS_HasProperty(jct, anObject, propertyName, &found))"); //####
        okSoFar = false;
        MpM_FAIL_("Problem searching for a property.");
    }
    if (okSoFar && found)
    {
        JS::RootedValue value(jct);

        if (JS_GetProperty(jct, anObject, propertyName, &value))
        {
            okSoFar = false;
            if (value.isString())
            {
                JSString * asString = value.toString();
                char *     asChars = JS_EncodeString(jct, asString);

                result = asChars;
                JS_free(jct, asChars);
                okSoFar = true;
            }
            else if (canBeFunction)
            {
                if (value.isObject())
                {
                    JS::RootedObject asObject(jct);

                    if (JS_ValueToObject(jct, value, &asObject))
                    {
                        if (JS_ObjectIsFunction(jct, asObject))
                        {
                            JS::HandleValueArray funcArgs(JS::HandleValueArray::empty());
                            JS::RootedValue      funcResult(jct);

                            JS_BeginRequest(jct);
                            if (JS_CallFunctionValue(jct, anObject, value, funcArgs, &funcResult))
                            {
                                if (funcResult.isString())
                                {
                                    JSString * asString = funcResult.toString();
                                    char *     asChars = JS_EncodeString(jct, asString);

                                    result = asChars;
                                    JS_free(jct, asChars);
                                    okSoFar = true;
                                }
                            }
                            else
                            {
                                ODL_LOG("! (JS_CallFunctionValue(jct, anObject, value, " //####
                                        "funcArgs, &funcResult))"); //####
                                JS::RootedValue exc(jct);

                                if (JS_GetPendingException(jct, &exc))
                                {
                                    JS_ClearPendingException(jct);
                                    YarpString message("Exception occurred while executing "
                                                       "function for Property '");

                                    message += propertyName;
                                    message += "'.";
                                    MpM_FAIL_(message.c_str());
                                }
                            }
                            JS_EndRequest(jct);
                        }
                    }
                }
            }
            if (! okSoFar)
            {
                ODL_LOG("! (okSoFar)"); //####
                okSoFar = false;
                YarpString message("Property '");

                message += propertyName;
                message += "' has the wrong type.";
                MpM_FAIL_(message.c_str());
            }
        }
        else
        {
            ODL_LOG("! (JS_GetProperty(jct, anObject, propertyName, &value))"); //####
            okSoFar = false;
            MpM_FAIL_("Problem retrieving a property.");
        }
    }
    ODL_EXIT_B(okSoFar); //####
    return okSoFar;
} // getLoadedString

/*! @brief Check an object for a specific function property.
 @param[in] jct The %JavaScript engine context.
 @param[in] anObject The object to check.
 @param[in] propertyName The name of the property being searched for.
 @param[in] arity The required arity for the function.
 @param[out] result The value of the function, if located.
 @returns @c true on success and @c false otherwise. */
static bool
getLoadedFunctionRef(JSContext *        jct,
                     JS::RootedObject & anObject,
                     const char *       propertyName,
                     const uint32_t     arity,
                     JS::RootedValue &  result)
{
    ODL_ENTER(); //####
    ODL_P3("jct = ", jct, "anObject = ", &anObject, "result = ", &result); //####
    ODL_S1("propertyName = ", propertyName); //####
    ODL_L1("arity = ", arity); //####
    bool found = false;
    bool okSoFar;

    if (JS_HasProperty(jct, anObject, propertyName, &found))
    {
        okSoFar = found;
    }
    else
    {
        ODL_LOG("! (JS_HasProperty(jct, anObject, propertyName, &found))"); //####
        okSoFar = false;
        MpM_FAIL_("Problem searching for a property.");
    }
    if (okSoFar)
    {
        if (JS_GetProperty(jct, anObject, propertyName, &result))
        {
            okSoFar = false;
            if (result.isObject())
            {
                JS::RootedObject asObject(jct);

                if (JS_ValueToObject(jct, result, &asObject) &&
                    JS_ObjectIsFunction(jct, asObject) && JS::IsCallable(asObject))
                {
                    JSFunction * asFunction = JS_ValueToFunction(jct, result);

                    if (asFunction)
                    {
                        okSoFar = (arity == JS_GetFunctionArity(asFunction));
                    }
                }
            }
            if (! okSoFar)
            {
                ODL_LOG("! (okSoFar)"); //####
                okSoFar = false;
                YarpString message("Property '");

                message += propertyName;
                message += "' has the wrong type.";
                MpM_FAIL_(message.c_str());
            }
        }
        else
        {
            ODL_LOG("! (JS_GetProperty(jct, anObject, propertyName, &result))"); //####
            okSoFar = false;
            MpM_FAIL_("Problem retrieving a property.");
        }
    }
    ODL_EXIT_B(okSoFar); //####
    return okSoFar;
} // getLoadedFunctionRef

/*! @brief Check a stream description.
 @param[in] jct The %JavaScript engine context.
 @param[in] anElement The stream description object to be checked.
 @param[in] inletHandlers non-@c NULL if there must be a handler for the stream description.
 @param[out] description The validated stream description.
 @returns @c true on success and @c false otherwise. */
static bool
processStreamDescription(JSContext *           jct,
                         JS::RootedValue &     anElement,
                         JS::AutoValueVector * inletHandlers,
                         ChannelDescription &  description)
{
    ODL_ENTER(); //####
    ODL_P4("jct = ", jct, "anElement = ", &anElement, "inletHandlers = ", inletHandlers, //####
           "description = ", &description); //####
    bool okSoFar = true;

    if (! anElement.isObject())
    {
        ODL_LOG("(! anElement.isObject())"); //####
        okSoFar = false;
        MpM_FAIL_("Array element has the wrong type.");
    }
    JS::RootedObject asObject(jct);

    if (okSoFar)
    {
        if (! JS_ValueToObject(jct, anElement, &asObject))
        {
            ODL_LOG("(! JS_ValueToObject(jct, anElement, &asObject))"); //####
            okSoFar = false;
            MpM_FAIL_("Problem converting array element to object.");
        }
    }
    if (okSoFar)
    {
//        PrintJavaScriptObject(cout, jct, asObject, 0);
        okSoFar = getLoadedString(jct, asObject, "name", false, false, description._portName);
    }
    if (okSoFar)
    {
        okSoFar = getLoadedString(jct, asObject, "protocol", false, false,
                                  description._portProtocol);
    }
    if (okSoFar)
    {
        okSoFar = getLoadedString(jct, asObject, "protocolDescription", false, false,
                                  description._protocolDescription);
    }
    if (okSoFar && inletHandlers)
    {
        JS::RootedValue result(jct);

        okSoFar = getLoadedFunctionRef(jct, asObject, "handler", 2, result);
        if (okSoFar)
        {
            inletHandlers->append(result);
        }
    }
    ODL_EXIT_B(okSoFar); //####
    return okSoFar;
} // processStreamDescription

/*! @brief Check the %JavaScript environment for a specific array variable containing stream
 descriptions.
 @param[in] jct The %JavaScript engine context.
 @param[in] global The %JavaScript global object.
 @param[in] arrayName The name of the array variable being searched for.
 @param[in] inletHandlers non-@c NULL if there must be a handler for each stream description.
 @param[out] streamDescriptions The list of loaded stream descriptions.
 @returns @c true on success and @c false otherwise. */
static bool
getLoadedStreamDescriptions(JSContext *           jct,
                            JS::RootedObject &    global,
                            const char *          arrayName,
                            JS::AutoValueVector * inletHandlers,
                            ChannelVector &       streamDescriptions)
{
    ODL_ENTER(); //####
    ODL_P4("jct = ", jct, "global = ", &global, "inletHandlers = ", inletHandlers, //####
           "streamDescriptions = ", &streamDescriptions); //####
    ODL_S1("arrayName = ", arrayName); //####
    bool found = false;
    bool okSoFar;

    streamDescriptions.clear();
    if (JS_HasProperty(jct, global, arrayName, &found))
    {
        okSoFar = true;
    }
    else
    {
        ODL_LOG("! (JS_HasProperty(jct, global, arrayName, &found))"); //####
        okSoFar = false;
        MpM_FAIL_("Problem searching for a global property.");
    }
    if (okSoFar && found)
    {
        JS::RootedValue  value(jct);
        JS::RootedObject asObject(jct);

        if (JS_GetProperty(jct, global, arrayName, &value))
        {
            okSoFar = false;
            if (value.isObject())
            {
                if (JS_ValueToObject(jct, value, &asObject))
                {
                    okSoFar = true;
                }
                else
                {
                    ODL_LOG("(! JS_ValueToObject(jct, value, &asObject))"); //####
                    MpM_FAIL_("Problem converting value to object.");
                }
            }
            if (okSoFar)
            {
                if (JS_ObjectIsFunction(jct, asObject))
                {
                    JS::HandleValueArray funcArgs(JS::HandleValueArray::empty());
                    JS::RootedValue      funcResult(jct);

                    okSoFar = false;
                    JS_BeginRequest(jct);
                    if (JS_CallFunctionValue(jct, global, value, funcArgs, &funcResult))
                    {
                        if (funcResult.isObject())
                        {
                            if (JS_ValueToObject(jct, funcResult, &asObject))
                            {
                                value = funcResult;
                                okSoFar = true;
                            }
                            else
                            {
                                ODL_LOG("(! JS_ValueToObject(jct, funcResult, &asObject))"); //####
                                MpM_FAIL_("Problem converting value to object.");
                            }
                        }
                    }
                    else
                    {
                        ODL_LOG("! (JS_CallFunctionValue(jct, global, value, funcArgs, " //####
                                "&funcResult))"); //####
                        JS::RootedValue exc(jct);

                        if (JS_GetPendingException(jct, &exc))
                        {
                            JS_ClearPendingException(jct);
                            YarpString message("Exception occurred while executing function for "
                                               "Property '");

                            message += arrayName;
                            message += "'.";
                            MpM_FAIL_(message.c_str());
                        }
                    }
                    JS_EndRequest(jct);
                }
            }
            if (! okSoFar)
            {
                ODL_LOG("(! okSoFar)"); //####
                YarpString message("Property '");

                message += arrayName;
                message += "' has the wrong type.";
                MpM_FAIL_(message.c_str());
            }
        }
        else
        {
            ODL_LOG("! (JS_GetProperty(jct, global, arrayName, &value))"); //####
            okSoFar = false;
            MpM_FAIL_("Problem retrieving a global property.");
        }
        uint32_t arrayLength;

        if (okSoFar)
        {
            if (! JS_GetArrayLength(jct, asObject, &arrayLength))
            {
                ODL_LOG("(! JS_GetArrayLength(jct, asObject, &arrayLength))"); //####
                okSoFar = false;
                MpM_FAIL_("Problem getting the array length.");
            }
        }
        if (okSoFar)
        {
            for (uint32_t ii = 0; okSoFar && (arrayLength > ii); ++ii)
            {
                JS::RootedValue anElement(jct);

                if (JS_GetElement(jct, asObject, ii, &anElement))
                {
                    ChannelDescription description;

                    okSoFar = processStreamDescription(jct, anElement, inletHandlers, description);
                    if (okSoFar)
                    {
                        streamDescriptions.push_back(description);
                    }
                }
                else
                {
                    ODL_LOG("! (JS_GetElement(jct, asObject, ii, &anElement))"); //####
                    okSoFar = false;
                    MpM_FAIL_("Problem getting an array element.");
                }
            }
        }
    }
    ODL_EXIT_B(okSoFar); //####
    return okSoFar;
} // getLoadedStreamDescriptions

/*! @brief Check the %JavaScript environment after loading a script.
 @param[in] jct The %JavaScript engine context.
 @param[in,out] global The %JavaScript global object.
 @param[out] sawThread @c true if a thread function was defined.
 @param[out] description The descriptive text from the script.
 @param[out] helpString The help text from the script.
 @param[out] loadedInletDescriptions The list of loaded inlet stream descriptions.
 @param[out] loadedOutletDescriptions The list of loaded outlet stream descriptions.
 @param[out] loadedInletHandlers The list of loaded inlet handlers.
 @param[out] loadedStartingFunction The function to execute on starting the service streams.
 @param[out] loadedStoppingFunction The function to execute on stopping the service streams.
 @param[out] loadedThreadFunction The function to execute on an output-generating thread.
 @param[out] loadedInterval The interval (in seconds) between executions of the output-generating
 thread.
 @returns @c true on success and @c false otherwise. */
static bool
validateLoadedScript(JSContext *           jct,
                     JS::RootedObject &    global,
                     bool &                sawThread,
                     YarpString &          description,
                     YarpString &          helpString,
                     ChannelVector &       loadedInletDescriptions,
                     ChannelVector &       loadedOutletDescriptions,
                     JS::AutoValueVector & loadedInletHandlers,
                     JS::RootedValue &     loadedStartingFunction,
                     JS::RootedValue &     loadedStoppingFunction,
                     JS::RootedValue &     loadedThreadFunction,
                     double &              loadedInterval)
{
    ODL_ENTER();
    ODL_P4("jct = ", jct, "global = ", &global, "sawThread = ", &sawThread, //####
           "description = ", &description); //####
    ODL_P4("helpString = ", &helpString, "loadedInletDescriptions = ", //####
           &loadedInletDescriptions, "loadedOutletDescriptions = ", //####
           &loadedOutletDescriptions, "loadedInletHandlers = ", &loadedInletHandlers); //####
    ODL_P4("loadedStartingFunction = ", &loadedStartingFunction, //####
           "loadedStoppingFunction = ", &loadedStoppingFunction, //####
           "loadedThreadFunction = ", &loadedThreadFunction, "loadedInterval = ", //####
           &loadedInterval); //####
    bool okSoFar;

//    PrintJavaScriptObject(cout, jct, global, 0);
    sawThread = false;
    loadedInterval = 1.0;
    loadedThreadFunction = JS::NullValue();
    okSoFar = getLoadedString(jct, global, "scriptDescription", true, false, description);
    if (okSoFar)
    {
        okSoFar = getLoadedString(jct, global, "scriptHelp", false, true, helpString);
    }
    if (okSoFar)
    {
        if (getLoadedFunctionRef(jct, global, "scriptThread", 0, loadedThreadFunction))
        {
//            cout << "function scriptThread defined" << endl;
            sawThread = true;
        }
    }
    if (okSoFar && (! sawThread))
    {
        okSoFar = getLoadedStreamDescriptions(jct, global, "scriptInlets", &loadedInletHandlers,
                                              loadedInletDescriptions);
    }
    if (okSoFar)
    {
        okSoFar = getLoadedStreamDescriptions(jct, global, "scriptOutlets", NULL,
                                              loadedOutletDescriptions);
    }
    if (okSoFar)
    {
        loadedStartingFunction = JS::NullValue();
        loadedStoppingFunction = JS::NullValue();
        if (getLoadedFunctionRef(jct, global, "scriptStarting", 0, loadedStartingFunction))
        {
//            cout << "function scriptStarting defined" << endl;
        }
        if (getLoadedFunctionRef(jct, global, "scriptStopping", 0, loadedStoppingFunction))
        {
//            cout << "function scriptStopping defined" << endl;
        }
    }
    if (okSoFar && sawThread)
    {
        okSoFar = getLoadedDouble(jct, global, "scriptInterval", true, true, loadedInterval);
    }
    ODL_EXIT_B(okSoFar);
    return okSoFar;
} // validateLoadedScript

/*! @brief Set up the environment and start the %JavaScript filter service.
 @param[in] argumentList Descriptions of the arguments to the executable.
 @param[in,out] scriptPath The script file to be processed.
 @param[in] arguments The arguments for the service.
 @param[in] progName The path to the executable.
 @param[in] argc The number of arguments in 'argv'.
 @param[in] argv The arguments to be used with the %JavaScript filter service.
 @param[in,out] tag The modifier for the service name and port names.
 @param[in,out] serviceEndpointName The YARP name to be assigned to the new service.
 @param[in] servicePortNumber The port being used by the service.
 @param[in] goWasSet @c true if the service is to be started immediately.
 @param[in] nameWasSet @c true if the endpoint name was set and @c false otherwise.
 @param[in] reportOnExit @c true if service metrics are to be reported on exit and @c false
 otherwise.
 @param[in] stdinAvailable @c true if running in the foreground and @c false otherwise. */
static void
setUpAndGo(const Utilities::DescriptorVector & argumentList,
           YarpString &                        scriptPath,
           const YarpStringVector &            arguments,
           const YarpString &                  progName,
           const int                           argc,
           char * *                            argv,
           YarpString &                        tag,
           YarpString &                        serviceEndpointName,
           const YarpString &                  servicePortNumber,
           const bool                          goWasSet,
           const bool                          nameWasSet,
           const bool                          reportOnExit,
           const bool                          stdinAvailable)
{
    ODL_ENTER(); //####
    ODL_P3("argumentList = ", &argumentList, "arguments = ", &arguments, "argv = ", argv); //####
    ODL_S4s("scriptPath = ", scriptPath, "progName = ", progName, "tag = ", tag, //####
            "serviceEndpointName = ", serviceEndpointName); //####
    ODL_S1s("servicePortNumber = ", servicePortNumber); //####
    ODL_LL1("argc = ", argc); //####
    ODL_B4("goWasSet = ", goWasSet, "nameWasSet = ", nameWasSet, "reportOnExit = ", //####
           reportOnExit, "stdinAvailable = ", stdinAvailable); //####
    YarpString scriptSource;

    // Make sure that the scriptPath is valid.
    FILE * scratch = fopen(scriptPath.c_str(), "r");

    if (scratch)
    {
        // The path given is a readable file, so read it in and prepare to start the service by
        // filling in the scriptSource.
        char   buffer[10240];
        size_t numRead;

        for ( ; ! feof(scratch); )
        {
            numRead = fread(buffer, 1, sizeof(buffer) - 1, scratch);
            if (numRead)
            {
                buffer[numRead] = '\0';
                scriptSource += buffer;
            }
        }
        fclose(scratch);
    }
    if (0 < scriptSource.length())
    {
        if (JS_Init())
        {
            JSContext * jct = NULL;
            ODL_LOG("creating the JavaScript runtime"); //####
            JSRuntime * jrt = JS_NewRuntime(JAVASCRIPT_GC_SIZE_ * 1024L * 1024L);

            if (jrt)
            {
                JS_SetErrorReporter(jrt, reportJavaScriptError);
#if (40 >= MOZJS_MAJOR_VERSION)
                // Avoid ambiguity between 'var x = ...' and 'x = ...'.
                JS::RuntimeOptionsRef(jrt).setVarObjFix(true);
#endif // 40 >= MOZJS_MAJOR_VERSION
                JS::RuntimeOptionsRef(jrt).setExtraWarnings(true);
                ODL_LOG("creating the JavaScript context"); //####
                jct = JS_NewContext(jrt, JAVASCRIPT_STACKCHUNK_SIZE_);
                if (jct)
                {
                    JS::ContextOptionsRef(jct).setDontReportUncaught(true);
                    JS::ContextOptionsRef(jct).setAutoJSAPIOwnsErrorReporting(true);
                }
                else
                {
                    ODL_LOG("! (jct)"); //####
                    MpM_FAIL_("JavaScript context could not be allocated.");
                    ODL_LOG("destroying the JavaScript runtime"); //####
                    JS_DestroyRuntime(jrt);
                    jrt = NULL;
                }
            }
            else
            {
                ODL_LOG("! (jrt)"); //####
                MpM_FAIL_("JavaScript runtime could not be allocated.");
            }
            if (jrt && jct)
            {
#if (47 <= MOZJS_MAJOR_VERSION)
                JS::CompartmentOptions opts;
                JS::RootedObject       global(jct, JS_NewGlobalObject(jct, &lGlobalClass, nullptr,
                                                                      JS::FireOnNewGlobalHook,
                                                                      opts));

#else // 47 > MOZJS_MAJOR_VERSION
                JS::RootedObject       global(jct, JS_NewGlobalObject(jct, &lGlobalClass, nullptr,
                                                                      JS::FireOnNewGlobalHook));

#endif // 47 > MOZJS_MAJOR_VERSION

                if (global)
                {
                    // Enter a request before running anything in the context. In particular, the
                    // request is needed in order for JS_InitStandardClasses to work properly.
                    bool                     okSoFar;
                    JSAutoRequest            ar(jct);
                    JSAutoCompartment        ac(jct, global);
                    JS::OwningCompileOptions options(jct); // this is used so that script
                                                           // objects persist

                    // Populate the global object with the standard globals, like Object and
                    // Array.
                    if (JS_InitStandardClasses(jct, global))
                    {
                        okSoFar = true;
                    }
                    else
                    {
                        ODL_LOG("! (JS_InitStandardClasses(jct, global))"); //####
                        okSoFar = false;
                        MpM_FAIL_("JavaScript global object could not be initialized.");
                    }
                    if (okSoFar)
                    {
                        if (! addCustomObjects(jct, global, tag, arguments))
                        {
                            ODL_LOG("(! addCustomObjects(jct, global, tag, arguments))"); //####
                            okSoFar = false;
                            MpM_FAIL_("Custom objects could not be added to the JavaScript global "
                                      "object.");
                        }
                    }
                    if (okSoFar)
                    {
                        if (! loadScript(jct, options, scriptSource, scriptPath))
                        {
                            ODL_LOG("(! loadScript(jct, options, scriptSource, " //####
                                    "scriptPath))"); //####
                            okSoFar = false;
                            MpM_FAIL_("Script could not be loaded.");
                        }
                    }
                    bool                sawThread;
                    ChannelVector       loadedInletDescriptions;
                    ChannelVector       loadedOutletDescriptions;
                    double              loadedInterval;
                    JS::AutoValueVector loadedInletHandlers(jct);
                    JS::RootedValue     loadedStartingFunction(jct);
                    JS::RootedValue     loadedStoppingFunction(jct);
                    JS::RootedValue     loadedThreadFunction(jct);
                    YarpString          description;
                    YarpString          helpText;

                    if (okSoFar)
                    {
                        if (! validateLoadedScript(jct, global, sawThread, description, helpText,
                                                   loadedInletDescriptions,
                                                   loadedOutletDescriptions,
                                                   loadedInletHandlers, loadedStartingFunction,
                                                   loadedStoppingFunction, loadedThreadFunction,
                                                   loadedInterval))
                        {
                            ODL_LOG("(! validateLoadedScript(jct, global, sawThread, " //####
                                    "description, helpText, loadedInletDescriptions, " //####
                                    "loadedOutletDescriptions, loadedInletHandlers, " //####
                                    "loadedStartingFunction, loadedStoppingFunction, " //####
                                    "loadedThreadFunction, loadedInterval))"); //####
                            okSoFar = false;
                            MpM_FAIL_("Script is missing one or more functions or variables.");
                        }
                    }
                    if (okSoFar)
                    {
                        JavaScriptFilterService * aService =
                                                        new JavaScriptFilterService(argumentList,
                                                                                    jct, global,
                                                                                    scriptPath,
                                                                                    argc, argv, tag,
                                                                                    description,
                                                                            loadedInletDescriptions,
                                                                        loadedOutletDescriptions,
                                                                                loadedInletHandlers,
                                                                            loadedStartingFunction,
                                                                            loadedStoppingFunction,
                                                                                    sawThread,
                                                                            loadedThreadFunction,
                                                                                    loadedInterval,
                                                                                serviceEndpointName,
                                                                                servicePortNumber);

                        if (aService)
                        {
                            aService->performLaunch(helpText, goWasSet, stdinAvailable,
                                                    reportOnExit);
                            delete aService;
                        }
                        else
                        {
                            ODL_LOG("! (aService)"); //####
                        }
                    }
                }
                else
                {
                    ODL_LOG("! (global)"); //####
                    MpM_FAIL_("JavaScript global object could not be created.");
                }
                ODL_LOG("destroying the JavaScript context"); //####
                JS_DestroyContext(jct);
                ODL_LOG("destroying the JavaScript runtime"); //####
                JS_DestroyRuntime(jrt);
            }
            ODL_LOG("shutting down JavaScript"); //####
            JS_ShutDown();
        }
        else
        {
            ODL_LOG("! (JS_Init())"); //####
            MpM_FAIL_("JavaScript engine could not be started.");
        }
    }
    else
    {
        ODL_LOG("! (0 < scriptSource.length())"); //####
        MpM_FAIL_("Empty script file.");
    }
    ODL_EXIT(); //####
} // setUpAndGo

#if defined(__APPLE__)
# pragma mark Global functions
#endif // defined(__APPLE__)

/*! @brief The entry point for running the %JavaScript filter service.

 The first argument is the path of the script to be run by the service.
 @param[in] argc The number of arguments in 'argv'.
 @param[in] argv The arguments to be used with the %JavaScript filter service.
 @returns @c 0 on a successful test and @c 1 on failure. */
int
main(int      argc,
     char * * argv)
{
    YarpString progName(*argv);

#if defined(MpM_ServicesLogToStandardError)
    ODL_INIT(progName.c_str(), kODLoggingOptionIncludeProcessID | //####
             kODLoggingOptionIncludeThreadID | kODLoggingOptionWriteToStderr | //####
             kODLoggingOptionEnableThreadSupport); //####
#else // ! defined(MpM_ServicesLogToStandardError)
    ODL_INIT(progName.c_str(), kODLoggingOptionIncludeProcessID | //####
             kODLoggingOptionIncludeThreadID | kODLoggingOptionEnableThreadSupport); //####
#endif // ! defined(MpM_ServicesLogToStandardError)
    ODL_ENTER(); //####
#if MAC_OR_LINUX_
    SetUpLogger(progName);
#endif // MAC_OR_LINUX_
    try
    {
        AddressTagModifier                    modFlag = kModificationNone;
        bool                                  goWasSet = false;
        bool                                  reportEndpoint = false;
        bool                                  reportOnExit = false;
        bool                                  stdinAvailable = CanReadFromStandardInput();
        YarpString                            serviceEndpointName;
        YarpString                            servicePortNumber;
        YarpString                            tag;
        YarpStringVector                      arguments;
        Utilities::FilePathArgumentDescriptor firstArg("filePath", T_("Path to script file to use"),
                                                       Utilities::kArgModeRequired, "", "", false,
                                                       false);
        Utilities::ExtraArgumentDescriptor    secondArg("scriptArgument",
                                                        T_("Additional script arguments"));
        Utilities::DescriptorVector           argumentList;

        argumentList.push_back(&firstArg);
        argumentList.push_back(&secondArg);
        if (ProcessStandardServiceOptions(argc, argv, argumentList,
                                          JAVASCRIPTFILTER_SERVICE_DESCRIPTION_, "", 2015,
                                          STANDARD_COPYRIGHT_NAME_, goWasSet, reportEndpoint,
                                          reportOnExit, tag, serviceEndpointName, servicePortNumber,
                                          modFlag, kSkipNone, &arguments))
        {
            Utilities::SetUpGlobalStatusReporter();
            Utilities::CheckForNameServerReporter();
            if (Utilities::CheckForValidNetwork())
            {
                yarp::os::Network yarp; // This is necessary to establish any connections to the
                                        // YARP infrastructure

                Initialize(progName);
                YarpString scriptPath(firstArg.getCurrentValue());
                YarpString tagModifier =
                                Utilities::GetFileNameBase(Utilities::GetFileNamePart(scriptPath));
                bool       nameWasSet = AdjustEndpointName(DEFAULT_JAVASCRIPT_SERVICE_NAME_,
                                                           modFlag, tag, serviceEndpointName,
                                                           tagModifier);

                if (reportEndpoint)
                {
                    cout << serviceEndpointName.c_str() << endl;
                }
                else if (Utilities::CheckForRegistryService())
                {
                    setUpAndGo(argumentList, scriptPath, arguments, progName, argc, argv, tag,
                               serviceEndpointName, servicePortNumber, goWasSet, nameWasSet,
                               reportOnExit, stdinAvailable);
                }
                else
                {
                    ODL_LOG("! (Utilities::CheckForRegistryService())"); //####
                    MpM_FAIL_(MSG_REGISTRY_NOT_RUNNING);
                }
            }
            else
            {
                ODL_LOG("! (Utilities::CheckForValidNetwork())"); //####
                MpM_FAIL_(MSG_YARP_NOT_RUNNING);
            }
            Utilities::ShutDownGlobalStatusReporter();
        }
    }
    catch (...)
    {
        ODL_LOG("Exception caught"); //####
    }
    yarp::os::Network::fini();
    ODL_EXIT_L(0); //####
    return 0;
} // main
