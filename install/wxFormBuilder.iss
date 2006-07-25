;~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
; File:     wxFormBuilder.iss
; Author:   Ryan Mulder
; Date:     02/07/2006
;~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

#define MyAppVer "2.0.62"
#define MyAppName "wxFormBuilder"
#define MyAppPublisher "José Antonio Hurtado"
#define MyAppURL "http://wxformbuilder.org"
#define MyAppExeName "wxFormBuilder.exe"
#define wxFormBuilderMinVer "2.0.56"
#define Additions "wxAdditions_setup.exe"

[_ISToolDownload]
Source: http://wxformbuilder.org/downloads/wxAdditions_setup.exe; DestDir: {tmp}; DestName: wxAdditions_setup.exe

[Setup]
AppName={#MyAppName}
AppVerName={#MyAppName} {#MyAppVer}
AppPublisher={#MyAppPublisher}
AppPublisherURL={#MyAppURL}
AppSupportURL={#MyAppURL}
AppUpdatesURL={#MyAppURL}
DefaultDirName={pf}\{#MyAppName}
DisableDirPage=false
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
LicenseFile=files\licence.txt

[Messages]
BeveledLabel={#MyAppName} v{#MyAppVer}

[Tasks]
Name: desktopicon; Description: {cm:CreateDesktopIcon}; GroupDescription: {cm:AdditionalIcons}; Flags: unchecked
;Name: wxadditions; Description: wxAdditions: Needed for building your application if you used any of the widgets from the 'Contrib' tab. (13.5MB Extra); Flags: unchecked; GroupDescription: Download and install (Requires internet connection)

[Files]
Source: files\*; DestDir: {app}; Flags: ignoreversion recursesubdirs createallsubdirs
Source: source\*; DestDir: {app}\source; Flags: ignoreversion recursesubdirs createallsubdirs; Components: main\srccode
Source: support\contrib.bmp; Flags: dontcopy

[Icons]
Name: {group}\{#MyAppName}; Filename: {app}\{#MyAppExeName}
;Name: {group}\{#MyAppName} Help; Filename: {app}\{#MyAppName}.hlp
Name: {group}\{cm:UninstallProgram,{#MyAppName}}; Filename: {uninstallexe}
Name: {userdesktop}\{#MyAppName}; Filename: {app}\{#MyAppExeName}; Tasks: desktopicon

[Run]
Filename: {app}\{#MyAppExeName}; Description: {cm:LaunchProgram,{#MyAppName}}; Flags: nowait postinstall skipifsilent
Filename: {tmp}\{#additions}; WorkingDir: {tmp}; StatusMsg: Installing wxAdditions ...; Flags: hidewizard; Check: wxAdditionsCheck

[Components]
Name: main; Description: wxFormBuilder (required); Flags: fixed dontinheritcheck checkablealone; Types: custom compact full
Name: main\srccode; Description: SourceCode; Types: custom; Flags: dontinheritcheck checkablealone disablenouninstallwarning

[Registry]
Root: HKLM; Subkey: SOFTWARE\Microsoft\Windows\CurrentVersion\App Paths\{#MyAppExeName}; ValueType: string; ValueData: {app}\{#MyAppExeName}; Flags: uninsdeletekey
Root: HKCR; SubKey: .fbp; ValueType: string; ValueData: {#MyAppName}.Project; Flags: uninsdeletekey
Root: HKCR; SubKey: {#MyAppName}.Project\DefaultIcon; ValueType: string; ValueName: ; ValueData: {app}\{#MyAppExeName}; Flags: uninsdeletekey
Root: HKCR; SubKey: {#MyAppName}.Project; ValueType: string; ValueData: {#MyAppName} Project File; Flags: uninsdeletekey
Root: HKCR; SubKey: {#MyAppName}.Project\Shell\Open\Command; ValueType: string; ValueData: """{app}\{#MyAppExeName}"" ""%1"""; Flags: uninsdeletevalue

[_ISToolPreCompile]
Name: create_source_package.bat; Parameters: ; Flags: runminimized

[Code]
// -- Version checking functions
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
		if CompareText( wxFormBuilderVersion, '{#wxFormBuilderMinVer}' ) <= 0 then begin
			if FileExists(sUninstallEXE) then begin
				if WizardSilent() then begin
					// Just uninstall without asking because we are in silent mode.
					Exec(sUninstallEXE,	'/SILENT', GetPathInstalled('{#MyAppName}'),
							SW_SHOWNORMAL, ewWaitUntilTerminated, ResultCode);

					// Make sure that Setup is visible and the foreground window
					BringToFrontAndRestore;
					result := true;
				end else begin
					// Ask if they really want to uninstall because we are in the default installer.
					if MsgBox('Version ' + wxFormBuilderVersion + ' of {#MyAppName} was detected.' #13 'It is recommended that you uninstall the old version first before continuing.' + #13 + #13 + 'Would you like to uninstall it now?', mbInformation, MB_YESNO) = IDYES then begin
						Exec(sUninstallEXE,	'/SILENT', GetPathInstalled('{#MyAppName}'),
							SW_SHOWNORMAL, ewWaitUntilTerminated, ResultCode);

						// Make sure that Setup is visible and the foreground window
						BringToFrontAndRestore;
						result := true;
					end else begin
						result := true;
					end;
				end;
			end;
		end else begin
			result := true;
		end;
	end;
end;
// -- END -- Version checking

// -- Functions for custom page and downloading
var
  wxAdditionsPage: TWizardPage;
  wxAdditionsCheckBox: TCheckBox;

function CreateCustomOptionPage(AAfterId: Integer; ACaption, ASubCaption, AImageFileName, ALabel1Caption, ALabel2Caption,
  ALabel3Caption, ACheckCaption: String; var CheckBox: TCheckBox): TWizardPage;
var
  Page: TWizardPage;
  Bitmap1: TBitmapImage;
  Label1, Label2, Label3: TNewStaticText;
begin
  Page := CreateCustomPage(AAfterID, ACaption, ASubCaption);

  AImageFileName := ExpandConstant('{tmp}\' + AImageFileName);
  if not FileExists(AImageFileName) then
    ExtractTemporaryFile(ExtractFileName(AImageFileName));

  Label1 := TNewStaticText.Create(Page);
  with Label1 do begin
    AutoSize := False;
    Width := Page.SurfaceWidth - Left;
    WordWrap := True;
    Caption := ALabel1Caption;
    Parent := Page.Surface;
  end;
  WizardForm.AdjustLabelHeight(Label1);

  Label2 := TNewStaticText.Create(Page);
  with Label2 do begin
    Top := Label1.Top + Label1.Height + ScaleY(12);
    Font.Style := [fsBold];
    Caption := ALabel2Caption;
    Parent := Page.Surface;
  end;
  WizardForm.AdjustLabelHeight(Label2);

  Bitmap1 := TBitmapImage.Create(Page);
  with Bitmap1 do begin
    AutoSize := True;
    Top := Label2.Top + Label2.Height + ScaleY(12);
    Bitmap.LoadFromFile(ExpandConstant(AImageFileName));
    Parent := Page.Surface;
  end;

  Label3 := TNewStaticText.Create(Page);
  with Label3 do begin
    Top := Bitmap1.Top + Bitmap1.Height + ScaleY(12);
    Caption := ALabel3Caption;
    Parent := Page.Surface;
  end;
  WizardForm.AdjustLabelHeight(Label3);

  CheckBox := TCheckBox.Create(Page);
  with CheckBox do begin
    Top := Label3.Top + Label3.Height + ScaleY(12);
    Width := Page.SurfaceWidth;
    Caption := ACheckCaption;
    Parent := Page.Surface;
  end;

  Result := Page;
end;

procedure CreateCustomPages;
var
  Caption, SubCaption1, ImageFileName, Label1Caption, Label2Caption, Label3Caption, CheckCaption: String;
begin
  Caption := 'wxAdditions';
  SubCaption1 := 'Would you like to download and install wxAdditions?';
  ImageFileName := 'contrib.bmp';
  Label1Caption :=
    'wxFormBuilder includes support for some contributed widgets, including ' +
    'wxScintilla, wxPropGrid, wxFlatNotebook, wxPlot, and AWX. ' +
    'To facilitate the use of these widgets in applications, the source and ' +
    'binaries (compiled with MinGW and VC7.1) are provided in the package ' +
    'wxAdditions (http://wiki.wxformbuilder.org).';
  Label2Caption := 'Using wxAdditions is especially recommended to get full functionality from' + #13#10 +
    'wxFormBuilder.';
  Label3Caption := 'Select whether you would like to download and install wxAdditions, then click Next.';
  CheckCaption := '&Download and install wxAdditions (13.5MB)';

  wxAdditionsPage := CreateCustomOptionPage(wpSelectProgramGroup, Caption, SubCaption1, ImageFileName, Label1Caption, Label2Caption, Label3Caption, CheckCaption, wxAdditionsCheckBox);
end;

function wxAdditionsCheck: Boolean;
begin
  if WizardSilent() then
  begin
    Result := False;
  end else
  begin
	Result := wxAdditionsCheckBox.Checked;
  end;
end;

procedure InitializeWizard;
begin
  CreateCustomPages;

  wxAdditionsCheckBox.Checked := GetPreviousData('wxAdditions', '1') = '1';
end;

procedure RegisterPreviousData(PreviousDataKey: Integer);
begin
  SetPreviousData(PreviousDataKey, 'wxAdditions', IntToStr(Ord(wxAdditionsCheckBox.Checked)));
end;

function NextButtonClick(CurPage: Integer): Boolean;
begin
	if (wxAdditionsCheckBox.Checked) and not (WizardSilent()) then
	begin
		Result := istool_download(CurPage);
	end
	else begin
		Result := True;
	end;
end;
// -- END -- Functions for custom page and downloading
