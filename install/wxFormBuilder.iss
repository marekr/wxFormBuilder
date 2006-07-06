;~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
; File:     wxFormBuilder.iss
; Author:   Ryan Mulder
; Date:     02/07/2006
;~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

#define MyAppVer "2.0.57"
#define MyAppName "wxFormBuilder"
#define MyAppPublisher "José Antonio Hurtado"
#define MyAppURL "http://wxformbuilder.sourceforge.net/index_en.html"
#define MyAppExeName "wxFormBuilder.exe"
#define wxFormBuilderMinVer "2.0.56"

[Setup]
AppName={#MyAppName}
AppVerName={#MyAppName} {#MyAppVer}
AppPublisher={#MyAppPublisher}
AppPublisherURL={#MyAppURL}
AppSupportURL={#MyAppURL}
AppUpdatesURL={#MyAppURL}
DefaultDirName={pf}\{#MyAppName}
DisableDirPage=true
DefaultGroupName={#MyAppName}
DisableProgramGroupPage=false
OutputBaseFilename={#MyAppName}_v{#MyAppVer}
Compression=lzma/ultra
SolidCompression=true
InternalCompressLevel=ultra
OutputDir=.
ShowLanguageDialog=yes
AppVersion={#MyAppVer}
AppendDefaultGroupName=false
AllowNoIcons=true
WizardImageFile=compiler:WizModernImage-IS.bmp
WizardSmallImageFile=compiler:WizModernSmallImage-IS.bmp
SetupIconFile=support\wxFormBuilder.ico
UninstallDisplayIcon={app}\wxFormBuilder.exe
ChangesAssociations=true
VersionInfoVersion={#MyAppVer}
VersionInfoDescription={#MyAppName}
InfoAfterFile=files\Changelog.txt

[Tasks]
Name: desktopicon; Description: {cm:CreateDesktopIcon}; GroupDescription: {cm:AdditionalIcons}; Flags: unchecked

[Files]
Source: files\*; DestDir: {app}; Flags: ignoreversion recursesubdirs createallsubdirs

[Icons]
Name: {group}\{#MyAppName}; Filename: {app}\{#MyAppExeName}
;Name: {group}\{#MyAppName} Help; Filename: {app}\{#MyAppName}.hlp
Name: {group}\{cm:UninstallProgram,{#MyAppName}}; Filename: {uninstallexe}
Name: {userdesktop}\{#MyAppName}; Filename: {app}\{#MyAppExeName}; Tasks: desktopicon

[Run]
Filename: {app}\{#MyAppExeName}; Description: {cm:LaunchProgram,{#MyAppName}}; Flags: nowait postinstall skipifsilent

[Components]

[Registry]
Root: HKLM; Subkey: SOFTWARE\Microsoft\Windows\CurrentVersion\App Paths\{#MyAppExeName}; ValueType: string; ValueData: {app}\{#MyAppExeName}; Flags: uninsdeletekey
Root: HKCR; SubKey: .fbp; ValueType: string; ValueData: {#MyAppName}.Project; Flags: uninsdeletekey
Root: HKCR; SubKey: {#MyAppName}.Project\DefaultIcon; ValueType: string; ValueName: ; ValueData: {app}\{#MyAppExeName}; Flags: uninsdeletekey
Root: HKCR; SubKey: {#MyAppName}.Project; ValueType: string; ValueData: {#MyAppName} Project File; Flags: uninsdeletekey
Root: HKCR; SubKey: {#MyAppName}.Project\Shell\Open\Command; ValueType: string; ValueData: """{app}\{#MyAppExeName}"" ""%1"""; Flags: uninsdeletevalue

[Code]
function GetPathInstalled( AppID: String ): String;
var
   sPrevPath: String;
begin
  // Debug Stuff
  //MsgBox( AppID + ' was passed into GetPathInstalled', mbInformation, MB_OK);

  sPrevPath := '';
  if not RegQueryStringValue( HKLM,
    'Software\Microsoft\Windows\CurrentVersion\Uninstall\'+AppID+'_is1',
      'Inno Setup: App Path', sPrevpath) then
    RegQueryStringValue( HKCU, 'Software\Microsoft\Windows\CurrentVersion\Uninstall\'+AppID+'_is1' ,
      'Inno Setup: App Path', sPrevpath);

  // Debug Stuff
  //MsgBox( 'Installed Path: ' + sPrevPath, mbInformation, MB_OK);

  Result := sPrevPath;
end;


function GetInstalledVersion( AppID: String ): String;
var
   sPrevPath: String;

begin
  sPrevPath := '';
  if not RegQueryStringValue( HKLM,
    'Software\Microsoft\Windows\CurrentVersion\Uninstall\'+AppID+'_is1',
		'DisplayVersion', sPrevpath) then
    RegQueryStringValue( HKCU, 'Software\Microsoft\Windows\CurrentVersion\Uninstall\'+AppID+'_is1' ,
		'DisplayVersion', sPrevpath);

  Result := sPrevPath;
end;

function GetPathUninstallString( AppID: String ): String;
var
   sPrevPath: String;
begin
  sPrevPath := '';
  if not RegQueryStringValue( HKLM,
    'Software\Microsoft\Windows\CurrentVersion\Uninstall\'+AppID+'_is1',
		'UninstallString', sPrevpath) then
    RegQueryStringValue( HKCU, 'Software\Microsoft\Windows\CurrentVersion\Uninstall\'+AppID+'_is1' ,
		'UninstallString', sPrevpath);

  Result := sPrevPath;
end;

function InitializeSetup(): boolean;
var
	ResultCode: Integer;
	wxFormBuilderVersion: String;
	sUninstallEXE: String;

begin
	wxFormBuilderVersion:= GetInstalledVersion('{#MyAppName}');
	sUninstallEXE:= RemoveQuotes(GetPathUninstallString('{#MyAppName}'));

	// Debug Stuff
	//MsgBox('wxAdditions Version ' + wxAdditionsVersion + ' was found' #13 'The length is ' + IntToStr(Length(wxAdditionsVersion)), mbInformation, MB_OK);
	//MsgBox('Version ' + wxVersion + ' was found' #13 'The length is ' + IntToStr(Length(wxVersion)), mbInformation, MB_OK);
	//MsgBox('Uninstall is located at : ' + sUninstallEXE, mbInformation, MB_OK);

	// Check to make sure there is an exceptable version of wxAdditions installed.
	if Length(wxFormBuilderVersion) = 0 then begin
		result:= true;
	end else begin
		//MsgBox('wxFormBuilder minimum version: ' + '{#wxFormBuilderMinVer}' #13 'wxFormBuilder current version: ' + wxFormBuilderVersion, mbInformation, MB_OK);
		if CompareStr( '{#wxFormBuilderMinVer}', wxFormBuilderVersion ) <= 0 then begin
			if FileExists(sUninstallEXE) then begin
				//if MsgBox('Version ' + wxFormBuilderVersion + ' of {#MyAppName} was detected.' #13 'It is recommended that you uninstall the old version first before continuing.' + #13 + #13 + 'Would you like to uninstall it now?', mbInformation, MB_YESNO) = IDYES then begin
					Exec(sUninstallEXE,
							'/SILENT',
							GetPathInstalled('{#MyAppName}'),
							SW_SHOWNORMAL,
							ewWaitUntilTerminated,
							ResultCode);

					// Make sure that Setup is visible and the foreground window
					BringToFrontAndRestore;
					result := true;
				//end else
					//result := true;
			end;
		end else begin
			result := true;
		end;
	end;
end;
