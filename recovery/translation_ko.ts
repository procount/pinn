<?xml version="1.0" encoding="utf-8"?>
<!DOCTYPE TS>
<TS version="2.0" language="ko" sourcelanguage="en">
<context>
    <name>BootSelectionDialog</name>
    <message>
        <location filename="bootselectiondialog.cpp" line="59"/>
        <source>Cannot display boot menu</source>
        <translation>부팅 메뉴를 표시할 수 없습니다</translation>
    </message>
    <message>
        <location filename="bootselectiondialog.cpp" line="59"/>
        <source>Error mounting settings partition</source>
        <translation>설정 파티선을 마운트하는 과정에서 오류가 발생하였습니다</translation>
    </message>
    <message>
        <location filename="bootselectiondialog.cpp" line="301"/>
        <source>HDMI safe mode</source>
        <translation>HDMI 안전 모드</translation>
    </message>
    <message>
        <location filename="bootselectiondialog.cpp" line="305"/>
        <source>composite PAL mode</source>
        <translation>PAL 영상 출력 모드</translation>
    </message>
    <message>
        <location filename="bootselectiondialog.cpp" line="309"/>
        <source>composite NTSC mode</source>
        <translation>NTSC 영상 출력 모드</translation>
    </message>
    <message>
        <location filename="bootselectiondialog.cpp" line="366"/>
        <source>PINN - Previously selected OS will boot in %1 seconds</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="bootselectiondialog.ui" line="25"/>
        <location filename="bootselectiondialog.cpp" line="361"/>
        <source>PINN - Select OS to boot</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="bootselectiondialog.ui" line="45"/>
        <source>Sticky default</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="bootselectiondialog.ui" line="52"/>
        <source>BOOT</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>ConfEditDialog</name>
    <message>
        <location filename="confeditdialog.ui" line="22"/>
        <source>Config editor</source>
        <translation>설정 편집기</translation>
    </message>
    <message>
        <location filename="confeditdialog.cpp" line="82"/>
        <source>Error</source>
        <translation>오류</translation>
    </message>
    <message>
        <location filename="confeditdialog.cpp" line="83"/>
        <source>Error mounting boot partition</source>
        <translation>부팅 파티선을 마운트하는 과정에서 오류가 발생하였습니다</translation>
    </message>
</context>
<context>
    <name>InitDriveThread</name>
    <message>
        <location filename="initdrivethread.cpp" line="39"/>
        <location filename="initdrivethread.cpp" line="70"/>
        <source>Mounting FAT partition</source>
        <translation>FAT 파티션을 마운트하고 있습니다.</translation>
    </message>
    <message>
        <location filename="initdrivethread.cpp" line="63"/>
        <source>Formatting settings partition</source>
        <translation>설정 파티션을 초기화하고 있습니다.</translation>
    </message>
    <message>
        <location filename="initdrivethread.cpp" line="66"/>
        <source>Error formatting settings partition</source>
        <translation>설정 파티션을 초기화하는 과정에 오류가 발생하였습니다.</translation>
    </message>
    <message>
        <location filename="initdrivethread.cpp" line="149"/>
        <location filename="initdrivethread.cpp" line="350"/>
        <source>Error unmounting system partition.</source>
        <translation>시스템 파티션을 마운트 해제하는 과정에 오류가 발생하였습니다.</translation>
    </message>
    <message>
        <location filename="initdrivethread.cpp" line="166"/>
        <location filename="initdrivethread.cpp" line="354"/>
        <source>Zeroing partition table</source>
        <translation>파티션 테이블을 초기화하고 있습니다.</translation>
    </message>
    <message>
        <location filename="initdrivethread.cpp" line="169"/>
        <location filename="initdrivethread.cpp" line="357"/>
        <source>Error zero&apos;ing MBR/GPT. SD card may be broken or advertising wrong capacity.</source>
        <translation>MBR/GPT를 초기화하는 과잠에 오류가 발생하였습니다. SD 카드가 손상되었거나 용량을 잘못 표시하고 있는지 확인하여 주십시요.</translation>
    </message>
    <message>
        <location filename="initdrivethread.cpp" line="160"/>
        <source>Would you like PINN to create one for you?
