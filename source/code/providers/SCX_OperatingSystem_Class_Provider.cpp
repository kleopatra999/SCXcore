/*--------------------------------------------------------------------------------
    Copyright (c) Microsoft Corporation. All rights reserved. See license.txt for license information.
*/
/**
    \file        SCX_OperatingSystem_Class_Provider.cpp

    \brief       Provider support using OMI framework.
    
    \date        03-22-2013 17:48:44
*/
/*----------------------------------------------------------------------------*/

/* @migen@ */
#include <MI.h>
#include "SCX_OperatingSystem_Class_Provider.h"

#include <scxcorelib/scxcmn.h>
#include <scxcorelib/scxfile.h>
#include <scxcorelib/scxlog.h>
#include <scxcorelib/scxmath.h>
#include <scxcorelib/scxnameresolver.h>
#include <scxcorelib/scxprocess.h>
#include <scxcorelib/scxuser.h>
#include <scxsystemlib/osenumeration.h>
#include <scxsystemlib/osinstance.h>
#include <scxsystemlib/memoryenumeration.h>
#include <scxsystemlib/processenumeration.h>
#include <scxsystemlib/scxostypeinfo.h>
#include <scxsystemlib/scxsysteminfo.h>
#include <util/Base64Helper.h>

#include "support/scxcimutils.h"
#include "support/scxrunasconfigurator.h"
#include "support/startuplog.h"
#include "support/osprovider.h"
#include "support/runasprovider.h"

using namespace SCXSystemLib;
using namespace SCXCoreLib;

namespace
{
    scxlong BytesToKiloBytes(scxulong bytes)
    {
        // This will do - let's just fix the names
        return SCXCoreLib::KiloBytesToMegaBytes(bytes);
    }
}

MI_BEGIN_NAMESPACE

/**
   Class that represents values passed between the threads for OMI methods
*/
class SCX_OperatingSystem_ThreadParam : public SCXThreadParam
{
public:
    /*----------------------------------------------------------------------------*/
    /**
       Constructor

       \param[in]     context  Context pointer from OMI server
    */
    SCX_OperatingSystem_ThreadParam(MI_Context* context)
        : SCXThreadParam(), m_context(context)
    {
    }

    /*----------------------------------------------------------------------------*/
    /**
       Retrieves the context pointer for this thread to use

       \returns  Pointer to context for this thread to use
    */
    Context& GetContext()
    {
        return m_context;
    }

private:
    Context m_context; //!< Context from OMI server
};

class SCX_OperatingSystem_Command_ThreadParam : public SCX_OperatingSystem_ThreadParam
{
public:
    SCX_OperatingSystem_Command_ThreadParam(MI_Context* context, const SCX_OperatingSystem_ExecuteCommand_Class in)
        : SCX_OperatingSystem_ThreadParam(context), m_input(in)
    {}
    const SCX_OperatingSystem_ExecuteCommand_Class& GetInput() { return m_input; }

private:
    const SCX_OperatingSystem_ExecuteCommand_Class m_input;
};

class SCX_OperatingSystem_ShellCommand_ThreadParam : public SCX_OperatingSystem_ThreadParam
{
public:
    SCX_OperatingSystem_ShellCommand_ThreadParam(MI_Context* context, const SCX_OperatingSystem_ExecuteShellCommand_Class in)
        : SCX_OperatingSystem_ThreadParam(context), m_input(in)
    {}
    const SCX_OperatingSystem_ExecuteShellCommand_Class& GetInput() { return m_input; }

private:
    const SCX_OperatingSystem_ExecuteShellCommand_Class m_input;
};

class SCX_OperatingSystem_Script_ThreadParam : public SCX_OperatingSystem_ThreadParam
{
public:
    SCX_OperatingSystem_Script_ThreadParam(MI_Context* context, const SCX_OperatingSystem_ExecuteScript_Class in)
        : SCX_OperatingSystem_ThreadParam(context), m_input(in)
    {}
    const SCX_OperatingSystem_ExecuteScript_Class& GetInput() { return m_input; }

private:
    const SCX_OperatingSystem_ExecuteScript_Class m_input;
};

