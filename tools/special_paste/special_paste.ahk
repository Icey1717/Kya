; special_paste.ahk
; Hotkey: Ctrl+E
; Reads clipboard, transforms it via transform.py, then pastes.
;
; Setup:
;   1. Install AutoHotkey v2  (https://www.autohotkey.com/)
;   2. Ensure Python 3 is on your PATH
;   3. Keep this file, transform.py, symbol_index.py and vector_assignments.py together
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
        exitCode := RunWait('cmd.exe /c python.exe -X utf8 "' scriptDir '\transform.py" < "' tempIn '" > "' tempOut '" 2> "' tempErr '"',, "Hide")

        errText := FileRead(tempErr, "UTF-8")
        if (exitCode != 0 || errText != "") {
            MsgBox "Python failed (exit " exitCode "):`n" errText, "Special Paste", "Icon!"
            return
        }

        result := FileRead(tempOut, "UTF-8")
        if (text != "" && result == "") {
            throw Error("Transformer returned empty output; paste cancelled.")
        }
        A_Clipboard := result
        if (!ClipWait(2)) {
            throw Error("Timed out waiting for the transformed clipboard.")
        }
        Send "^v"
        TrayTip "Transformed!", "Special Paste OK", "Mute"
    } catch Error as e {
        MsgBox "Error:`n" e.Message, "Special Paste", "Icon!"
    }
}
