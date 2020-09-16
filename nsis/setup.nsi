; 该脚本使用 HM VNISEdit 脚本编辑器向导产生

; 安装程序初始定义常量
!define PRODUCT_NAME "快速行情"
!define SHORTCUT_NAME "MarketInfo"
!define PRODUCT_VERSION "1.0.1"
!define PRODUCT_PUBLISHER "TSingFun, Inc."
!define PRODUCT_WEB_SITE "http://futures.tsingfun.com"
!define PRODUCT_DIR_REGKEY "Software\Microsoft\Windows\CurrentVersion\App Paths\MarketInfo.exe"
!define PRODUCT_UNINST_KEY "Software\Microsoft\Windows\CurrentVersion\Uninstall\${SHORTCUT_NAME}"
!define PRODUCT_UNINST_ROOT_KEY "HKLM"
!define PRODUCT_DEFAULT_PATH "C:\${SHORTCUT_NAME}"
!define PRODUCT_STARTMENU_REGVAL "NSIS:StartMenuDir"

SetCompressor /SOLID lzma
SetCompressorDictSize 32

; ------ MUI 现代界面定义 (1.67 版本以上兼容) ------
!include "MUI.nsh"

; MUI 预定义常量
!define MUI_ABORTWARNING
!define MUI_ICON "${NSISDIR}\Contrib\Graphics\Icons\modern-install.ico"
!define MUI_UNICON "${NSISDIR}\Contrib\Graphics\Icons\modern-uninstall.ico"
!define MUI_WELCOMEFINISHPAGE_BITMAP "install.bmp"
!define MUI_CUSTOMFUNCTION_GUIINIT onGUIInit

; 语言选择窗口常量设置
!define MUI_LANGDLL_REGISTRY_ROOT "${PRODUCT_UNINST_ROOT_KEY}"
!define MUI_LANGDLL_REGISTRY_KEY "${PRODUCT_UNINST_KEY}"
!define MUI_LANGDLL_REGISTRY_VALUENAME "NSIS:Language"

; 欢迎页面
!insertmacro MUI_PAGE_WELCOME
; 许可协议页面
!define MUI_LICENSEPAGE_RADIOBUTTONS
!insertmacro MUI_PAGE_LICENSE "Eula.rtf"
; 安装目录选择页面
!insertmacro MUI_PAGE_DIRECTORY
DirText "安装程序将安装 $(^NameDA) 在下列文件夹。要安装到不同文件夹，单击 [浏览(B)] 并选择其他的文件夹。 $_CLICK"

; 安装过程页面
!insertmacro MUI_PAGE_INSTFILES
; 安装完成页面
!define MUI_FINISHPAGE_RUN "$INSTDIR\MarketInfo.exe"
!insertmacro MUI_PAGE_FINISH

; 安装卸载过程页面
!insertmacro MUI_UNPAGE_INSTFILES

; 安装界面包含的语言设置
!insertmacro MUI_LANGUAGE "English"
!insertmacro MUI_LANGUAGE "SimpChinese"

; 安装预释放文件
!insertmacro MUI_RESERVEFILE_LANGDLL
!insertmacro MUI_RESERVEFILE_INSTALLOPTIONS
; ------ MUI 现代界面定义结束 ------

Name "${PRODUCT_NAME} ${PRODUCT_VERSION}"
OutFile "Setup.exe"
InstallDir "$PROGRAMFILES\MarketInfo"
InstallDirRegKey HKLM "${PRODUCT_UNINST_KEY}" "UninstallString"
ShowInstDetails hide
ShowUnInstDetails show
BrandingText "快速行情软件安装向导"
Function onGUIInit
	StrCpy $INSTDIR "${PRODUCT_DEFAULT_PATH}"
FunctionEnd

Section "MainSection" SEC01

;--------------------------------------------------------------
!include logiclib.nsh
FindProcDLL::FindProc "MarketInfo.exe"
StrCmp $R0 1 0 +2

