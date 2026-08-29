[Setup]
AppName=FinacleDevAssist
AppVersion=1.0
VersionInfoVersion=1.0.0.0
AppPublisherURL=https://github.com/santhoshswamyv
VersionInfoCompany=Santhosh Swamy
VersionInfoDescription=FinacleDevAssist Installer
DefaultDirName={tmp}
DisableDirPage=yes
DisableProgramGroupPage=yes
OutputDir=output
OutputBaseFilename=FinacleDevAssist-Setup
Compression=lzma2
SolidCompression=yes
PrivilegesRequiredOverridesAllowed=commandline
PrivilegesRequired=lowest
CreateUninstallRegKey=no
Uninstallable=no
DisableReadyPage=yes
WizardStyle=modern dark
WizardImageStretch=no
SetupIconFile=fda_small.ico
WizardImageFile=fda_small.bmp
WizardSmallImageFile=fda_small.bmp

[Code]
var
  NotepadExePage: TInputFileWizardPage;
  ReleasePage: TInputOptionWizardPage;

  NppDir: String;
  PluginsDir: String;
  InstalledVersion: String;

  ReleaseTags: TArrayOfString;
  ReleaseUrls: TArrayOfString;
  ChosenReleaseUrl: String;
  ChosenReleaseTag: String;
  
  CheckingPage: TOutputProgressWizardPage;
  InstallErrorMessage: String;

const
  GITHUB_API_URL = 'https://api.github.com/repos/santhoshswamyv/FinacleDevAssist/releases';
  MAX_RELEASES_SHOWN = 5;

{ ---------- Logging ---------- }
procedure Log(Message: String);
var
  LogFile: String;
begin
  LogFile := ExtractFileDir(ExpandConstant('{srcexe}')) + '\FDA-Installer.log';
  SaveStringToFile(
    LogFile,
    GetDateTimeString('yyyy-mm-dd hh:nn:ss', '-', ':') + ' : ' + Message + #13#10,
    True
  );
end;

{ ---------- Utility ---------- }
function IsDirWritable(Dir: String): Boolean;
var
  TestFile: String;
begin
  TestFile := AddBackslash(Dir) + 'fda_write_test.tmp';
  Result := SaveStringToFile(TestFile, 'test', False);
  if Result then
    DeleteFile(TestFile);
end;

function NormalizeVersion(V: String): String;
begin
  Result := Trim(V);
  if (Length(Result) > 0) and ((Result[1] = 'v') or (Result[1] = 'V')) then
    Result := Copy(Result, 2, Length(Result) - 1);
end;

function SplitOnChar(const S: String; Delim: Char): TArrayOfString;
var
  Temp: TArrayOfString;
  Current: String;
  I: Integer;
  C: Char;
begin
  SetArrayLength(Temp, 0);
  Current := '';
  for I := 1 to Length(S) do
  begin
    C := S[I];
    if C = Delim then
    begin
      SetArrayLength(Temp, GetArrayLength(Temp) + 1);
      Temp[GetArrayLength(Temp) - 1] := Current;
      Current := '';
    end
    else
      Current := Current + C;
  end;
  SetArrayLength(Temp, GetArrayLength(Temp) + 1);
  Temp[GetArrayLength(Temp) - 1] := Current;
  Result := Temp;
end;

function GetInstalledDllVersion(PluginDir: String): String;
var
  DllPath: String;
  VersionStr: String;
  Parts: TArrayOfString;
begin
  Result := 'Not installed';
  DllPath := AddBackslash(PluginDir) + 'FinacleDevAssist.dll';
  if FileExists(DllPath) then
  begin
    if GetVersionNumbersString(DllPath, VersionStr) then
    begin
      Parts := SplitOnChar(VersionStr, '.');
      if GetArrayLength(Parts) >= 3 then
        Result := Parts[0] + '.' + Parts[1] + '.' + Parts[2]
      else
        Result := VersionStr;
    end;
  end;
end;

{ ---------- GitHub API ---------- }
function HttpGetText(Url: String; var ResponseText: String): Boolean;
var
  Http: Variant;