WARNING: This will erase all data on your SD card</source>
        <oldsource>Would you like PINN to create one for you?
WARNING: This will erase all data on your SD card</oldsource>
        <translation type="unfinished">PINN로 작업을 시작하시겠습니까?
경고: SD 카드의 모든 데이터를 지웁니다.</translation>
    </message>
    <message>
        <location filename="initdrivethread.cpp" line="161"/>
        <source>Error: No MBR present on SD Card</source>
        <translation>오류: SD 카드에 MBR이 없습니다.</translation>
    </message>
    <message>
        <location filename="initdrivethread.cpp" line="174"/>
        <source>Writing new MBR</source>
        <translation>MBR을 새로 기록하고 있습니다.</translation>
    </message>
    <message>
        <location filename="initdrivethread.cpp" line="182"/>
        <source>Error creating MBR</source>
        <translation>MBR을 만드는 과정에서 오류가 발생하였습니다.</translation>
    </message>
    <message>
        <location filename="initdrivethread.cpp" line="190"/>
        <source>SD card has now been formatted ready for PINN installation. Please re-copy the PINN files onto the card and reboot</source>
        <oldsource>SD card has now been formatted ready for PINN installation. Please re-copy the PINN files onto the card and reboot</oldsource>
        <translation type="unfinished">SD 카드가 PINN 설치를 준비할 수 있도록 초기화 하였습니다.SD 카드에 PINN를 다시 복사한 다음 재부팅하여 주십시요.</translation>
    </message>
    <message>
        <location filename="initdrivethread.cpp" line="195"/>
        <source>SD card has not been formatted correctly. Please reformat using the SD Association Formatting Tool and try again.</source>
        <translation>SD 카드가 정상적으로 초기화되지 않았습니다. SD 카드 초기화 툴을 이용하여 카드를 초기화한 뒤 다시 시도하여 주십시요.</translation>
    </message>
    <message>
        <location filename="initdrivethread.cpp" line="201"/>
        <source>Removing partitions 2,3,4</source>
        <translation>2,3,4번 파티션을 제거하고 있습니다.</translation>
    </message>
    <message>
        <location filename="initdrivethread.cpp" line="215"/>
        <source>Resizing FAT partition</source>
        <translation>FAT 파티션의 크기를 조정하고 있습니다.</translation>
    </message>
    <message>
        <location filename="initdrivethread.cpp" line="239"/>
        <source>Error resizing existing FAT partition</source>
        <translation>이미 있는 FAT 파티션의 크기를 조정하는 과정에서 오류가 발생하였습니다.</translation>
    </message>
    <message>
        <location filename="initdrivethread.cpp" line="245"/>
        <source>Creating extended partition</source>
        <translation>확장 파티션을 만들고 있습니다.</translation>
    </message>
    <message>
        <location filename="initdrivethread.cpp" line="276"/>
        <source>Error creating extended partition</source>
        <translation>확장 파티선을 만드는 과정에서 오류가 발생하였습니다.</translation>
    </message>
    <message>
        <location filename="initdrivethread.cpp" line="342"/>
        <source>Saving boot files to memory</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="initdrivethread.cpp" line="345"/>
        <source>Error saving boot files to memory. SD card may be damaged.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="initdrivethread.cpp" line="361"/>
        <source>Creating partitions</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="initdrivethread.cpp" line="365"/>
        <source>Error partitioning</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="initdrivethread.cpp" line="369"/>
        <source>Formatting boot partition (fat)</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="initdrivethread.cpp" line="372"/>
        <source>Error formatting boot partition (fat)</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="initdrivethread.cpp" line="376"/>
        <source>Copying boot files to storage</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="initdrivethread.cpp" line="452"/>
        <source>Setting disk volume ID</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="initdrivethread.cpp" line="73"/>
        <source>Error mounting system partition.</source>
        <translation>시스템 파티션을 마운트하는 과정에서 오류가 발생하였습니다.</translation>
    </message>
    <message>
        <location filename="initdrivethread.cpp" line="78"/>
        <source>Editing cmdline.txt</source>
        <translation>cmdline.txt 를 편집하고 있습니다.</translation>
    </message>
    <message>
        <location filename="initdrivethread.cpp" line="88"/>
        <source>Error opening %1</source>
        <translation>%1 을 여는 중에 오류가 발생하였습니다.</translation>
    </message>
    <message>
        <location filename="initdrivethread.cpp" line="101"/>
        <source>Writing RiscOS blob</source>
        <translation>RiscOS 바이너리 데이터를 저장하고 있습니다.</translation>
    </message>
    <message>
        <location filename="initdrivethread.cpp" line="104"/>
        <source>Error writing RiscOS blob</source>
        <translation>RiscOS 바이너리 데이터를 저장하는 과정에서 오류가 발생하였습니다.</translation>
    </message>
    <message>
        <location filename="initdrivethread.cpp" line="111"/>
        <source>Unmounting boot partition</source>
        <translation>부팅 파티션을 마운트 해제하고 있습니다.</translation>
    </message>
    <message>
        <location filename="initdrivethread.cpp" line="114"/>
        <source>Finish writing to disk (sync)</source>
        <translation>디스크 기록을 완료하고 있습니다. (동기화 중)</translation>
    </message>
    <message>
        <location filename="initdrivethread.cpp" line="124"/>
        <source>Mounting boot partition again</source>
        <translation>부팅 파티션을 다시 마운트하고 있습니다.</translation>
    </message>
    <message>
        <location filename="initdrivethread.cpp" line="135"/>
        <source>SD card broken (writes do not persist)</source>
        <translation>SD 카드가 손상되었습니다. (기록 내용은 유지되지 않음)</translation>
    </message>
