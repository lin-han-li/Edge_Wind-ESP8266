#define AppName "EdgeWind Monitor"
#define AppVersion "1.0.0"
#define AppPublisher "EdgeWind Team"
#define AppCompany "EdgeWind"

; Allow overriding dist folder from command line:
;   ISCC /DDistDir="D:\Edge_Wind\dist" installer.iss
#ifndef DistDir
#define DistDir "dist"
#endif

[Setup]
AppName={#AppName}
AppVersion={#AppVersion}
AppPublisher={#AppPublisher}
DefaultDirName={pf}\{#AppName}
DefaultGroupName={#AppName}
OutputDir=installer
OutputBaseFilename=EdgeWind_Monitor_Setup
Compression=lzma
SolidCompression=yes
UninstallDisplayName={#AppName}
UninstallDisplayIcon={app}\EdgeWind_Monitor.exe
VersionInfoVersion=1.0.0.0
VersionInfoCompany={#AppCompany}
VersionInfoDescription={#AppName}
VersionInfoProductName={#AppName}
#ifdef SetupIcon
SetupIconFile={#SetupIcon}
#endif

[Tasks]
Name: "desktopicon"; Description: "Create a desktop icon"; Flags: unchecked

[Files]
Source: "{#DistDir}\EdgeWind_Monitor.exe"; DestDir: "{app}"; Flags: ignoreversion

[Icons]
Name: "{group}\EdgeWind Monitor"; Filename: "{app}\EdgeWind_Monitor.exe"
Name: "{commondesktop}\EdgeWind Monitor"; Filename: "{app}\EdgeWind_Monitor.exe"; Tasks: desktopicon

[Run]
Filename: "{app}\EdgeWind_Monitor.exe"; Description: "Launch EdgeWind Monitor"; Flags: nowait postinstall skipifsilent