static void EnumerateOneInstance(
    Context& context,
    SCX_OperatingSystem_Class& inst,
    bool keysOnly,
    SCXHandle<OSInstance> osinst,
    SCXHandle<MemoryInstance> meminst)
{
    // Get some handles
    SCXHandle<SCXOSTypeInfo> osTypeInfo = SCXCore::g_OSProvider.GetOSTypeInfo();
    SCXLogHandle& log = SCXCore::g_OSProvider.GetLogHandle();

    SCX_LOGTRACE(log, L"OSProvider EnumerateOneInstance()");

    // Fill in the keys
    inst.Name_value( StrToMultibyte(osTypeInfo->GetOSName(true)).c_str() );
    inst.CSCreationClassName_value( "SCX_ComputerSystem" );

    try {
        NameResolver mi;
        inst.CSName_value( StrToMultibyte(mi.GetHostDomainname()).c_str() );
    } catch (SCXException& e) {
        SCX_LOGWARNING(log, StrAppend(
                              StrAppend(L"Can't read host/domainname because ", e.What()),
                              e.Where()));
    }

    inst.CreationClassName_value( "SCX_OperatingSystem" );

    if ( !keysOnly )
    {
        SCXCalendarTime ASCXCalendarTime;
        scxulong Ascxulong, Ascxulong1;
        unsigned short Aunsignedshort;
        vector<string> Avector;
        vector<unsigned short> Aushortvector;
        wstring Awstring;
        signed short Ashort;
        unsigned int Auint;

        /*===================================================================================*/
        /* Defaulted Values (from MOF)                                                       */
        /*===================================================================================*/

        inst.EnabledDefault_value( 2 );
        inst.EnabledState_value( 5 );
        inst.RequestedState_value( 12 );

        /*===================================================================================*/
        /* Properties of CIM_ManagedElement                                                  */
        /*===================================================================================*/

        inst.Caption_value( StrToMultibyte(osTypeInfo->GetCaption()).c_str() );
        inst.Description_value( StrToMultibyte(osTypeInfo->GetDescription()).c_str() );

        /*===================================================================================*/
        /* Properties of CIM_ManagedSystemElement                                            */
        /*===================================================================================*/

        // We don't support the following because there's no way to retrieve on any platforms:
        //      InstallDate
        //      Status
        //      OperationalStatus
        //      StatusDescriptions
        //      HealthState

        /*===================================================================================*/
        /* Properties of CIM_OperatingSystem                                                 */
        /*===================================================================================*/

        // We don't support the following because there's no way to retrieve on any platforms:
        //      EnabledState
        //      OtherEnabledState
        //      RequestedState
        //      EnabledDefault
        //      TimeOfLastStateChange
        //      OverwritePolicy
        //      Distributed

        /* CSCreationClassName is a key property and thus set in AddKeys */
        /* CSName is a key property and thus set in AddKeys */
        /* CreationClassName is a key property and thus set in AddKeys */

        if (osinst->GetOSType(Aunsignedshort))
            inst.OSType_value( Aunsignedshort );

        if (osinst->GetOtherTypeDescription(Awstring))
            inst.OtherTypeDescription_value( StrToMultibyte(Awstring).c_str() );

        if (osinst->GetVersion(Awstring))
            inst.Version_value( StrToMultibyte(Awstring).c_str() );

        if (osinst->GetLastBootUpTime(ASCXCalendarTime))
        {
            MI_Datetime bootTime;
            CIMUtils::ConvertToCIMDatetime( bootTime, ASCXCalendarTime );
            inst.LastBootUpTime_value( bootTime );
        }

        if (osinst->GetLocalDateTime(ASCXCalendarTime))
        {
            MI_Datetime localTime;
            CIMUtils::ConvertToCIMDatetime( localTime, ASCXCalendarTime );
            inst.LocalDateTime_value( localTime );
        }

        if (osinst->GetCurrentTimeZone(Ashort))
            inst.CurrentTimeZone_value( Ashort );

        if (osinst->GetNumberOfLicensedUsers(Auint))
            inst.NumberOfLicensedUsers_value( Auint );

        if (osinst->GetNumberOfUsers(Auint))
            inst.NumberOfUsers_value( Auint );

        if (ProcessEnumeration::GetNumberOfProcesses(Auint))
            inst.NumberOfProcesses_value( Auint );

        if (osinst->GetMaxNumberOfProcesses(Auint))
            inst.MaxNumberOfProcesses_value( Auint );

        if (meminst->GetTotalSwap(Ascxulong))
        {
            inst.TotalSwapSpaceSize_value( BytesToKiloBytes(Ascxulong) );
        }

        if (meminst->GetTotalPhysicalMemory(Ascxulong) && meminst->GetTotalSwap(Ascxulong1))
        {
            inst.TotalVirtualMemorySize_value( BytesToKiloBytes(Ascxulong) + BytesToKiloBytes(Ascxulong1) );
        }

        if (meminst->GetAvailableMemory(Ascxulong))
        {
            Ascxulong = BytesToKiloBytes(Ascxulong);

            if (meminst->GetAvailableSwap(Ascxulong1)) {
                inst.FreeVirtualMemory_value( Ascxulong + BytesToKiloBytes(Ascxulong1) );
            }

            inst.FreePhysicalMemory_value( Ascxulong );
        }

        if (meminst->GetTotalPhysicalMemory(Ascxulong))
            inst.TotalVisibleMemorySize_value( BytesToKiloBytes(Ascxulong) );

        if (meminst->GetTotalSwap(Ascxulong))
            inst.SizeStoredInPagingFiles_value( BytesToKiloBytes(Ascxulong) );

        if (meminst->GetAvailableSwap(Ascxulong))
            inst.FreeSpaceInPagingFiles_value( BytesToKiloBytes(Ascxulong) );

        if (osinst->GetMaxProcessMemorySize(Ascxulong))
            inst.MaxProcessMemorySize_value( Ascxulong );

        if (osinst->GetMaxProcessesPerUser(Auint))
            inst.MaxProcessesPerUser_value( Auint );

        /*===================================================================================*/
        /* Properties of SCX_OperatingSystem (Taken from PG_OperatingSystem)                 */
        /*===================================================================================*/

        SystemInfo sysInfo;
        if (sysInfo.GetNativeBitSize(Aunsignedshort))
        {
            std::ostringstream bitText;
            bitText << Aunsignedshort << " bit";

            inst.OperatingSystemCapability_value( bitText.str().c_str() );
        }

        if (osinst->GetSystemUpTime(Ascxulong))
            inst.SystemUpTime_value( Ascxulong );
    }

    context.Post(inst);
}