begin
  Result := False;
  try
    Http := CreateOleObject('WinHttp.WinHttpRequest.5.1');
    Http.Open('GET', Url, False);
    Http.SetRequestHeader('User-Agent', 'FinacleDevAssist-Installer');
    Http.SetRequestHeader('Accept', 'application/vnd.github+json');
    Http.Send();
    if Http.Status = 200 then
    begin
      ResponseText := Http.ResponseText;
      Result := True;
    end
    else
      Log('GitHub HTTP Status = ' + IntToStr(Http.Status));
  except
    Log('GitHub API Exception: ' + GetExceptionMessage);
  end;
end;

function PosEx(SubStr: String; S: String; Offset: Integer): Integer;
var
  Temp: String;
  P: Integer;
begin
  Result := 0;
  if Offset > Length(S) then Exit;
  Temp := Copy(S, Offset, Length(S) - Offset + 1);
  P := Pos(SubStr, Temp);
  if P > 0 then
    Result := Offset + P - 1;
end;

function ExtractFieldValues(Json: String; FieldName: String): TArrayOfString;
var
  Results: TArrayOfString;
  SearchIndex, FieldIndex, ColonIndex, ValueStart, ValueEnd: Integer;
begin
  SetArrayLength(Results, 0);
  SearchIndex := 1;
  while True do
  begin
    FieldIndex := PosEx(FieldName, Json, SearchIndex);
    if FieldIndex = 0 then Break;

    ColonIndex := FieldIndex + Length(FieldName);
    while (ColonIndex <= Length(Json)) and (Json[ColonIndex] <> ':') do
      Inc(ColonIndex);
    if ColonIndex > Length(Json) then Break;
    Inc(ColonIndex);
    while (ColonIndex <= Length(Json)) and (Json[ColonIndex] <= ' ') do
      Inc(ColonIndex);

    if Json[ColonIndex] <> '"' then
    begin
      SearchIndex := ColonIndex;
      Continue;
    end;

    ValueStart := ColonIndex + 1;
    ValueEnd := ValueStart;
    while (ValueEnd <= Length(Json)) and (Json[ValueEnd] <> '"') do
      Inc(ValueEnd);
    if ValueEnd > Length(Json) then Break;

    SetArrayLength(Results, GetArrayLength(Results) + 1);
    Results[GetArrayLength(Results) - 1] := Copy(Json, ValueStart, ValueEnd - ValueStart);
    SearchIndex := ValueEnd + 1;
  end;
  Result := Results;
end;

function CheckInternetAndFetchReleases(): Boolean;
var
  Json: String;
  Tags, DownloadUrls: TArrayOfString;
  I: Integer;
  Tag: String;
begin
  Result := False;
  Log('Fetching GitHub releases');

  if not HttpGetText(GITHUB_API_URL, Json) then
  begin
    InstallErrorMessage := 'Unable to connect to GitHub releases.' + #13#10 + 'Please check your internet connection.';
    Exit;
  end;

  Tags := ExtractFieldValues(Json, '"tag_name"');
  DownloadUrls := ExtractFieldValues(Json, '"browser_download_url"');

  SetArrayLength(ReleaseTags, 0);
  SetArrayLength(ReleaseUrls, 0);

  for I := 0 to GetArrayLength(Tags) - 1 do
  begin
    if GetArrayLength(ReleaseTags) >= MAX_RELEASES_SHOWN then Break;
    if I >= GetArrayLength(DownloadUrls) then Break;

    Tag := Trim(Tags[I]);
    { Ensure 'v' is prepended if not already present }
    if (Length(Tag) > 0) and (Tag[1] <> 'v') and (Tag[1] <> 'V') then
      Tag := 'v' + Tag;

    SetArrayLength(ReleaseTags, GetArrayLength(ReleaseTags) + 1);
    SetArrayLength(ReleaseUrls, GetArrayLength(ReleaseUrls) + 1);
    ReleaseTags[GetArrayLength(ReleaseTags) - 1] := Tag;
    ReleaseUrls[GetArrayLength(ReleaseUrls) - 1] := DownloadUrls[I];
  end;

  if GetArrayLength(ReleaseTags) = 0 then
  begin
    InstallErrorMessage := 'No downloadable releases found on GitHub.';
    Exit;
  end;

  Result := True;
end;

{ ---------- Download / extract ---------- }
function URLDownloadToFile(
  lpCaller: Integer; szURL: String; szFileName: String;
  dwReserved: Integer; lpfnCB: Integer
): Integer;
external 'URLDownloadToFileW@urlmon.dll stdcall';

