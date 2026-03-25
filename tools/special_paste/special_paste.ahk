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

    try {
        f := FileOpen(tempIn, "w", "UTF-8")
        f.Write(text)
        f.Close()

        scriptDir := A_ScriptDir
        RunWait('cmd.exe /c python.exe "' scriptDir '\transform.py" < "' tempIn '" > "' tempOut '"',, "Hide")

        result := FileRead(tempOut, "UTF-8")
        if (result != "") {
            A_Clipboard := result
            ClipWait(2)
        }
        Send "^v"
        TrayTip "Transformed!", "Special Paste OK", "Mute"
    } catch Error as e {
        MsgBox "Error:`n" e.Message, "Special Paste", "Icon!"
    }
}