SCX_OperatingSystem_Class_Provider::SCX_OperatingSystem_Class_Provider(
    Module* module) :
    m_Module(module)
{
}

SCX_OperatingSystem_Class_Provider::~SCX_OperatingSystem_Class_Provider()
{
}

void SCX_OperatingSystem_Class_Provider::Load(
        Context& context)
{
    SCX_PEX_BEGIN
    {
        SCXThreadLock lock(ThreadLockHandleGet(L"SCXCore::OSProvider::Lock"));
        SCXCore::g_OSProvider.Load();
        SCXCore::g_RunAsProvider.Load();

        // Notify that we don't wish to unload
        MI_Result r = context.RefuseUnload();
        if ( MI_RESULT_OK != r )
        {
            SCX_LOGWARNING(SCXCore::g_OSProvider.GetLogHandle(),
                StrAppend(L"SCX_OperatingSystem_Class_Provider::Load() refuses to not unload, error = ", r));
        }

        context.Post(MI_RESULT_OK);
    }
    SCX_PEX_END( L"SCX_OperatingSystem_Class_Provider::Load", SCXCore::g_OSProvider.GetLogHandle() );
}

void SCX_OperatingSystem_Class_Provider::Unload(
        Context& context)
{
    SCX_PEX_BEGIN
    {
        SCXThreadLock lock(ThreadLockHandleGet(L"SCXCore::OSProvider::Lock"));
        SCXCore::g_OSProvider.Unload();
        SCXCore::g_RunAsProvider.Unload();
        context.Post(MI_RESULT_OK);
    }
    SCX_PEX_END( L"SCX_OperatingSystem_Class_Provider::Unload", SCXCore::g_OSProvider.GetLogHandle() );
}

