;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;
;; photivo
;;
;; Copyright (C) 2010 Bernd Sch�ler
;;
;; This file is part of photivo.
;;
;; photivo is free software: you can redistribute it and;or modify
;; it under the terms of the GNU General Public License version 3
;; as published by the Free Software Foundation.
;;
;; photivo is distributed in the hope that it will be useful,
;; but WITHOUT ANY WARRANTY; without even the implied warranty of
;; MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
;; GNU General Public License for more details.
;;
;; You should have received a copy of the GNU General Public License
;; along with photivo.  If not, see <http:;;www.gnu.org;licenses;>.
;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

[Setup]
; NOTE: The value of AppId uniquely identifies this application.
; Do not use the same AppId value in installers for other applications.
; (To generate a new GUID, click Tools | Generate GUID inside the IDE.)
AppID={F7E4DC4D-EFDF-4896-95EA-7AB47255CFF8}
AppName=Photivo
AppVersion=DD MMM 20YY (???)
AppPublisherURL=http://photivo.org/
AppSupportURL=http://photivo.org/
AppUpdatesURL=http://photivo.org/
DefaultDirName={pf}\Photivo
DefaultGroupName=Photivo
AllowNoIcons=yes
InfoBeforeFile=C:\Compiler\Changelog.txt
OutputBaseFilename=photivo-setup--win32
Compression=lzma/Max
SolidCompression=false
ArchitecturesAllowed=x86
ChangesAssociations=false
ShowLanguageDialog=no
LanguageDetectionMethod=none

[Languages]
Name: "english"; MessagesFile: "compiler:Default.isl"

[Tasks]
Name: "desktopicon"; Description: "{cm:CreateDesktopIcon}"; GroupDescription: "{cm:AdditionalIcons}"; Flags: unchecked
Name: "quicklaunchicon"; Description: "{cm:CreateQuickLaunchIcon}"; GroupDescription: "{cm:AdditionalIcons}"; Flags: unchecked; OnlyBelowVersion: 0,6.1

[Files]
Source: "C:\Compiler\Photivo\build32\_bin\*"; DestDir: "{app}"; Flags: ignoreversion recursesubdirs createallsubdirs

[Icons]
Name: "{group}\Photivo"; Filename: "{app}\photivo.exe"
Name: "{group}\{cm:ProgramOnTheWeb,Photivo}"; Filename: "http://photivo.org/"
Name: "{group}\{cm:UninstallProgram,Photivo}"; Filename: "{uninstallexe}"
Name: "{commondesktop}\Photivo"; Filename: "{app}\photivo.exe"; Tasks: desktopicon
Name: "{userappdata}\Microsoft\Internet Explorer\Quick Launch\Photivo"; Filename: "{app}\photivo.exe"; Tasks: quicklaunchicon