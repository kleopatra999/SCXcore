/*--------------------------------------------------------------------------------
 *        Copyright (c) Microsoft Corporation. All rights reserved. See license.txt for license information.
*/
/**
        \file        runasprovider.cpp

        \brief       RunAs provider implementation

        \date        05-15-13 
*/
/*----------------------------------------------------------------------------*/

#include <scxcorelib/scxfile.h>
#include <scxcorelib/scxprocess.h>
#include <scxcorelib/scxdirectoryinfo.h>
#include <scxcorelib/logsuppressor.h>
#include <scxsystemlib/scxsysteminfo.h>
#include "startuplog.h"
#include "scxrunasconfigurator.h"
#include "runasprovider.h"

const std::wstring s_defaultTmpDir = L"/etc/opt/microsoft/scx/conf/tmpdir/";

using namespace SCXSystemLib;
using namespace SCXCoreLib;

namespace SCXCore
{
    void RunAsProvider::Load()
    {
        SCXASSERT( ms_loadCount >= 0 );
        if ( 1 == ++ms_loadCount )
        {
            m_log = SCXLogHandleFactory::GetLogHandle(L"scx.core.providers.runasprovider");
            LogStartup();
            SCX_LOGTRACE(m_log, L"RunAsProvider::Load()");

            if ( NULL == m_Configurator )
            {
                m_Configurator = SCXCoreLib::SCXHandle<RunAsConfigurator> (new RunAsConfigurator());
            }

            ParseConfiguration();
            // set tmpdir location when RunAs provider gets loaded. This will later be used in 
            // every ExecuteScript call. Check for existence of directory will be done in
            // ExecuteScript method so that latest state is taken.
            m_defaultTmpDir = s_defaultTmpDir;
        }
    }

    void RunAsProvider::Unload()
    {
        SCX_LOGTRACE(m_log, L"OSProvider::Unload()");

        SCXASSERT( ms_loadCount >= 1 );
        if (0 == --ms_loadCount)
        {
            m_Configurator = NULL;
        }
    }

    /*----------------------------------------------------------------------------*/
    /**
        Execute a command

        \param[in]     command          Command to execute
        \param[out]    resultOut        Result string from stdout
        \param[out]    resultErr        Result string from stderr
        \param[out]    returncode       Return code from command
        \param[in]     timeout          Accepted number of seconds to wait
        \param[in]     elevationtype    Elevation type 
        \returns       true if command succeeded, else false
        \throws SCXAccessViolationException If execution is prohibited by configuration
    */
    bool RunAsProvider::ExecuteCommand(const std::wstring &command, std::wstring &resultOut, std::wstring &resultErr,
                                       int& returncode, unsigned timeout, const std::wstring &elevationtype)
    {
        SCX_LOGTRACE(m_log, L"RunAsProvider ExecuteCommand");

        if ( ! m_Configurator->GetAllowRoot() )
        {
            SCXUser currentUser;
            if (currentUser.IsRoot())
            {
                throw SCXAccessViolationException(L"Configuration prohibits execution with user: root", SCXSRCLOCATION);
            }
        }

        std::istringstream processInput;
        std::ostringstream processOutput;
        std::ostringstream processError;
        
        // Construct the command by considering the elevation type. It simply returns the command
        // when elevation type is not empty or the current user is already privilege.
        // The elevated command will become a shell command by the design.        
        std::wstring elecommand = ConstructCommandWithElevation(command, elevationtype);

        try
        {
            returncode = SCXCoreLib::SCXProcess::Run(elecommand, processInput, processOutput, processError, timeout * 1000,
                m_Configurator->GetCWD(), m_Configurator->GetChRootPath());
            SCX_LOGHYSTERICAL(m_log, L"\"" + elecommand + L"\" returned " + StrFrom(returncode));
            resultOut = StrFromMultibyte(processOutput.str());
            SCX_LOGHYSTERICAL(m_log, L"stdout: " + resultOut);
            resultErr = StrFromMultibyte(processError.str());
            SCX_LOGHYSTERICAL(m_log, L"stderr: " + resultErr);

            // Trim output if necessary
            if ( OutputLimiter(resultOut, resultErr) )
            {
                SCX_LOGWARNING(m_log, StrAppend(L"ExecuteCommand: Exceeded maximum output size for provider (64k), output truncated. Monitoring will not be reliable! Command executed: ", command));
            }
        }
        catch (SCXCoreLib::SCXException& e)
        {
            resultOut = StrFromMultibyte(processOutput.str());
            resultErr = StrFromMultibyte(processError.str()) + e.What();
            returncode = -1;
        }

        return (returncode == 0);
    }