</context>
<context>
    <name>LanguageDialog</name>
    <message>
        <location filename="languagedialog.ui" line="135"/>
        <source>Language (l): </source>
        <translation>언어 (l): </translation>
    </message>
    <message>
        <location filename="languagedialog.ui" line="156"/>
        <source>Keyboard</source>
        <translation>키보드 레이아웃</translation>
    </message>
</context>
<context>
    <name>MainWindow</name>
    <message>
        <location filename="mainwindow.ui" line="47"/>
        <source>Disk space</source>
        <translation>디스크 공간</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="59"/>
        <source>Destination drive:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="82"/>
        <source>Needed:</source>
        <translation>필요한 공간:</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="89"/>
        <source>Available:</source>
        <translation>가능한 공간:</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="131"/>
        <source>toolBar</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="210"/>
        <source>Install (i)</source>
        <translation>설치 (I)</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="213"/>
        <source>Installs the selected operating system image onto this SD card.</source>
        <translation>이 SD 카드에 선택한 운영체제를 설치합니다.</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="216"/>
        <source>I</source>
        <translation>l</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="225"/>
        <source>Exit (Esc)</source>
        <translation>종료 (Esc)</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="228"/>
        <source>Exits and boots the installed operating system.</source>
        <translation>종료하고 설치한 운영체제로 부팅합니다.</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="294"/>
        <source>Wifi (w)</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="312"/>
        <source>Password (p)</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="315"/>
        <source>Change password</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="318"/>
        <source>P</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="330"/>
        <source>Backup (b)</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="333"/>
        <source>B</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="348"/>
        <source>fschk (f)</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="351"/>
        <source>F</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="363"/>
        <source>Clone (c)</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="366"/>
        <source>C</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="170"/>
        <location filename="mainwindow.ui" line="246"/>
        <source>Advanced</source>
        <translation>고급 설정</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="243"/>
        <source>Advanced (a)</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="249"/>
        <source>A</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="261"/>
        <source>Edit config (e)</source>
        <translation>설정 편집 (E)</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="264"/>
        <source>Opens the config.txt and cmdline.txt for the installed OS in a text editor.</source>
        <translation>설치한 운영체제에 사용하는 config.txt 와 cmdline.txt 를 문서 편집기로 엽니다.</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="276"/>
        <source>Online help (h)</source>
        <translation>온라인 도움말 (H)</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="279"/>
        <source>Opens a web browser displaying the Raspberry Pi Forum.</source>
        <translation>Raspberry Pi 포럼을 웹 브라우저로 열어 보여줍니다.</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="297"/>
        <source>Select a wifi network to connect to</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="300"/>
        <source>W</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="380"/>
        <location filename="mainwindow.cpp" line="1452"/>
        <location filename="mainwindow.cpp" line="2038"/>
        <source>RECOMMENDED</source>
        <translation>추천 설치</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="637"/>
        <location filename="mainwindow.cpp" line="656"/>
        <source>Confirm</source>
        <translation>확인</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="122"/>
        <source>Setting up SD card</source>
        <translation>SD 카드 설정</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="168"/>
        <source>Error mounting settings partition</source>
        <translation>설정 파티션을 마운트하는 과정에서 오류가 발생하였습니다.</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="169"/>
        <source>Persistent settings partition seems corrupt. Reformat?</source>
        <translation>고정된 설정 파티션이 손상된 것으로 보입니다. 다시 초기화하시겠습니까?</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="176"/>
        <source>Reformat failed</source>
        <translation>다시 초기화를 할 수 없었습니다.</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="176"/>
        <source>SD card might be damaged</source>
        <translation>SD 카드가 손상된 것으로 보입니다.</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="293"/>
        <source>Please wait while PINN initialises</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="632"/>
        <source>Warning: this will install the selected Operating System(s). All existing data on the SD card will be overwritten, including any OSes that are already installed.</source>
        <oldsource>Warning: this will install the selected Operating System. All existing data on the SD card will be overwritten.</oldsource>
        <translation>경고: 선택한 운영체제를 설치하겠습니다. SD 카드에 이미 설치된 모든 운영체제와 개인 데이터를 전부 덮어씁니다.</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="734"/>
        <location filename="mainwindow.cpp" line="1229"/>
        <location filename="mainwindow.cpp" line="1347"/>
        <location filename="mainwindow.cpp" line="2175"/>
        <source>Error</source>
        <translation>오류</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="383"/>
        <source>INSTALLED</source>
        <translation>설치됨</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="855"/>
        <source>Display Mode Changed</source>
        <translation>디스플레이 모드가 변경되었습니다.</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="787"/>
        <source>HDMI preferred mode</source>
        <translation>HDMI 선호 모드</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="143"/>
        <source>Waiting for SD card (settings partition)</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="156"/>
        <source>Mounting settings partition</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="634"/>
        <source>SD card</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="634"/>
        <source>drive</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="657"/>
        <source>Warning: incompatible Operating System(s) detected. The following OSes aren&apos;t supported on this revision of Raspberry Pi and may fail to boot or function correctly:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="699"/>
        <source>The install process will begin shortly.</source>
        <translation>곧 설치 과정을 시작합니다.</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="722"/>
        <source>OS(es) installed</source>
        <oldsource>OS installed</oldsource>
        <translation>운영체제 설치 완료</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="723"/>
        <source>OS(es) Installed Successfully</source>
        <oldsource>OS Installed Successfully</oldsource>
        <translation>운영체제 설치에 성공하였습니다.</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="755"/>
        <source>PINN v%1 - Built: %2 (%3)</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="791"/>
        <source>HDMI safe mode</source>
        <translation>HDMI 안전 모드</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="795"/>
        <source>composite PAL mode</source>
        <translation>PAL 영상 출력 모드</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="799"/>
        <source>composite NTSC mode</source>
        <translation>NTSC 영상 출력 모드</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="856"/>
        <source>Display mode changed to %1
