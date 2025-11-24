#SingleInstance Force
#Requires AutoHotkey v2.0

; Show a warning when the script is activated
MsgBox("Dropdown Test Script is running.`nF9 = Test Dropdown Click`nCtrl+F8 = exit")

; ========= EXIT HOTKEY =========
^F8::
{
    MsgBox("Stopping test script and exiting.")
    ExitApp()
}

; ========= TEST HOTKEY =========
F9::
{
    ; ========= CONFIG (Copied from main script) =========
    ; *** FIX: Reverting to the relative path assuming image is in the 'input' folder ***
    imageToFind     := "input\answers_dropdown.PNG"
    imageWidth      := 45   ; You must verify this is correct
    imageHeight     := 30   ; You must verify this is correct

    ; Activate the target window before starting the test
    WinActivate("ahk_exe chrome.exe") ; <-- CHANGE 'chrome.exe' if needed
    Sleep(200)
    
    ; Use screen coordinates for searching and clicking
    CoordMode("Pixel", "Screen")
    CoordMode("Mouse", "Screen")
    
    MsgBox("Starting Dropdown Test. Please wait.")
    
    ; ========= IMAGE SEARCH AND CLICK TEST =========
    foundX := 0, foundY := 0
    ErrorLevel := 1

    ; *** IMPROVEMENT: Using *70 tolerance for better matching ***
    ImageSearch(&foundX, &foundY, 0, 0, A_ScreenWidth, A_ScreenHeight, "*70 " imageToFind)

    if (ErrorLevel = 0)     ; 0 = found
    {
        ; Calculate center coordinates
        clickX := foundX + imageWidth/2
        clickY := foundY + imageHeight/2
        
        MsgBox(Format("✅ Dropdown FOUND at X={}, Y={}. Mouse will move and click in 3 seconds...", foundX, foundY))
        
        ; Give yourself time to see the coordinates and move the mouse
        Sleep(3000)

        ; Move the mouse to the calculated center and click
        MouseMove(clickX, clickY, 10)
        Click("Left")
        
        MsgBox("Click attempted. Check if the dropdown opened.")
    }
    else 
    {
        MsgBox(Format("❌ Dropdown NOT FOUND. ErrorLevel is %ErrorLevel%. Check image path: '%imageToFind%'"))
    }
}