void SCX_OperatingSystem_Class_Provider::EnumerateInstances(
    Context& context,
    const String& nameSpace,
    const PropertySet& propertySet,
    bool keysOnly,
    const MI_Filter* filter)
{
    SCX_PEX_BEGIN
    {
        SCXThreadLock lock(ThreadLockHandleGet(L"SCXCore::OSProvider::Lock"));

        // Refresh the collection
        SCXHandle<OSEnumeration> osEnum = SCXCore::g_OSProvider.GetOS_Enumerator();
        SCXHandle<MemoryEnumeration> memEnum = SCXCore::g_OSProvider.GetMemory_Enumerator();
        osEnum->Update();
        memEnum->Update();

        SCX_OperatingSystem_Class inst;
        EnumerateOneInstance( context, inst, keysOnly, osEnum->GetTotalInstance(), memEnum->GetTotalInstance() );
        context.Post(MI_RESULT_OK);
    }
    SCX_PEX_END( L"SCX_OperatingSystem_Class_Provider::EnumerateInstances", SCXCore::g_OSProvider.GetLogHandle() );
}

void SCX_OperatingSystem_Class_Provider::GetInstance(
    Context& context,
    const String& nameSpace,
    const SCX_OperatingSystem_Class& instanceName,
    const PropertySet& propertySet)
{
    SCX_PEX_BEGIN
    {
        SCXThreadLock lock(ThreadLockHandleGet(L"SCXCore::OSProvider::Lock"));

        // Was have a 4-part key (on Redhat, it looks like this):
        //   [Key] Name=Red Hat Distribution
        //   [Key] CSCreationClassName=SCX_ComputerSystem
        //   [Key] CSName=jeffcof64-rhel6-01.scx.com
        //   [Key] CreationClassName=SCX_OperatingSystem
        // Considered returning our one instance without validation, but that's not following the rules
        //
        // Look up the values of the two non-fixed keys

        if (!instanceName.Name_exists() || !instanceName.CSCreationClassName_exists() ||
            !instanceName.CSName_exists() || !instanceName.CreationClassName_exists())
        {
            context.Post(MI_RESULT_INVALID_PARAMETER);
            return;
        }

        std::string osName = StrToMultibyte(SCXCore::g_OSProvider.GetOSTypeInfo()->GetOSName(true)).c_str();
        std::string csName;
        try {
            NameResolver mi;
            csName = StrToMultibyte(mi.GetHostDomainname()).c_str();
        } catch (SCXException& e) {
            SCX_LOGWARNING(SCXCore::g_OSProvider.GetLogHandle(), StrAppend(
                               StrAppend(L"Can't read host/domainname because ", e.What()),
                               e.Where()));
        }

        // Now compare (case insensitive for the class names, case sensitive for the others)
        if ( 0 != strcasecmp("SCX_ComputerSystem", instanceName.CSCreationClassName_value().Str())
             || 0 != strcasecmp("SCX_OperatingSystem", instanceName.CreationClassName_value().Str())
             || 0 != strcmp(osName.c_str(), instanceName.Name_value().Str())
             || 0 != strcmp(csName.c_str(), instanceName.CSName_value().Str()))
        {
            context.Post(MI_RESULT_NOT_FOUND);
            return;
        }

        //
        // We have a match, so return the instance
        //

        // Refresh the collection
        SCXHandle<OSEnumeration> osEnum = SCXCore::g_OSProvider.GetOS_Enumerator();
        SCXHandle<MemoryEnumeration> memEnum = SCXCore::g_OSProvider.GetMemory_Enumerator();
        osEnum->Update();
        memEnum->Update();

        SCX_OperatingSystem_Class inst;
        EnumerateOneInstance( context, inst, false, osEnum->GetTotalInstance(), memEnum->GetTotalInstance() );
        context.Post(MI_RESULT_OK);
    }
    SCX_PEX_END( L"SCX_OperatingSystem_Class_Provider::GetInstance", SCXCore::g_OSProvider.GetLogHandle() );
}