Would you like to make this setting permanent?</source>
        <translation>디스플레이 모드가 %1 로 변경되었습니다.
이 설정을 계속 유지하시겠습니까?</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="1213"/>
        <location filename="mainwindow.cpp" line="1331"/>
        <source>Error downloading distribution list from Internet:
</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="1215"/>
        <location filename="mainwindow.cpp" line="1333"/>
        <location filename="mainwindow.cpp" line="1703"/>
        <location filename="mainwindow.cpp" line="1713"/>
        <location filename="mainwindow.cpp" line="2270"/>
        <location filename="mainwindow.cpp" line="2288"/>
        <source>Download error</source>
        <translation>다운로드 오류</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="1229"/>
        <source>Error parsing repolist.json downloaded from server</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="1829"/>
        <source>Network access is required to use PINN without local images. Please select your wifi network in the next screen.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="1837"/>
        <source>Wired network access is required to use PINN without local images. Please insert a network cable into the network port.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="1952"/>
        <source>Are you sure you want to reformat the drive &apos;%1&apos; for use with PINN? All existing data on the drive will be deleted!</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="2129"/>
        <source>This will erase all content on the device &apos;%s&apos;. Are you sure?</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="2131"/>
        <source>Clone SD Card</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="2133"/>
        <source>Yes</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="2134"/>
        <source>No</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="2162"/>
        <source>Clone Completed</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="2163"/>
        <source>Clone Completed Successfully</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="2270"/>
        <source>Error downloading update file</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="2349"/>
        <source>PINN UPDATE</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="2350"/>
        <source>A new version of PINN is available</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="2351"/>
        <source>Do you want to download this version?</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="2375"/>
        <source>Downloading Update</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="2375"/>
        <source>Press ESC to cancel</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="1347"/>
        <source>Error parsing list.json downloaded from server</source>
        <translation>서버에서 가져온 list.json의 내용을 분석하는 과정에서 오류가 발생하였습니다.</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="1575"/>
        <source>Needed</source>
        <translation>필요함</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="1576"/>
        <source>Available</source>
        <translation>가능함</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="1703"/>
        <source>Error downloading meta file</source>
        <translation>메타파일을 가져오는 과정에서 오류가 발생하였습니다.</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="1713"/>
        <location filename="mainwindow.cpp" line="2288"/>
        <source>Error writing downloaded file to SD card. SD card or file system may be damaged.</source>
        <translation>SD 카드에 가져온 파일을 기록하는 과정에서 오류가 발생하였습니다. SD 카드 또는 파일 시스템이 손상된 것으로 보입니다.</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="1951"/>
        <source>Reformat drive?</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="991"/>
        <location filename="mainwindow.cpp" line="1828"/>
        <location filename="mainwindow.cpp" line="1836"/>
        <source>No network access</source>
        <translation>네트워크에 접속할 수 없음</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="992"/>
        <source>Wired network access is required for this feature. Please insert a network cable into the network port.</source>
        <translation>이 기능은 유선 네트워크 연결이 필요합니다. 네트워크 포트에 네트워크 케이블을 삽입하여 주십시요.</translation>
    </message>
