#SingleInstance Force

; Global flag to stop the loop
global stopLoop := false

; Show a warning when the script is activated
MsgBox("Screenshot helper is running.`nF8 = start capture`nSpace = stop current capture`nCtrl+F8 = exit")

; ========= EXIT HOTKEY =========
^F8::
{
    MsgBox("Stopping screenshot helper and exiting.")
    ExitApp()
}

; ========= STOP LOOP HOTKEY =========
Space::
{
    global stopLoop
    stopLoop := true
    ToolTip("Stopping after current screenshot...")
    SetTimer(() => ToolTip(), -2000)  ; Hide tooltip after 2 seconds
}

; ========= MAIN HOTKEY =========
F8::
{
    global stopLoop
    stopLoop := false  ; Reset the stop flag
    
    ; ========= CONFIG =========
    baseFolder      := "D:\Code\school\screenshots"          ; Base folder for screenshots
    imageToFind     := "input\answers_dropdown.PNG"          ; Image for ImageSearch
    iterations      := 10                                    ; How many times to repeat
    scrollLines     := 8                                     ; Mouse wheel steps after each shot
    
    ; *** IMPORTANT: Measure your dropdown image file and adjust these dimensions ***
    imageWidth      := 288                                   ; Example width of the dropdown image
    imageHeight     := 36                                    ; Example height of the dropdown image

    MsgBox(Format("Starting capture loop with {} iterations.`nPress Space to stop early.", iterations))

    ; *** IMPROVEMENT: Activate the target window before starting the loop ***
    WinActivate("ahk_exe chrome.exe") ; <-- CHANGE 'chrome.exe' to your browser's executable (e.g., 'msedge.exe', 'firefox.exe')
    Sleep(200)

    ; Get page URL once at the beginning and sanitize it for filename use
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

    ; Use screen coordinates for searching and clicking
    CoordMode("Pixel", "Screen")
    CoordMode("Mouse", "Screen")

    Loop iterations
    {
        ; Check if user pressed Space to stop
        if (stopLoop) {
            MsgBox("Capture stopped by user at screenshot " A_Index - 1 " of " iterations ".")
            stopLoop := false
            return
        }
        
        idx := A_Index
        ; Example: 1-of-10.png (simpler naming since folder already has the page name)
        fileName := Format("{}-of-{}.png", idx, iterations)
        fullPath := saveFolder "\" fileName

        ; ========= SCREENSHOT =========
        CaptureFullScreen(fullPath)

        ; ========= SCROLL DOWN FOR NEXT ITERATION =========
        if (idx < iterations)
        {
            Send("{WheelDown " scrollLines "}")
            Sleep(1500) ; *** IMPROVEMENT: Increased sleep to allow page elements to load after scrolling ***
        }
    }

    MsgBox("Capture loop finished. Screenshots saved in: " saveFolder)
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
    ; Multi line PowerShell script - no double quotes inside
    ; NOTE: This version uses simple string replacement for the path, assuming no spaces in 'path' or that
    ; the script handles it correctly in execution environment.
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