    /*----------------------------------------------------------------------------*/
    /**
        Execute a command in the default shell.

        \param[in]     command     Command to execute
        \param[out]    resultOut        Result string from stdout
        \param[out]    resultErr        Result string from stderr
        \param[out]    returncode       Return code from command
        \param[in]     timeout          Accepted number of seconds to wait
        \param[in]     elevationtype    Elevation type
        \returns       true if command succeeded, else false
        \throws SCXAccessViolationException If execution is prohibited by configuration
    */
    bool RunAsProvider::ExecuteShellCommand(const std::wstring &command, std::wstring &resultOut, std::wstring &resultErr,
                                            int& returncode, unsigned timeout, const std::wstring &elevationtype)
    {
        SCX_LOGTRACE(m_log, L"RunAsProvider ExecuteShellCommand");

        if ( ! m_Configurator->GetAllowRoot() )
        {
            SCXUser currentUser;
            if (currentUser.IsRoot())
            {
                throw SCXAccessViolationException(L"Configuration prohibits execution with user: root", SCXSRCLOCATION);
            }
        }

        std::istringstream processInput;
        std::ostringstream processOutput;
        std::ostringstream processError;
       
        // Construct the shell command with the given command and elevation type.
        // Please be noted that the constructed shell command use the single quotes. Hence,
        // the current limitation is that the shell command fails if the given command has 
        // single quote. 
        std::wstring shellcommand = ConstructShellCommandWithElevation(command, elevationtype);

        try
        {
            returncode = SCXCoreLib::SCXProcess::Run(shellcommand, processInput, processOutput, processError,
                timeout * 1000, m_Configurator->GetCWD(), m_Configurator->GetChRootPath());

            SCX_LOGHYSTERICAL(m_log, L"\"" + shellcommand + L"\" returned " + StrFrom(returncode));
            resultOut = StrFromMultibyte(processOutput.str());
            SCX_LOGHYSTERICAL(m_log, L"stdout: " + resultOut);
            resultErr = StrFromMultibyte(processError.str());
            SCX_LOGHYSTERICAL(m_log, L"stderr: " + resultErr);

            // Trim output if necessary
            if ( OutputLimiter(resultOut, resultErr) )
            {
                SCX_LOGWARNING(m_log, StrAppend(L"ExecuteShellCommand: Exceeded maximum output size for provider (64k), output truncated. Monitoring will not be reliable! Command executed: ", command));
            }
        }
        catch (SCXCoreLib::SCXException& e)
        {
            resultOut = L"";
            resultErr = e.What();
            returncode = -1;
        }

        return (returncode == 0);
    }

