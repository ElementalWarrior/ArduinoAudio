^!b::
    ; WinActivate, Atmel Flip
    ; flip
    ; WinWaitActive, Atmel Flip
    send ^{u}
    send {Enter}
    Sleep 300
    title = 
    WinGetActiveTitle, title
    if(title = "com.atmel.flip.IspException") {
        Return
    }
    send ^{l}
    Sleep 300
    SendInput, AudioOutput.hex
    Send {Enter}
    Sleep 500
    ; WinSetTitle, Atmel Flip
    WinGetActiveTitle, title
    ; MsgBox, , %title%
    ControlClick, Run
    MouseGetPos, x,y
    Click, 62,442
    MouseMove, x,y
Return