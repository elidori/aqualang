#define MyAppName "AquaLang"
#define MyAppFolder "AquaLang"
#define MyAppVersion "1.1.16"
#define MyAppPublisher "AquaLang"
#define MyAppURL ""
#define WORK GetEnv('WORK')

[Setup]
AppId={{F020DF17-75F2-478F-8998-3F6059005E4A}     
AppName={#MyAppName}
AppVersion={#MyAppVersion}
AppPublisher={#MyAppPublisher}
AppPublisherURL={#MyAppURL}
AppSupportURL={#MyAppURL}
AppUpdatesURL={#MyAppURL}
DefaultDirName={commonpf}\{#MyAppFolder}
DefaultGroupName={#MyAppName}
OutputBaseFilename={#MyAppName}
Compression=lzma
SolidCompression=true

SetupIconFile=AquaLang.ico
UninstallDisplayIcon="{app}\AquaLang.ico"

[Languages]
Name: english; MessagesFile: compiler:Default.isl

[Icons]
Name: "{group}\{cm:UninstallProgram, {#MyAppName} }"; Filename: "{uninstallexe}"; IconFilename: "{commonpf}\{#MyAppFolder}\AquaLang.ico"   
Name: "{group}\Stop {#MyAppName}"; Filename: "{app}\AquaLang.exe"; Parameters: "-stop"
Name: "{group}\Run {#MyAppName}"; Filename: "{app}\AquaLang.exe";


[Dirs]
Name: {app}     

[Files]
Source: "AquaLang License Agreenent.rtf"; DestDir: "{app}"
Source: "Aqualang-HowTo.htm"; DestDir: "{app}"
Source: "AquaLang.ico"; DestDir: "{app}"
Source: "vcruntime140.dll"; DestDir: "{app}"
Source: "msvcp140.dll"; DestDir: "{app}"
Source: "vccorlib140.dll"; DestDir: "{app}"
Source: "{#WORK}\AquaLang\Dlls\AquaLang.exe"; DestDir: "{app}";
Source: "{#WORK}\AquaLang\Dlls\AquaLangLib.dll"; DestDir: "{app}";
Source: "{#WORK}\AquaLang\Dlls\AutoStartSetter.exe"; DestDir: "{app}";

[UninstallDelete]
Type: filesandordirs; Name: "{commonpf}\{#MyAppFolder}\*.*"

[Run]
Filename: "{app}\AquaLang.exe"; Parameters: -install; WorkingDir: "{app}"; StatusMsg: "running application"; Flags: runminimized
Filename: "{app}\AutoStartSetter.exe"; Parameters: -a AquaLang; WorkingDir: "{app}"; StatusMsg: "adding application to startup list"; Flags: runminimized

[UninstallRun]
Filename: "{app}\AutoStartSetter.exe"; Parameters: -r AquaLang; WorkingDir: "{app}"; RunOnceId: "AquaLangAutoStart"; Flags: runminimized
Filename: "{app}\AquaLang.exe"; Parameters: -uninstall; WorkingDir: "{app}"; RunOnceId: "AquaLang"; Flags: runminimized
