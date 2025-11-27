#SingleInstance Force

; Global flag to stop the loop
global stopLoop := false

; Show a warning when the script is activated
MsgBox("Screenshot helper is running.`nF8 = start/continue capture`nSpace (in Chrome) = finish current page`nCtrl+F8 = exit")

; ========= EXIT HOTKEY =========
^F8::
{
    MsgBox("Stopping screenshot helper and exiting.")
    ExitApp()
}

; ========= STOP LOOP HOTKEY =========
; Only trigger in Chrome browser
#HotIf WinActive("ahk_exe chrome.exe")
Space::
{
    global stopLoop
    stopLoop := true
    ToolTip("Finishing current page...")
    SetTimer(() => ToolTip(), -2000)  ; Hide tooltip after 2 seconds
}
#HotIf

; ========= MAIN HOTKEY =========
F8::
{
    global stopLoop
    stopLoop := false  ; Reset the stop flag
    
    ; ========= CONFIG =========
    baseFolder      := "D:\Code\school\screenshots"          ; Base folder for screenshots
    iterations      := 100                                   ; Max iterations per page
    scrollLines     := 8                                     ; Mouse wheel steps after each shot

    ; *** IMPROVEMENT: Activate the target window before starting the loop ***
    WinActivate("ahk_exe chrome.exe") ; <-- CHANGE 'chrome.exe' to your browser's executable (e.g., 'msedge.exe', 'firefox.exe')
    Sleep(200)

    ; *** SCROLL TO TOP OF PAGE ***
    Send("^{Home}")  ; Ctrl+Home to jump to top
    Sleep(500)  ; Wait for page to scroll to top

    ; Get page URL and sanitize it for filename use
    pageUrl := GetActivePageUrl()
    if (pageUrl = "") {
        safeUrl := "no_url_detected"
    } else {
        ; Split the URL by the forward slash
        urlParts := StrSplit(pageUrl, "/")
        
        ; Get the index of the last element
        lastIndex := urlParts.Length
        
        ; Determine the part to use for the filename
        fileNamePart := ""
        
        ; 1. Try the very last part (handles /page)
        if (lastIndex > 0)
            fileNamePart := urlParts[lastIndex]
            
        ; 2. If the last part is empty (due to trailing /), try the second-to-last part (handles /page/)
        ;    Ensure there are at least two parts available (index > 1)
        if (fileNamePart = "" && lastIndex > 1)
            fileNamePart := urlParts[lastIndex - 1]

        ; Sanitize the selected part
        safeUrl := SanitizeFileName(fileNamePart)
        
        ; Fallback if everything fails
        if (safeUrl = "")
            safeUrl := "no_url_detected"
    }

    ; Create subfolder for this page
    saveFolder := baseFolder "\" safeUrl
    if !DirExist(saveFolder)
        DirCreate(saveFolder)

    ToolTip("Capturing: " safeUrl)
    SetTimer(() => ToolTip(), -2000)

    ; Use screen coordinates for searching and clicking
    CoordMode("Pixel", "Screen")
    CoordMode("Mouse", "Screen")

    ; Variables to track duplicate screenshots
    lastFileSize := 0
    duplicateCount := 0
    maxDuplicates := 2  ; Stop after 2 identical screenshots in a row

    Loop iterations
    {
        idx := A_Index
        ; Format with leading zeros: filename_01.png, filename_02.png, etc.
        fileName := Format("{}_{:02d}.png", safeUrl, idx)
        fullPath := saveFolder "\" fileName

        ; ========= SCREENSHOT =========
        CaptureChromeWindow(fullPath)

        ; Check file size to detect if we've reached the bottom
        currentFileSize := FileGetSize(fullPath)
        
        if (currentFileSize = lastFileSize && lastFileSize != 0) {
            duplicateCount++
            if (duplicateCount >= maxDuplicates) {
                ; Delete the duplicate screenshots
                Loop duplicateCount {
                    dupIdx := idx - duplicateCount + A_Index
                    dupFile := Format("{}\{}_{:02d}.png", saveFolder, safeUrl, dupIdx)
                    if FileExist(dupFile)
                        FileDelete(dupFile)
                }
                MsgBox("Reached bottom of page (detected " duplicateCount " duplicate screenshots).`n" idx - duplicateCount " screenshots saved.`nNavigate to next page and press F8 to continue.")
                stopLoop := false
                return
            }
        } else {
            duplicateCount := 0  ; Reset counter if screenshots are different
        }
        lastFileSize := currentFileSize

        ; Check if user pressed Space to stop (after taking the screenshot)
        if (stopLoop) {
            MsgBox("Page finished. " A_Index " screenshots saved.`nNavigate to next page and press F8 to continue.")
            stopLoop := false
            return
        }

        ; ========= SCROLL DOWN FOR NEXT ITERATION =========
        if (idx < iterations)
        {
            Send("{WheelDown " scrollLines "}")
            Sleep(500) ; Wait for page elements to load after scrolling
        }
    }

    MsgBox("Reached maximum iterations (" iterations "). Screenshots saved in: " saveFolder)
    stopLoop := false
}


