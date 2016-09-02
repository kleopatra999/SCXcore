# SCXcore [![Build Status](https://travis-ci.org/Microsoft/SCXcore.svg?branch=master)](https://travis-ci.org/Microsoft/SCXcore)

SCXcore is the Operations Manager UNIX/Linux Agent, typically used for
Operations Manager, but is also used in a host of other products as
well.

The SCXcore provides a CIMOM provider, based on [OMI][], to return
logging and statistical information for a UNIX or Linux system. The
SCXcore provider runs on AIX 6.1 and later, HP/UX 11.31 and later,
Solaris 5.10 and later, and most versions of Linux as far back as
RedHat 5.0, SuSE 10.1, and Debian 5.0.

[OMI]: https://travis-ci.org/Microsoft/omi

The SCXcore provider provides the following classes (note that
performance measurements were taken on an idle machine, so idle times
were of little value):

- [SCX_Agent](#enumeration-of-scx_agent)
- [SCX_Application_Server](#enumeration-of-scx_application_server)
- [SCX_DiskDrive]()
- [SCX_DiskDriveStatisticalInformation]()
- [SCX_EthernetPortStatistics]()
- [SCX_FileSystem]()
- [SCX_FileSystemStatisticalInformation]()
- [SCX_IPProtocolEndpoint]()
- [SCX_LANEndpoint]()
- [SCX_LogFile]()
- [SCX_MemoryStatisticalInformation]()
- [SCX_OperatingSystem]()
- [SCX_ProcessorStatisticalInformation]()
- [SCX_RTProcessorStatisticalInformation]()
- [SCX_UnixProcess]()
- [SCX_UnixProcessStatisticalInformation]()

-----

The following output shows the results of enumeration of classes:


### Enumeration of SCX_Agent

```
> /opt/omi/bin/omicli ei root/scx SCX_Agent
instance of SCX_Agent
{
    Caption=SCX Agent meta-information
    Description=Release_Build - 20160901
    InstallDate=20160902085021.000000+000
    [Key] Name=scx
    VersionString=1.6.2-416
    MajorVersion=1
    MinorVersion=6
    RevisionNumber=2
    BuildNumber=416
    BuildDate=2016-09-01T00:00:00Z
    Architecture=x64
    OSName=CentOS Linux
    OSType=Linux
    OSVersion=7.0
    KitVersionString=1.6.2-416
    Hostname=jeffcof64-cent7x-01.scx.com
    OSAlias=UniversalR
    UnameArchitecture=x86_64
    MinActiveLogSeverityThreshold=INFO
    MachineType=Virtual
    PhysicalProcessors=1
    LogicalProcessors=2
}
```

### Enumeration of SCX_Application_Server

```
> /opt/omi/bin/omicli ei root/scx SCX_Application_Server
```

### Enumeration of SCX_DiskDrive

### Enumeration of SCX_DiskDriveStatisticalInformation

### Enumeration of SCX_EthernetPortStatistics

### Enumeration of SCX_FileSystem

### Enumeration of SCX_FileSystemStatisticalInformation

### Enumeration of SCX_IPProtocolEndpoint

### Enumeration of SCX_LANEndpoint

### Enumeration of SCX_LogFile

### Enumeration of SCX_MemoryStatisticalInformation

### Enumeration of SCX_OperatingSystem

### Enumeration of SCX_ProcessorStatisticalInformation

### Enumeration of SCX_RTProcessorStatisticalInformation

### Enumeration of SCX_UnixProcess

### Enumeration of SCX_UnixProcessStatisticalInformation