function DownloadRelease(Url: String; DestPath: String): Boolean;
var
  ResultCode: Integer;
begin
  Result := False;
  Log('Starting download: ' + Url + ' to ' + DestPath);

  ResultCode := URLDownloadToFile(0, Url, DestPath, 0, 0);
  Log('URLDownload result = ' + IntToStr(ResultCode));

  if ResultCode <> 0 then
  begin
    InstallErrorMessage := 'Download failed. Error code: ' + IntToStr(ResultCode);
    Exit;
  end;
  if not FileExists(DestPath) then
  begin
    InstallErrorMessage := 'Download completed but ZIP file was not created.';
    Exit;
  end;

  Log('Download completed');
  Result := True;
end;

function ExtractZip(ZipPath, DestDir: String): Boolean;
var
  ShellObj, ZipItems, Folder: Variant;
begin
  Result := False;
  try
    ForceDirectories(DestDir);
    ShellObj := CreateOleObject('Shell.Application');
    ZipItems := ShellObj.NameSpace(ZipPath).Items;
    Folder := ShellObj.NameSpace(DestDir);
    { 20 = 4 (Do not display progress) + 16 (Respond "Yes to All") }
    Folder.CopyHere(ZipItems, 20);
    
    Sleep(1500); // Async process brief wait
    Result := True;
    Log('Extracted ' + ZipPath + ' -> ' + DestDir);
  except
    InstallErrorMessage := 'Failed while extracting ZIP:' + #13#10 + GetExceptionMessage;
    Log('Extract Exception: ' + GetExceptionMessage);
  end;
end;

{ ---------- Main install using native Inno Setup progress UI ---------- }
function InstallRelease(): Boolean;
var
  ZipPath, FinalDestDir, JreZipPath: String;
begin
  Result := False;
  InstallErrorMessage := '';

  { We are downloading directly to PluginsDir }
  ZipPath := AddBackslash(PluginsDir) + 'FinacleDevAssist-release.zip';
  FinalDestDir := AddBackslash(PluginsDir) + 'FinacleDevAssist';

  WizardForm.ProgressGauge.Style := npbstNormal;
  WizardForm.ProgressGauge.Min := 0;
  WizardForm.ProgressGauge.Max := 100;

  WizardForm.StatusLabel.Caption := 'Downloading release package...';
  WizardForm.ProgressGauge.Position := 10;
  WizardForm.Refresh; { Force UI update before download freezes thread }

  if not DownloadRelease(ChosenReleaseUrl, ZipPath) then
    Exit;

  WizardForm.StatusLabel.Caption := 'Preparing extraction...';
  WizardForm.ProgressGauge.Position := 40;
  WizardForm.Refresh;

  if DirExists(FinalDestDir) then
  begin
    Log('Removing existing installation at ' + FinalDestDir);
    DelTree(FinalDestDir, True, True, True);
  end;

  WizardForm.StatusLabel.Caption := 'Extracting FinacleDevAssist files...';
  WizardForm.ProgressGauge.Position := 50;
  WizardForm.Refresh;

  if not ExtractZip(ZipPath, PluginsDir) then
    Exit;

  JreZipPath := AddBackslash(FinalDestDir) + 'jre-17.zip';

  if FileExists(JreZipPath) then
  begin
    WizardForm.StatusLabel.Caption := 'Extracting Java runtime...';
    WizardForm.ProgressGauge.Position := 75;
    WizardForm.Refresh;

    if not ExtractZip(JreZipPath, FinalDestDir) then
      Exit;

    DeleteFile(JreZipPath);
  end
  else
    Log('Warning: jre-17.zip was not found inside the release package.');

  WizardForm.StatusLabel.Caption := 'Cleaning up and verifying installation...';
  WizardForm.ProgressGauge.Position := 90;
  WizardForm.Refresh;

  { Cleanup: Delete the ZIP file from plugins dir as requested }
  if FileExists(ZipPath) then 
    DeleteFile(ZipPath);

  { Verify }
  if not FileExists(AddBackslash(FinalDestDir) + 'FinacleDevAssist.dll') then
  begin
    InstallErrorMessage := 'FinacleDevAssist.dll missing after installation.';
    Exit;
  end;

  WizardForm.ProgressGauge.Position := 100;
  Log('Installation completed successfully: ' + ChosenReleaseTag);
  Result := True;
end;