messagebox::show MB_SETFOREGROUND|MB_ICONHAND|MB_DEFBUTTON3|MB_TOPMOST "${PRODUCT_NAME}" "" \
	'检测到程序正在运行，是否立即终止程序？$\n$\n\
	【终止】终止程序，安装最新版本$\n\
	【取消】取消安装' \
	 "终止" "取消"
Pop $0

${If} $0 == 1
  KillProcDLL::KillProc "MarketInfo.exe"
${Elseif} $0 == 2
  Abort
${EndIf}
;--------------------------------------------------------------

  SetOutPath "$INSTDIR"
  SetOverwrite on
  
  ; first pic
  EBanner::show /NOUNLOAD "$PLUGINSDIR\Data_1.png"
  Sleep 500
  File "mfc*.dll"
  File "msvcp*.dll"
  File "msvcr*.dll"
  
  File "commlib.dll"
  File "thostmduserapi.dll"
  File "securitymduserapi.dll"
  File "securitytraderapi.dll"
  File "MarketInfo.exe"
  
  ;LiveUpdate
  File "LiveUpdate.exe"
  File "libcurl*.dll"
  ;BugTrap
  File "BugTrap*.dll"
  
  ; second pic
  EBanner::show /NOUNLOAD "$PLUGINSDIR\Data_2.png"
  Sleep 500
  SetOutPath "$INSTDIR\config"
  File "config\futures.dat"
  File "config\ETFs.dat"
  File "config\stocks.dat"
  
  SetOutPath "$INSTDIR\config\bitmap"
  File "config\bitmap\toolbar_close.bmp"
  File "config\bitmap\toolbar_close_hover.bmp"
  File "config\bitmap\toolbar_max.bmp"
  File "config\bitmap\toolbar_max_hover.bmp"
  File "config\bitmap\toolbar_min.bmp"
  File "config\bitmap\toolbar_min_hover.bmp"
  File "config\bitmap\caption_logo.bmp"
  
  SetOutPath "$INSTDIR\config\bitmap\scroll_bar"
  File "config\bitmap\scroll_bar\DownArrow_big.bmp"
  File "config\bitmap\scroll_bar\DownArrow_window_big.bmp"
  File "config\bitmap\scroll_bar\UpArrow_big.bmp"
  File "config\bitmap\scroll_bar\UpArrow_window_big.bmp"
  File "config\bitmap\scroll_bar\VSPAN_window_big.bmp"
  File "config\bitmap\scroll_bar\VThumb1_big.bmp"
  File "config\bitmap\scroll_bar\VThumb1_window_big.bmp"
  File "config\bitmap\scroll_bar\VThumb2_window_big.bmp"
  
  ; third pic
  EBanner::show /NOUNLOAD "$PLUGINSDIR\Data_3.png"
  Sleep 500
  SetOutPath "$INSTDIR"

; 创建开始菜单快捷方式

  CreateDirectory "$SMPROGRAMS\${SHORTCUT_NAME}"
  CreateShortCut "$SMPROGRAMS\${SHORTCUT_NAME}\快速行情.lnk" "$INSTDIR\MarketInfo.exe"
  CreateShortCut "$DESKTOP\快速行情.lnk" "$INSTDIR\MarketInfo.exe"

SectionEnd

Section -AdditionalIcons

  WriteIniStr "$INSTDIR\${SHORTCUT_NAME}.url" "InternetShortcut" "URL" "${PRODUCT_WEB_SITE}"
  CreateShortCut "$SMPROGRAMS\${SHORTCUT_NAME}\快速行情首页.lnk" "$INSTDIR\${SHORTCUT_NAME}.url"
  CreateShortCut "$SMPROGRAMS\${SHORTCUT_NAME}\卸载快速行情.lnk" "$INSTDIR\uninst.exe"

SectionEnd

