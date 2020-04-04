# automated build script for Windows

# check if idf.py is avaialble
try {if(Get-Command idf.py){}}
Catch {
 Write-Host "idf.py not available. Run script in ESP-IDF command prompt with"
 Write-Host "powershell .\build-windows.ps1"
 $Host.UI.RawUI.ReadKey('NoEcho,IncludeKeyDown')
 exit
}

# compress web pages
cd main\www
start-process -wait compress_pages.sh
cd ..\..

# build project
Start-Process -wait idf.py all -NoNewWindow

# read default COM port from build-windows-defaultcomport file
$defaultCOMport = Get-Content build-windows-defaultcomport

# ask user for COM port

$COMport = Read-Host -Prompt "Enter COM port (default: $defaultCOMport)"
if (-not $COMport) {
	$COMport = $defaultCOMport
}

if (-not $COMport.StartsWith("COM")) {
    $COMport = "COM" + $COMport
}

Write-Host 'Restart Device in Upload Mode now'
Write-Host -NoNewLine "Press any key to upload firmware to $COMport"
$null = $Host.UI.RawUI.ReadKey('NoEcho,IncludeKeyDown')

# flash firmware
idf.py -p $COMport app-flash