{ ---------- Wizard Setup ---------- }
procedure InitializeWizard();
begin
  NotepadExePage := CreateInputFilePage(
    wpWelcome,
    'Select Notepad++ executable',
    'Select installed Notepad++ executable.',
    'Browse location:'
  );
  NotepadExePage.Add('Notepad++.exe:', 'Executable files|*.exe|All files|*.*', '.exe');

  ReleasePage := CreateInputOptionPage(
    NotepadExePage.ID,
    'Select release',
    'Choose FinacleDevAssist version',
    'Available releases (most recent 5):',
    True, False
  );

  CheckingPage := CreateOutputProgressPage('Checking Updates', 'Checking GitHub releases...');
end;

function NextButtonClick(CurPageID: Integer): Boolean;
var
  SelectedExe: String;
  I, SelectedIndex: Integer;
begin
  Result := True;

  { Validate Notepad++ }
  if CurPageID = NotepadExePage.ID then
  begin
    SelectedExe := NotepadExePage.Values[0];

    if SelectedExe = '' then
    begin
      MsgBox('Please select Notepad++.exe before continuing.', mbError, MB_OK);
      Result := False; Exit;
    end;

    if not FileExists(SelectedExe) then
    begin
      MsgBox('Selected Notepad++ executable does not exist.', mbError, MB_OK);
      Result := False; Exit;
    end;

    if CompareText(ExtractFileName(SelectedExe), 'notepad++.exe') <> 0 then
    begin
      MsgBox('Please select the correct notepad++.exe file.', mbError, MB_OK);
      Result := False; Exit;
    end;

    NppDir := ExtractFileDir(SelectedExe);
    PluginsDir := AddBackslash(NppDir) + 'plugins';

    if not DirExists(PluginsDir) then ForceDirectories(PluginsDir);

    if not IsDirWritable(PluginsDir) then
    begin
      MsgBox('Administrator privileges are required to install into: ' + #13#10 + PluginsDir, mbError, MB_OK);
      Result := False; Exit;
    end;

    InstalledVersion := GetInstalledDllVersion(AddBackslash(PluginsDir) + 'FinacleDevAssist');

    CheckingPage.Show;
    try
      CheckingPage.ProgressBar.Style := npbstMarquee;
      if not CheckInternetAndFetchReleases() then
      begin
        MsgBox(InstallErrorMessage, mbError, MB_OK);
        Result := False;
        Exit;
      end;
    finally
      CheckingPage.Hide;
    end;

    if InstalledVersion = 'Not installed' then
      ReleasePage.SubCaptionLabel.Caption := 'FinacleDevAssist is not currently installed.'
    else
      ReleasePage.SubCaptionLabel.Caption := 'Currently installed version: v' + NormalizeVersion(InstalledVersion);

    ReleasePage.CheckListBox.Items.Clear;
    for I := 0 to GetArrayLength(ReleaseTags)-1 do
      ReleasePage.Add(ReleaseTags[I]);

    ReleasePage.SelectedValueIndex := 0;
  end;

  { Release verification before we let it move to standard Installation }
  if CurPageID = ReleasePage.ID then
  begin
    SelectedIndex := ReleasePage.SelectedValueIndex;
    ChosenReleaseUrl := ReleaseUrls[SelectedIndex];
    ChosenReleaseTag := ReleaseTags[SelectedIndex];

    if NormalizeVersion(ChosenReleaseTag) = NormalizeVersion(InstalledVersion) then
    begin
      if MsgBox('Selected version (' + ChosenReleaseTag + ') is already installed.' + #13#10 +
                'Do you want to reinstall?', mbConfirmation, MB_YESNO) = IDNO then
      begin
        Result := False;
        Exit;
      end;
    end;
  end;
end;

procedure CurStepChanged(CurStep: TSetupStep);
begin
  { Hijack the standard Installation Phase }
  if CurStep = ssInstall then
  begin
    if not InstallRelease() then
    begin
      MsgBox('Installation Failed:' + #13#10 + InstallErrorMessage, mbError, MB_OK);
      Abort(); { Immediately stop execution safely }
    end;
  end;
end;

procedure DeinitializeSetup();
var
  LogFile: String;
begin
  LogFile := ExtractFileDir(ExpandConstant('{srcexe}')) + '\FDA-Installer.log';
  if FileExists(LogFile) then
  begin
    DeleteFile(LogFile);
  end;
end;