; ========= FUNCTIONS =========

; Remove characters that are illegal in Windows filenames
SanitizeFileName(name) {
    ; Correctly escaping the double-quote character (") using a backtick (`")
    invalid := ["\", "/", ":", "*", "?", "`"", "<", ">", "|"]
    for _, char in invalid
        name := StrReplace(name, char, "_")
    return Trim(name)
}

; Get the URL of the active browser tab by focusing the address bar and copying it
GetActivePageUrl() {
    ; Backup clipboard
    clipBackup := ClipboardAll()
    A_Clipboard := ""
    Sleep(50)

    ; Focus address bar and copy
    Send("^l")
    Sleep(100)
    Send("^c")
    ClipWait(0.5)

    url := A_Clipboard

    ; Restore clipboard
    A_Clipboard := clipBackup

    url := Trim(url)
    return url
}


; Capture the full primary screen and save to a PNG file using PowerShell
CaptureFullScreen(path) {
    psScript := "
    (
Add-Type -AssemblyName System.Windows.Forms,System.Drawing;
$bounds = [System.Windows.Forms.Screen]::PrimaryScreen.Bounds;
$bmp = New-Object System.Drawing.Bitmap($bounds.Width, $bounds.Height);
$g = [System.Drawing.Graphics]::FromImage($bmp);
$g.CopyFromScreen($bounds.Location, [System.Drawing.Point]::Empty, $bounds.Size);
$bmp.Save('{path}', [System.Drawing.Imaging.ImageFormat]::Png);
$g.Dispose();
$bmp.Dispose();
    )"

    psScript := StrReplace(psScript, "{path}", path)
    cmd := 'powershell -NoProfile -Command "' psScript '"'
    RunWait(cmd,, "Hide")
}

; Capture only the Chrome window and save to a PNG file using PowerShell
CaptureChromeWindow(path) {
    ; Get Chrome window position and size
    WinGetPos(&x, &y, &width, &height, "ahk_exe chrome.exe")
    
    psScript := "
    (
Add-Type -AssemblyName System.Windows.Forms,System.Drawing;
$x = {x};
$y = {y};
$width = {width};
$height = {height};
$bmp = New-Object System.Drawing.Bitmap($width, $height);
$g = [System.Drawing.Graphics]::FromImage($bmp);
$g.CopyFromScreen($x, $y, 0, 0, [System.Drawing.Size]::new($width, $height));
$bmp.Save('{path}', [System.Drawing.Imaging.ImageFormat]::Png);
$g.Dispose();
$bmp.Dispose();
    )"

    psScript := StrReplace(psScript, "{path}", path)
    psScript := StrReplace(psScript, "{x}", x)
    psScript := StrReplace(psScript, "{y}", y)
    psScript := StrReplace(psScript, "{width}", width)
    psScript := StrReplace(psScript, "{height}", height)
    
    cmd := 'powershell -NoProfile -Command "' psScript '"'
    RunWait(cmd,, "Hide")
}