</context>
<context>
    <name>MultiImageWriteThread</name>
    <message>
        <location filename="multiimagewritethread.cpp" line="55"/>
        <source>partitions.json invalid</source>
        <translation>partitions.json이 올바르지 않음</translation>
    </message>
    <message>
        <location filename="multiimagewritethread.cpp" line="79"/>
        <source>RISCOS cannot be installed. Size of recovery partition too large.</source>
        <translation>RISCOS를 설치할 수 없습니다. 복구 파티션의 크기가 너무 큽니다.</translation>
    </message>
    <message>
        <location filename="multiimagewritethread.cpp" line="144"/>
        <source>Not enough disk space. Need %1 MB, got %2 MB</source>
        <translation>디스크 공간이 부족합니다. 필요한 공간은 %1 MB이며, 남은 공간은 %2 MB입니다.</translation>
    </message>
    <message>
        <location filename="multiimagewritethread.cpp" line="310"/>
        <source>Finish writing (sync)</source>
        <translation>기록을 완료하고 있습니다. (동기화 중)</translation>
    </message>
    <message>
        <location filename="multiimagewritethread.cpp" line="447"/>
        <source>File &apos;%1&apos; does not exist</source>
        <translation>파일 &apos;%1&apos; 이 없습니다.</translation>
    </message>
    <message>
        <location filename="multiimagewritethread.cpp" line="480"/>
        <location filename="multiimagewritethread.cpp" line="486"/>
        <source>%1: Writing OS image</source>
        <oldsource>Writing image</oldsource>
        <translation>%1: 운영체제 이미지 파일을 기록하고 있습니다.</translation>
    </message>
    <message>
        <location filename="multiimagewritethread.cpp" line="500"/>
        <source>%1: Mounting file system</source>
        <oldsource>Mounting file system</oldsource>
        <translation>%1: 파일 시스템을 마운트하고 있습니다.</translation>
    </message>
    <message>
        <location filename="multiimagewritethread.cpp" line="508"/>
        <location filename="multiimagewritethread.cpp" line="532"/>
        <location filename="multiimagewritethread.cpp" line="700"/>
        <source>%1: Error mounting file system</source>
        <oldsource>Error mounting file system</oldsource>
        <translation>%1: 파일 시스템을 마운트하는 과정에서 오류가 발생하였습니다.</translation>
    </message>
    <message>
        <location filename="multiimagewritethread.cpp" line="493"/>
        <source>%1: Creating filesystem (%2)</source>
        <translation>%1: 파일시스템을 만들고 있습니다. (%2)</translation>
    </message>
    <message>
        <location filename="multiimagewritethread.cpp" line="68"/>
        <source>RISCOS cannot be installed. RISCOS offset value mismatch.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="multiimagewritethread.cpp" line="74"/>
        <source>RISCOS cannot be installed. RISCOS offset value missing.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="multiimagewritethread.cpp" line="106"/>
        <source>More than one operating system requires partition number %1</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="multiimagewritethread.cpp" line="111"/>
        <source>Operating system cannot require a system partition (1,5)</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="multiimagewritethread.cpp" line="116"/>
        <source>Operating system cannot claim both primary partitions 2 and 4</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="multiimagewritethread.cpp" line="213"/>
        <source>Fixed partition offset too low</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="multiimagewritethread.cpp" line="273"/>
        <source>Writing partition table</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="multiimagewritethread.cpp" line="280"/>
        <source>Writing boot partition table</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="multiimagewritethread.cpp" line="296"/>
        <source>Zero&apos;ing start of each partition</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="multiimagewritethread.cpp" line="330"/>
        <source>Error reading existing partition table</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="multiimagewritethread.cpp" line="415"/>
        <source>Error creating partition table</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="multiimagewritethread.cpp" line="513"/>
        <source>%1: Downloading and extracting filesystem</source>
        <translation>%1: 파일 시스템을 다운로드 및 추출하고 있습니다.</translation>
    </message>
    <message>
        <location filename="multiimagewritethread.cpp" line="515"/>
        <source>%1: Extracting filesystem</source>
        <oldsource>%1 Extracting filesystem</oldsource>
        <translation>%1: 파일 시스템을 추출하고 있습니다.</translation>
    </message>
    <message>
        <location filename="multiimagewritethread.cpp" line="529"/>
        <source>%1: Mounting FAT partition</source>
        <oldsource>Mouning FAT partition</oldsource>
        <translation>%1: FAT 파티션을 마운트하고 있습니다.</translation>
    </message>
    <message>
        <location filename="multiimagewritethread.cpp" line="541"/>
        <source>%1: Creating os_config.json</source>
        <oldsource>Creating os_config.json</oldsource>
        <translation>%1: os_config.json을 만들고 있습니다.</translation>
    </message>
    <message>
        <location filename="multiimagewritethread.cpp" line="566"/>
        <source>%1: Saving display mode to config.txt</source>
        <oldsource>Saving display mode to config.txt</oldsource>
        <translation>%1: 디스플레이 모드를 config.txt에 저장하고 있습니다.</translation>
    </message>
    <message>
        <location filename="multiimagewritethread.cpp" line="577"/>
        <source>%1: Running partition setup script</source>
        <oldsource>Running partition setup script</oldsource>
        <translation>%1: 파티션 설치 스크립트를 실행하고 있습니다.</translation>
    </message>
    <message>
        <location filename="multiimagewritethread.cpp" line="626"/>
        <source>%1: Error executing partition setup script</source>
        <oldsource>Error executing partition setup script</oldsource>
        <translation>%1: 파티션 설치 스크립트를 실행하는 과정에서 오류가 발생하였습니다.</translation>
    </message>
    <message>
        <location filename="multiimagewritethread.cpp" line="635"/>
        <source>%1: Configuring flavour</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="multiimagewritethread.cpp" line="649"/>
        <source>%1: Unmounting FAT partition</source>
        <oldsource>Unmounting FAT partition</oldsource>
        <translation>%1: FAT 파티션을 마운트 해제하고 있습니다.</translation>
    </message>
    <message>
        <location filename="multiimagewritethread.cpp" line="652"/>
        <source>%1: Error unmounting</source>
        <oldsource>Error unmounting</oldsource>
        <translation>%1: 파일 시스템을 마운트 해제하는 과정에서 오류가 발생하였습니다.</translation>
    </message>
    <message>
        <location filename="multiimagewritethread.cpp" line="917"/>
        <source>Error creating file system</source>
        <translation>파일 시스템을 만드는 과정에서 오류가 발생하였습니다.</translation>
    </message>
    <message>
        <location filename="multiimagewritethread.cpp" line="959"/>
        <location filename="multiimagewritethread.cpp" line="1030"/>
        <location filename="multiimagewritethread.cpp" line="1089"/>
        <source>Unknown compression format file extension. Expecting .lzo, .gz, .xz, .bz2 or .zip</source>
        <translation>압축 파일의 형식을 알아낼 수 없었습니다. .lzo 또는 .gz, .xz, .bz2, .zip 형식을 지원합니다.</translation>
    </message>
    <message>
        <location filename="multiimagewritethread.cpp" line="992"/>
        <source>Error downloading or extracting tarball</source>
        <translation>tarball을 다운로드/추출하는 과정에서 오류가 발생하였습니다.</translation>
    </message>
    <message>
        <location filename="multiimagewritethread.cpp" line="1051"/>
        <location filename="multiimagewritethread.cpp" line="1110"/>
        <source>Error downloading or writing OS to SD card</source>
        <translation>운영체제를 SD 카드에 다운로드/기록하는 과정에서 오류가 발생하였습니다.</translation>
    </message>