Section -Post
  WriteUninstaller "$INSTDIR\uninst.exe"
  WriteRegStr HKLM "${PRODUCT_DIR_REGKEY}" "" "$INSTDIR\MarketInfo.exe"
  WriteRegStr ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "DisplayName" "$(^Name)"
  WriteRegStr ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "UninstallString" "$INSTDIR\uninst.exe"
  WriteRegStr ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "DisplayIcon" "$INSTDIR\MarketInfo.exe"
  WriteRegStr ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "DisplayVersion" "${PRODUCT_VERSION}"
  WriteRegStr ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "URLInfoAbout" "${PRODUCT_WEB_SITE}"
  WriteRegStr ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "Publisher" "${PRODUCT_PUBLISHER}"
SectionEnd

#-- 根据 NSIS 脚本编辑规则，所有 Function 区段必须放置在 Section 区段之后编写，以避免安装程序出现未可预知的问题。--#

Function .onInit
  !insertmacro MUI_LANGDLL_DISPLAY
  
  InitPluginsDir
  SetOutPath $PLUGINSDIR
  File ".\Data_*.png"
FunctionEnd

Function .onInstSuccess
  Delete ".\Data_*.png"
FunctionEnd

/******************************
 *  以下是安装程序的卸载部分  *
 ******************************/

Section Uninstall
;--------------------------------------------------------------
!include logiclib.nsh
FindProcDLL::FindProc "MarketInfo.exe"
StrCmp $R0 1 0 +2

messagebox::show MB_SETFOREGROUND|MB_ICONHAND|MB_DEFBUTTON3|MB_TOPMOST "${PRODUCT_NAME}" "" \
	'检测到程序正在运行，是否立即终止程序？$\n$\n\
	【终止】终止程序，继续卸载$\n\
	【取消】取消卸载' \
	 "终止" "取消"
Pop $0

${If} $0 == 1
  KillProcDLL::KillProc "MarketInfo.exe"
${Elseif} $0 == 2
  Abort
${EndIf}
;--------------------------------------------------------------
  Delete "$INSTDIR\${SHORTCUT_NAME}.url"
  Delete "$INSTDIR\uninst.exe"
  ;======================================================
  Delete "$INSTDIR\mfc*.dll"
  Delete "$INSTDIR\msvcp*.dll"
  Delete "$INSTDIR\msvcr*.dll"
  
  Delete "$INSTDIR\MarketInfo.exe"
  Delete "$INSTDIR\commlib.dll"
  Delete "$INSTDIR\thostmduserapi.dll"
  Delete "$INSTDIR\securitymduserapi.dll"
  Delete "$INSTDIR\securitytraderapi.dll"
  Delete "$INSTDIR\*.con"
  
  ;LiveUpdate
  Delete "$INSTDIR\LiveUpdate.exe"
  Delete "$INSTDIR\libcurl*.dll"
  ;BugTrap
  Delete "$INSTDIR\BugTrap*.dll"

  Delete "$SMPROGRAMS\${SHORTCUT_NAME}\卸载快速行情.lnk"
  Delete "$SMPROGRAMS\${SHORTCUT_NAME}\快速行情首页.lnk"
  Delete "$DESKTOP\快速行情.lnk"
  Delete "$SMPROGRAMS\${SHORTCUT_NAME}\快速行情.lnk"

  RMDir /r "$SMPROGRAMS\${SHORTCUT_NAME}"
  RMDir /r "$INSTDIR\config\"

  RMDir "$INSTDIR\"

  DeleteRegKey ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}"
  DeleteRegKey HKLM "${PRODUCT_DIR_REGKEY}"
  SetAutoClose true
SectionEnd

#-- 根据 NSIS 脚本编辑规则，所有 Function 区段必须放置在 Section 区段之后编写，以避免安装程序出现未可预知的问题。--#

Function un.onInit
  MessageBox MB_ICONQUESTION|MB_YESNO|MB_DEFBUTTON2 "您确实要完全移除 $(^Name) ，及其所有的组件？" IDYES +2
  Abort
FunctionEnd

Function un.onUninstSuccess
  HideWindow
  MessageBox MB_ICONINFORMATION|MB_OK "$(^Name) 已成功地从您的计算机移除。"
FunctionEnd