void SCX_OperatingSystem_Class_Provider::CreateInstance(
    Context& context,
    const String& nameSpace,
    const SCX_OperatingSystem_Class& newInstance)
{
    context.Post(MI_RESULT_NOT_SUPPORTED);
}

void SCX_OperatingSystem_Class_Provider::ModifyInstance(
    Context& context,
    const String& nameSpace,
    const SCX_OperatingSystem_Class& modifiedInstance,
    const PropertySet& propertySet)
{
    context.Post(MI_RESULT_NOT_SUPPORTED);
}

void SCX_OperatingSystem_Class_Provider::DeleteInstance(
    Context& context,
    const String& nameSpace,
    const SCX_OperatingSystem_Class& instanceName)
{
    context.Post(MI_RESULT_NOT_SUPPORTED);
}

void SCX_OperatingSystem_Class_Provider::Invoke_RequestStateChange(
    Context& context,
    const String& nameSpace,
    const SCX_OperatingSystem_Class& instanceName,
    const SCX_OperatingSystem_RequestStateChange_Class& in)
{
    context.Post(MI_RESULT_NOT_SUPPORTED);
}

void SCX_OperatingSystem_Class_Provider::Invoke_Reboot(
    Context& context,
    const String& nameSpace,
    const SCX_OperatingSystem_Class& instanceName,
    const SCX_OperatingSystem_Reboot_Class& in)
{
    context.Post(MI_RESULT_NOT_SUPPORTED);
}

void SCX_OperatingSystem_Class_Provider::Invoke_Shutdown(
    Context& context,
    const String& nameSpace,
    const SCX_OperatingSystem_Class& instanceName,
    const SCX_OperatingSystem_Shutdown_Class& in)
{
    context.Post(MI_RESULT_NOT_SUPPORTED);
}

static void Invoke_ExecuteCommand_ThreadBody(SCXCoreLib::SCXThreadParamHandle& param)
{
    if (param == 0)
    {
        SCXASSERT( ! "No parameters to Invoke_ExecuteCommand_ThreadBody");
        return;
    }

    SCX_OperatingSystem_Command_ThreadParam* params = static_cast<SCX_OperatingSystem_Command_ThreadParam*> (param.GetData());
    if (params == 0)
    {
        SCXASSERT( ! "Invalid parameters to Invoke_ExecuteCommand_ThreadBody");
        return;
    }

    SCXCoreLib::SCXLogHandle log = SCXCore::g_RunAsProvider.GetLogHandle();
    Context& context = params->GetContext();
    const SCX_OperatingSystem_ExecuteCommand_Class& in = params->GetInput();

    SCX_PEX_BEGIN
    {
        // We specifically do not lock here; we want multiple instances to run
        SCX_LOGTRACE( log, L"SCX_OperatingSystem_Class_Provider::Invoke_ExecuteCommand" )

        // Parameters (from MOF file):
        //   [IN] string Command, 
        //   [OUT] sint32 ReturnCode, 
        //   [OUT] string StdOut, 
        //   [OUT] string StdErr, 
        //   [IN] uint32 timeout,
        //   [IN] string ElevationType (optional)

        // Validate that we have mandatory arguments
        if ( !in.Command_exists() || 0 == strlen(in.Command_value().Str()) || !in.timeout_exists() )
        {
            SCX_LOGTRACE( log, L"Missing arguments to Invoke_ExecuteCommand method" );
            context.Post(MI_RESULT_INVALID_PARAMETER);
            return;
        }

        std::wstring command = StrFromMultibyte( in.Command_value().Str() );
        std::wstring return_out, return_err;

        std::wstring elevation = L"";
        if ( in.ElevationType_exists() )
        {
            elevation = StrToLower( StrFromMultibyte(in.ElevationType_value().Str()) );

            if (elevation != L"sudo" && elevation != L"")
            {
                SCX_LOGTRACE( log, L"Wrong elevation type " + elevation);
                context.Post(MI_RESULT_INVALID_PARAMETER);
                return;
            }
        }

        std::wstring returnOut, returnErr;
        int returnCode;
        bool cmdok;

        SCX_LOGTRACE( log, L"SCX_OperatingSystem_Class_Provider::Invoke_ExecuteCommand - Executing command: " + command);
        cmdok = SCXCore::g_RunAsProvider.ExecuteCommand(command, returnOut, returnErr, returnCode, in.timeout_value(), elevation);
        SCX_LOGTRACE( log, L"SCX_OperatingSystem_Class_Provider::Invoke_ExecuteCommand - Finished executing: " + command);

        // Pass the results back up the chain

        SCX_OperatingSystem_ExecuteCommand_Class inst;

        inst.ReturnCode_value( returnCode );
        inst.StdOut_value( StrToMultibyte(returnOut).c_str() );
        inst.StdErr_value( StrToMultibyte(returnErr).c_str() );
        inst.MIReturn_value( cmdok );
        context.Post(inst);
        context.Post(MI_RESULT_OK);
    } 
    SCX_PEX_END( L"SCX_OperatingSystem_Class_Provider::Invoke_ExecuteCommand", log );
}