</context>
<context>
    <name>Passwd</name>
    <message>
        <location filename="passwd.ui" line="14"/>
        <source>Reset Password</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="passwd.ui" line="45"/>
        <source>Username</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="passwd.ui" line="71"/>
        <source>Password</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="passwd.ui" line="110"/>
        <source>Retype Password</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="passwd.ui" line="123"/>
        <source>Strength</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="passwd.ui" line="158"/>
        <source>Show Password</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="passwd.ui" line="171"/>
        <source>Use Default</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="passwd.cpp" line="226"/>
        <source>Very Weak</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="passwd.cpp" line="227"/>
        <source>Weak</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="passwd.cpp" line="228"/>
        <source>Good</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="passwd.cpp" line="229"/>
        <source>Strong</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="passwd.cpp" line="230"/>
        <source>Very Strong</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>ProgressSlideshowDialog</name>
    <message>
        <location filename="progressslideshowdialog.cpp" line="153"/>
        <source>%1 MB of %2 MB written (%3 MB/sec)</source>
        <translation>%2 MB 중 %1 MB 기록 완료 (%3 MB/초)</translation>
    </message>
    <message>
        <location filename="progressslideshowdialog.cpp" line="171"/>
        <source>%1 MB written (%2 MB/sec)</source>
        <translation>%1 MB 기록 완료 (%2 MB/초)</translation>
    </message>
