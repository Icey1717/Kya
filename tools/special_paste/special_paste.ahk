; special_paste.ahk
; Hotkey: Ctrl+Shift+V
; Reads clipboard, transforms it via transform.py, then pastes.
;
; Setup:
;   1. Install AutoHotkey v2  (https://www.autohotkey.com/)
;   2. Ensure Python 3 is on your PATH
;   3. Place this file and transform.py in the same directory
;   4. Double-click this script (or run: AutoHotkey64.exe special_paste.ahk)
;
; Add a new transform: edit transform.py and add a function to the TRANSFORMS list.

#Requires AutoHotkey v2.0
#SingleInstance Force

^e:: {
    text := A_Clipboard

    tempIn  := A_Temp "\kya_paste_in.txt"
    tempOut := A_Temp "\kya_paste_out.txt"

    tempErr  := A_Temp "\kya_paste_err.txt"

    try {
        f := FileOpen(tempIn, "w", "UTF-8-RAW")
        f.Write(text)
        f.Close()

        scriptDir := A_ScriptDir
        RunWait('cmd.exe /c python.exe -X utf8 "' scriptDir '\transform.py" < "' tempIn '" > "' tempOut '" 2> "' tempErr '"',, "Hide")

        errText := FileRead(tempErr, "UTF-8")
        if (errText != "") {
            MsgBox "Python error:`n" errText, "Special Paste", "Icon!"
            return
        }

        result := FileRead(tempOut, "UTF-8")
        A_Clipboard := result
        ClipWait(2)
        Send "^v"
        TrayTip "Transformed!", "Special Paste OK", "Mute"
    } catch Error as e {
        MsgBox "Error:`n" e.Message, "Special Paste", "Icon!"
    }
}
