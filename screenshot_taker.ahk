#SingleInstance Force

; Show a warning when the script is activated
MsgBox("Screenshot helper is running.`nF8 = start capture`nCtrl+F8 = exit")

; ========= EXIT HOTKEY =========
^F8::
{
    MsgBox("Stopping screenshot helper and exiting.")
    ExitApp()
}


; ========= MAIN HOTKEY =========
F8::
{
    ; ========= CONFIG =========
    saveFolder      := "D:\Code\school\screenshots"          ; Folder for screenshots
    imageToFind     := "input\answers_dropdown.PNG"          ; Image for ImageSearch
    iterations      := 10                                    ; How many times to repeat
    scrollLines     := 8                                     ; Mouse wheel steps after each shot
    
    ; *** IMPORTANT: Measure your dropdown image file and adjust these dimensions ***
    imageWidth      := 288                                    ; Example width of the dropdown image
    imageHeight     := 36                                     ; Example height of the dropdown image

    MsgBox(Format("Starting capture loop with {} iterations.`nPress Ctrl+F8 to exit early.", iterations))

    ; *** IMPROVEMENT: Activate the target window before starting the loop ***
    WinActivate("ahk_exe chrome.exe") ; <-- CHANGE 'chrome.exe' to your browser's executable (e.g., 'msedge.exe', 'firefox.exe')
    Sleep(200)

    ; Create folder if it does not exist
    if !DirExist(saveFolder)
        DirCreate(saveFolder)

    ; Get page URL once at the beginning and sanitize it for filename use
    pageUrl := GetActivePageUrl()
    if (pageUrl = "") {
        safeUrl := "no_url_detected"
    } else {
        ; *** NEW LOGIC: Extract text after the last forward slash (/) ***
        urlParts := StrSplit(pageUrl, "/")
        lastPart := urlParts[urlParts.Length]
        
        ; Sanitize only the last part of the URL
        safeUrl := SanitizeFileName(lastPart)
        
        ; Use "no_url_detected" if the last part is empty after splitting
        if (safeUrl = "")
            safeUrl := "no_url_detected"
    }

    ; Use screen coordinates for searching and clicking
    CoordMode("Pixel", "Screen")
    CoordMode("Mouse", "Screen")

    Loop iterations
    {
        idx := A_Index
        ; Example: page_slug-1-of-10.png
        fileName := Format("{}_{}-of-{}.png", safeUrl, idx, iterations)
        fullPath := saveFolder "\" fileName

        ; ========= IMAGE SEARCH AND CLICK =========
        foundX := 0, foundY := 0
        ErrorLevel := 1

        ; Added *50 to tolerance for better matching on potentially anti-aliased elements
        ImageSearch(&foundX, &foundY, 0, 0, A_ScreenWidth, A_ScreenHeight, "*50 " imageToFind)

        if (ErrorLevel = 0)     ; 0 = found
        {
            ; *** FIX: Calculate center coordinates to ensure a successful click ***
            clickX := foundX + imageWidth/2
            clickY := foundY + imageHeight/2

            MouseMove(clickX, clickY, 10)
            Click("Left")
            Sleep(500)
        }

        ; ========= SCREENSHOT =========
        CaptureFullScreen(fullPath)

        ; ========= SCROLL DOWN FOR NEXT ITERATION =========
        if (idx < iterations)
        {
            Send("{WheelDown " scrollLines "}")
            Sleep(1500) ; *** IMPROVEMENT: Increased sleep to allow page elements to load after scrolling ***
        }
    }

    MsgBox("Capture loop finished.")
}


; ========= FUNCTIONS =========

; Remove characters that are illegal in Windows filenames
SanitizeFileName(name) {
    ; **FIXED LINE 79:** Correctly escaping the double-quote character (") using a backtick (`")
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
    ; Multi line PowerShell script - no double quotes inside
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

    ; Wrap script for powershell -Command
    cmd := 'powershell -NoProfile -Command "' psScript '"'
    RunWait(cmd,, "Hide")
}