</context>
<context>
    <name>QDialogButtonBox</name>
    <message>
        <location filename="languagedialog.cpp" line="27"/>
        <source>OK</source>
        <translation>확인</translation>
    </message>
    <message>
        <location filename="languagedialog.cpp" line="29"/>
        <source>Cancel</source>
        <translation>취소</translation>
    </message>
    <message>
        <location filename="languagedialog.cpp" line="30"/>
        <source>&amp;Cancel</source>
        <translation>취소(&amp;C)</translation>
    </message>
    <message>
        <location filename="languagedialog.cpp" line="31"/>
        <source>Close</source>
        <translation>닫기</translation>
    </message>
    <message>
        <location filename="languagedialog.cpp" line="32"/>
        <source>&amp;Close</source>
        <translation>닫기(&amp;C)</translation>
    </message>
    <message>
        <location filename="languagedialog.cpp" line="33"/>
        <source>&amp;Yes</source>
        <translation>예(&amp;Y)</translation>
    </message>
    <message>
        <location filename="languagedialog.cpp" line="34"/>
        <source>&amp;No</source>
        <translation>아니요(&amp;N)</translation>
    </message>
    <message>
        <location filename="languagedialog.cpp" line="35"/>
        <source>Ignore</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="languagedialog.cpp" line="36"/>
        <source>Show Details...</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="languagedialog.cpp" line="37"/>
        <source>Hide Details...</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="languagedialog.cpp" line="28"/>
        <source>&amp;OK</source>
        <translation>확인(&amp;O)</translation>
    </message>