    /*----------------------------------------------------------------------------*/
    /**
        Execute a script

        \param[in]     script           Script to execute
        \param[in]     arguments        Command line arguments to script
        \param[out]    resultOut        Result string from stdout
        \param[out]    resultErr        Result string from stderr
        \param[out]    returncode       Return code from command
        \param[in]     timeout          Accepted number of seconds to wait
        \param[in]     elevationtype    Elevation type

        \returns       true if script succeeded, else false
        \throws SCXAccessViolationException If execution is prohibited by configuration    */
    bool RunAsProvider::ExecuteScript(const std::wstring &script, const std::wstring &arguments, std::wstring &resultOut,
                                      std::wstring &resultErr, int& returncode, unsigned timeout,
                                      const std::wstring &elevationtype)
    {
        SCX_LOGTRACE(m_log, L"SCXRunAsProvider ExecuteScript");

        if ( ! m_Configurator->GetAllowRoot() )
        {
            SCXUser currentUser;
            if (currentUser.IsRoot())
            {
                throw SCXAccessViolationException(L"Configuration prohibits execution with user: root", SCXSRCLOCATION);
            }
        }

        std::istringstream processInput;
        std::ostringstream processOutput;
        std::ostringstream processError;

        try
        {
        	bool tmpDirExists = SCXCoreLib::SCXDirectory::Exists(m_defaultTmpDir);
        	std::wstring tmpDir = tmpDirExists ? m_defaultTmpDir : L"/tmp/";
        	if(!tmpDirExists) {
        		static SCXCoreLib::LogSuppressor suppressor(SCXCoreLib::eWarning, SCXCoreLib::eHysterical);
        		SCX_LOG(m_log, suppressor.GetSeverity(m_defaultTmpDir), L"Default tmp Directory does not exist. Falling back to /tmp");
        	}
            SCXFilePath scriptfile = SCXFile::CreateTempFile(script, tmpDir);
            SCXFileSystem::Attributes attribs = SCXFileSystem::GetAttributes(scriptfile);
            attribs.insert(SCXFileSystem::eUserExecute);
            SCXFile::SetAttributes(scriptfile, attribs);

            std::wstring command(scriptfile.Get());
            command.append(L" ").append(arguments);

            // Construct the command with the given elevation type.
            command = ConstructCommandWithElevation(command, elevationtype);

            returncode = SCXCoreLib::SCXProcess::Run(command, processInput, processOutput, processError, timeout * 1000,
                m_Configurator->GetCWD(), m_Configurator->GetChRootPath());
            SCXFile::Delete(scriptfile);

            SCX_LOGHYSTERICAL(m_log, L"\"" + command + L"\" returned " + StrFrom(returncode));
            resultOut = StrFromMultibyte(processOutput.str());
            SCX_LOGHYSTERICAL(m_log, L"stdout: " + resultOut);
            resultErr = StrFromMultibyte(processError.str());
            SCX_LOGHYSTERICAL(m_log, L"stderr: " + resultErr);

            // Trim output if necessary
            if ( OutputLimiter(resultOut, resultErr) )
            {
                SCX_LOGWARNING(m_log, L"ExecuteScript: Exceeded maximum output size for provider (64k), output truncated. Monitoring will not be reliable! Script contents logged only with hysterical logging.");
            }
        }
        catch (SCXCoreLib::SCXException& e)
        {
            resultOut = L"";
            resultErr = e.What();
            returncode = -1;
        }

        return (returncode == 0);
    }
    
    std::wstring RunAsProvider::ConstructCommandWithElevation(const std::wstring &command, 
                                                              const std::wstring &elevationtype)
    {
        // Construct the command by considering the elevation type.
        // Noted that SystemInfo GetElevatedCommand function will return a
        // shell command when the elevation type is sudo (it simply returns
        // the command when the current user is already elevated).

        SCXSystemLib::SystemInfo si;
        if (elevationtype == L"sudo")
        {
            return si.GetElevatedCommand(command);
        }

        return command;
    }

    // Construct a shell command for the given command and the elevation type.  
    std::wstring RunAsProvider::ConstructShellCommandWithElevation(const std::wstring &command,
                                                                   const std::wstring &elevationtype)
    {
        SCXSystemLib::SystemInfo si;

        std::wstring newCommand(si.GetShellCommand(command));

        // Only when current user is not priviledged and elevation type is sudo
        // the command need to be elevated.
        // Force a shell command so we get a shell (even if already elevated)
        if (elevationtype == L"sudo")
        {
            newCommand = si.GetElevatedCommand(newCommand);
        }

        return newCommand;
    }

    // Limit stdout/stderr length to avoid bumping up against OMI's 64k limit per instance
    // (Not a whole lot of sense in raising that, since WS-Man has a limit as well)
    bool RunAsProvider::OutputLimiter(std::wstring& resultOut, std::wstring& resultErr)
    {
        const size_t maxOutputSize = 60*1024;

        // Do we need to truncate the output?
        if (resultOut.size() + resultErr.size() <= maxOutputSize)
        {
            // Nope, we're good
            return false;
        }

        if ( resultErr.size() == 0 )
        {
            // Truncate stdout only
            resultOut = resultOut.substr(0, maxOutputSize-1);
        }
        else if ( resultOut.size() == 0 )
        {
            // Truncate stderr only
            resultErr = resultErr.substr(0, maxOutputSize-1);
        }
        else
        {
            // They are both non-zero in size. There are a number of ways to do
            // this, but PM said to keep it simple and do this ...

            resultOut = resultOut.substr(0, maxOutputSize - 1 - 1024);
            resultErr = resultErr.substr(0, 1024 - 1);
        }

        return true;
    }

    // Only construct ApplicationServer class once
    int RunAsProvider::ms_loadCount = 0;
    RunAsProvider g_RunAsProvider;
}


/*----------------------------E-N-D---O-F---F-I-L-E---------------------------*/