void SCX_OperatingSystem_Class_Provider::Invoke_ExecuteCommand(
    Context& context,
    const String& nameSpace,
    const SCX_OperatingSystem_Class& instanceName,
    const SCX_OperatingSystem_ExecuteCommand_Class& in)
{
    SCXCoreLib::SCXLogHandle log = SCXCore::g_RunAsProvider.GetLogHandle();

    SCX_PEX_BEGIN
    {
        SCX_OperatingSystem_Command_ThreadParam* params = new SCX_OperatingSystem_Command_ThreadParam(context.context(), in);
        new SCXCoreLib::SCXThread(Invoke_ExecuteCommand_ThreadBody, params);
    }
    SCX_PEX_END( L"SCX_OperatingSystem_Class_Provider::Invoke_ExecuteCommand", log );
}

static void Invoke_ExecuteShellCommand_ThreadBody(SCXCoreLib::SCXThreadParamHandle& param)
{
    if (param == 0)
    {
        SCXASSERT( ! "No parameters to Invoke_ExecuteShellCommand_ThreadBody");
        return;
    }

    SCX_OperatingSystem_ShellCommand_ThreadParam* params = static_cast<SCX_OperatingSystem_ShellCommand_ThreadParam*> (param.GetData());
    if (params == 0)
    {
        SCXASSERT( ! "Invalid parameters to Invoke_ExecuteShellCommand_ThreadBody");
        return;
    }

    SCXCoreLib::SCXLogHandle log = SCXCore::g_RunAsProvider.GetLogHandle();
    Context& context = params->GetContext();
    const SCX_OperatingSystem_ExecuteShellCommand_Class& in = params->GetInput();

    SCX_PEX_BEGIN
    {
        // We specifically do not lock here; we want multiple instances to run
        SCX_LOGTRACE( log, L"SCX_OperatingSystem_Class_Provider::Invoke_ExecuteShellCommand" )

        // Parameters (from MOF file):
        //   [IN] string Command, 
        //   [OUT] sint32 ReturnCode, 
        //   [OUT] string StdOut, 
        //   [OUT] string StdErr, 
        //   [IN] uint32 timeout,
        //   [IN] string ElevationType (optional)
        //   [IN] boolean b64encoded (optional)

        // Validate that we have mandatory arguments
        if ( !in.Command_exists() || 0 == strlen(in.Command_value().Str()) || !in.timeout_exists() )
        {
            SCX_LOGTRACE( log, L"Missing arguments to Invoke_ExecuteShellCommand method" );
            context.Post(MI_RESULT_INVALID_PARAMETER);
            return;
        }

        std::string commandNarrow = in.Command_value().Str();

        std::wstring elevation = L"";
        if ( in.ElevationType_exists() )
        {
            elevation = StrToLower( StrFromMultibyte(in.ElevationType_value().Str()) );

            if (elevation != L"sudo" && elevation != L"")
            {
                SCX_LOGTRACE( log, L"Wrong elevation type " + elevation);
                context.Post(MI_RESULT_INVALID_PARAMETER);
                return;
            }
        }

        // If we need to decode a Base64-encoded string, do so
        if ( in.b64encoded_exists() && in.b64encoded_value() )
        {
            bool result = util::Base64Helper::Decode(commandNarrow, commandNarrow);
            if ( ! result )
            {
                // Base64 conversion error - return failure
                context.Post(MI_RESULT_FAILED);
                return;
            }
        }

        std::wstring command = StrFromMultibyte( commandNarrow );
        std::wstring returnOut, returnErr;
        int returnCode;
        bool cmdok;

        SCX_LOGTRACE( log, L"SCX_OperatingSystem_Class_Provider::Invoke_ExecuteShellCommand - Executing command: " + command);
        cmdok = SCXCore::g_RunAsProvider.ExecuteShellCommand(command, returnOut, returnErr, returnCode, in.timeout_value(), elevation);
        SCX_LOGTRACE( log, L"SCX_OperatingSystem_Class_Provider::Invoke_ExecuteShellCommand - Finished executing: " + command);

        // Pass the results back up the chain

        SCX_OperatingSystem_ExecuteShellCommand_Class inst;

        inst.ReturnCode_value( returnCode );
        inst.StdOut_value( StrToMultibyte(returnOut).c_str() );
        inst.StdErr_value( StrToMultibyte(returnErr).c_str() );
        inst.MIReturn_value( cmdok );
        context.Post(inst);
        context.Post(MI_RESULT_OK);
    }
    SCX_PEX_END( L"SCX_OperatingSystem_Class_Provider::Invoke_ExecuteShellCommand", log );
}

