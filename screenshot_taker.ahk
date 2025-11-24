#SingleInstance Force

; ===== EXIT HOTKEY =====
; website : https://www.iitianacademy.com/course/ib-dp-maths-ai-hl-ib-style-practice-questions-with-answer-topic-wise-paper-3/lessons/ahl-1-10-simplifying-expressions-ai-hl-paper-3/
^F8::ExitApp()


; ===== MAIN HOTKEY =====
F8::
{
    ; ========= CONFIG =========
    saveFolder         := "D:\Code\school\screenshots"  ; Folder for screenshots
    fileTitle          := "page"                           ; Filename prefix

    scrollSteps        := 25        ; How many times to scroll to bottom to load content
    scrollDelay        := 600       ; Delay between scrolls (ms)

    zoomOutSteps       := 6         ; How many times to zoom out
    zoomDelay          := 250       ; Delay between zoom steps (ms)

    devToolsDelay      := 900       ; Wait after opening DevTools (ms)
    commandMenuDelay   := 700       ; Wait after opening command menu (ms)
    saveDialogTimeout  := 10000     ; Max wait for Save As dialog (ms)
    afterSaveDelay     := 2500      ; Wait after pressing Enter to save (ms)
    ; ==========================

    if !DirExist(saveFolder)
        DirCreate(saveFolder)

    if !WinActive("ahk_exe chrome.exe") {
        MsgBox("Focus the Chrome window with the page you want to capture, then press F8.")
        return
    }

    Sleep(800)

    Send("^{Home}")
    Sleep(800)

    Loop scrollSteps
    {
        Send("{PgDn}")
        Sleep(scrollDelay)
    }

    Sleep(1500)

    Send("^{Home}")
    Sleep(800)

    Send("^0")
    Sleep(600)

    Loop zoomOutSteps
    {
        Send("^{-}")
        Sleep(zoomDelay)
    }

    Sleep(1200)

    Send("{F12}")
    Sleep(devToolsDelay)

    Send("^+p")
    Sleep(commandMenuDelay)

    SendText("capture full size screenshot")
    Sleep(900)
    Send("{Enter}")

    if !WinWait("Save As",, saveDialogTimeout)
    {
        MsgBox("Did not see a 'Save As' dialog.")
        Send("{F12}")
        return
    }

    timestamp := FormatTime(, "yyyyMMdd_HHmmss")
    fullPath  := saveFolder "\" fileTitle "_" timestamp ".png"

    Sleep(700)
    SendText(fullPath)
    Sleep(700)
    Send("{Enter}")

    Sleep(afterSaveDelay)

    Send("{F12}")

    MsgBox("Full-page screenshot saved:`n" fullPath)
}