</context>
<context>
    <name>WifiSettingsDialog</name>
    <message>
        <location filename="wifisettingsdialog.ui" line="22"/>
        <source>Wifi network selection</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="wifisettingsdialog.ui" line="28"/>
        <source>Wifi network</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="wifisettingsdialog.ui" line="55"/>
        <source>Authentication method</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="wifisettingsdialog.ui" line="64"/>
        <source>Pressing the WPS button on my wifi router</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="wifisettingsdialog.ui" line="71"/>
        <source>Password authentication</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="wifisettingsdialog.ui" line="96"/>
        <source>Username</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="wifisettingsdialog.ui" line="110"/>
        <source>Password</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="wifisettingsdialog.ui" line="124"/>
        <source>Show Password</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="wifisettingsdialog.cpp" line="44"/>
        <source>No wifi interfaces</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="wifisettingsdialog.cpp" line="44"/>
        <source>No wifi interfaces available</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="wifisettingsdialog.cpp" line="227"/>
        <source>Authenticated but still waiting for DHCP lease</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="wifisettingsdialog.cpp" line="234"/>
        <source>Connecting to wifi failed</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="wifisettingsdialog.cpp" line="234"/>
        <source>Connecting to the wifi access point failed. Check your password</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="wifisettingsdialog.cpp" line="269"/>
        <source>Cancel</source>
        <translation>취소</translation>
    </message>
    <message>
        <location filename="wifisettingsdialog.cpp" line="275"/>
        <source>Releasing old DHCP lease</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="wifisettingsdialog.cpp" line="290"/>
        <source>Disconnecting</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="wifisettingsdialog.cpp" line="295"/>
        <source>Connecting</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>piCloneThread</name>
    <message>
        <location filename="piclonethread.cpp" line="87"/>
        <source>Preparing target...</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="piclonethread.cpp" line="116"/>
        <source>Reading Partitions...</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="piclonethread.cpp" line="162"/>
        <source>Preparing Partitions...</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="piclonethread.cpp" line="169"/>
        <source>Creating partition %1 of %2 </source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="piclonethread.cpp" line="275"/>
        <source>Copying partition %d of %d...</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="piclonethread.cpp" line="346"/>
        <source>Copying %1 MB of %2 MB</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>piclonedialog</name>
    <message>
        <location filename="piclonedialog.ui" line="14"/>
        <source>Clone SD Card</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="piclonedialog.ui" line="42"/>
        <source>Copy From Device:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="piclonedialog.ui" line="75"/>
        <source>Copy To Device:</source>
        <translation type="unfinished"></translation>
    </message>
</context>
</TS>
