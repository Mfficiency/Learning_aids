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
    saveFolder    := "D:\Code\school\screenshots"          ; Folder for screenshots
    imageToFind   := "input\answers_dropdown.PNG"          ; Image for ImageSearch
    iterations    := 10                                    ; How many times to repeat
    scrollLines   := 8                                     ; Mouse wheel steps after each shot

    MsgBox(Format("Starting capture loop with {} iterations.`nPress Ctrl+F8 to exit early.", iterations))

    ; Create folder if it does not exist
    if !DirExist(saveFolder)
        DirCreate(saveFolder)

    ; Get page URL once at the beginning and sanitize it for filename use
    pageUrl := GetActivePageUrl()
    if (pageUrl = "")
        pageUrl := "no_url_detected"

    ; Replace characters that are illegal in Windows filenames
    safeUrl := RegExReplace(pageUrl, "[\\/:*?""<>|]", "_")

    ; Use screen coordinates for searching and clicking
    CoordMode("Pixel", "Screen")
    CoordMode("Mouse", "Screen")

    Loop iterations
    {
        idx := A_Index
        ; Example: https___site_com_1-of-10.png
        fileName := Format("{}_{}-of-{}.png", safeUrl, idx, iterations)
        fullPath := saveFolder "\" fileName

        ; ========= IMAGE SEARCH AND CLICK =========
        foundX := 0, foundY := 0
        ErrorLevel := 1

        ImageSearch(&foundX, &foundY, 0, 0, A_ScreenWidth, A_ScreenHeight, imageToFind)

        if (ErrorLevel = 0)  ; 0 = found
        {
            MouseMove(foundX, foundY, 10)
            Click("Left")
            Sleep(500)
        }

        ; ========= SCREENSHOT =========
        CaptureFullScreen(fullPath)

        ; ========= SCROLL DOWN FOR NEXT ITERATION =========
        if (idx < iterations)
        {
            Send("{WheelDown " scrollLines "}")
            Sleep(700)
        }
    }

    MsgBox("Capture loop finished.")
}


; ========= FUNCTIONS =========

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

    ; Trim whitespace
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