void SCX_OperatingSystem_Class_Provider::Invoke_ExecuteShellCommand(
    Context& context,
    const String& nameSpace,
    const SCX_OperatingSystem_Class& instanceName,
    const SCX_OperatingSystem_ExecuteShellCommand_Class& in)
{
    SCX_PEX_BEGIN
    {
        SCX_OperatingSystem_ShellCommand_ThreadParam* params = new SCX_OperatingSystem_ShellCommand_ThreadParam(context.context(), in);
        new SCXCoreLib::SCXThread(Invoke_ExecuteShellCommand_ThreadBody, params);
    }
    SCX_PEX_END( L"SCX_OperatingSystem_Class_Provider::Invoke_ExecuteShellCommand", SCXCore::g_RunAsProvider.GetLogHandle() );
}

static void Invoke_ExecuteScript_ThreadBody(SCXCoreLib::SCXThreadParamHandle& param)
{
    if (param == 0)
    {
        SCXASSERT( ! "No parameters to Invoke_ExecuteScript_ThreadBody");
        return;
    }

    SCX_OperatingSystem_Script_ThreadParam* params = static_cast<SCX_OperatingSystem_Script_ThreadParam*> (param.GetData());
    if (params == 0)
    {
        SCXASSERT( ! "Invalid parameters to Invoke_ExecuteScript_ThreadBody");
        return;
    }

    SCXCoreLib::SCXLogHandle log = SCXCore::g_RunAsProvider.GetLogHandle();
    Context& context = params->GetContext();
    const SCX_OperatingSystem_ExecuteScript_Class& in = params->GetInput();

    SCX_PEX_BEGIN
    {
        // We specifically do not lock here; we want multiple instances to run
        SCX_LOGTRACE( log, L"SCX_OperatingSystem_Class_Provider::Invoke_ExecuteScript" )

        // Parameters (from MOF file):
        //   [IN] string Script, 
        //   [IN] string Arguments, 
        //   [OUT] sint32 ReturnCode, 
        //   [OUT] string StdOut, 
        //   [OUT] string StdErr, 
        //   [IN] uint32 timeout, 
        //   [IN] string ElevationType (optional)
        //   [in] string b64encoded (optional)

        // Validate that we have mandatory arguments
        if ( !in.Script_exists() || 0 == strlen(in.Script_value().Str())
             || !in.Arguments_exists() || !in.timeout_exists() )
        {
            SCX_LOGTRACE( log, L"Missing arguments to Invoke_ExecuteScript method" );
            context.Post(MI_RESULT_INVALID_PARAMETER);
            return;
        }

        std::wstring elevation = L"";
        if ( in.ElevationType_exists() )
        {
            elevation = StrToLower( StrFromMultibyte(in.ElevationType_value().Str()) );

            if (elevation != L"sudo" && elevation != L"")
            {
                SCX_LOGTRACE( log, L"Wrong elevation type " + elevation);
                context.Post(MI_RESULT_INVALID_PARAMETER);
                return;
            }
        }

        std::string strScriptNarrow = in.Script_value().Str();
        std::wstring strArgs = StrFromMultibyte(in.Arguments_value().Str());
        std::wstring returnOut, returnErr;
        int returnCode;

        // If we need to decode a Base64-encoded script, do so (just the script, not the arguments)
        if ( in.b64encoded_exists() && in.b64encoded_value() )
        {
            bool result = util::Base64Helper::Decode(strScriptNarrow, strScriptNarrow);
            if ( ! result )
            {
                // Base64 conversion error - return failure
                context.Post(MI_RESULT_FAILED);
                return;
            }
        }

        std::wstring strScript = StrFromMultibyte( strScriptNarrow );

        // Historically, sometimes WSman/Pegasus removed '\r' characters, sometimes not.
        // (Depended on the product.)  Do so here to play it safe.

        std::wstring::size_type pos_slash_r = strScript.find( '\r' );

        while ( std::wstring::npos != pos_slash_r ){
            strScript.erase( pos_slash_r, 1 );
            pos_slash_r = strScript.find( '\r' );
        }

        SCX_LOGTRACE( log, L"SCX_OperatingSystem_Class_Provider::Invoke_ExecuteScript - Executing script: " + strScript);
        bool cmdok = SCXCore::g_RunAsProvider.ExecuteScript(strScript, strArgs, returnOut, returnErr, returnCode, in.timeout_value(), elevation);
        SCX_LOGTRACE( log, L"SCX_OperatingSystem_Class_Provider::Invoke_ExecuteScript - Finshed executing: " + strScript);

        SCX_OperatingSystem_ExecuteScript_Class inst;

        inst.ReturnCode_value( returnCode );
        inst.StdOut_value( StrToMultibyte(returnOut).c_str() );
        inst.StdErr_value( StrToMultibyte(returnErr).c_str() );
        inst.MIReturn_value( cmdok );
        context.Post(inst);
        context.Post(MI_RESULT_OK);
    }
    SCX_PEX_END( L"SCX_OperatingSystem_Class_Provider::Invoke_ExecuteScript", log );
}

void SCX_OperatingSystem_Class_Provider::Invoke_ExecuteScript(
    Context& context,
    const String& nameSpace,
    const SCX_OperatingSystem_Class& instanceName,
    const SCX_OperatingSystem_ExecuteScript_Class& in)
{
    SCXCoreLib::SCXLogHandle log = SCXCore::g_RunAsProvider.GetLogHandle();

    SCX_PEX_BEGIN
    {
        SCX_OperatingSystem_Script_ThreadParam* params = new SCX_OperatingSystem_Script_ThreadParam(context.context(), in);
        new SCXCoreLib::SCXThread(Invoke_ExecuteScript_ThreadBody, params);
    }
    SCX_PEX_END( L"SCX_OperatingSystem_Class_Provider::Invoke_ExecuteScript", log );
}

MI_END_NAMESPACE
