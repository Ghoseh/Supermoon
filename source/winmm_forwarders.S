.intel_syntax noprefix
.text
.globl WinmmStub0
WinmmStub0:
  cmp qword ptr [rip+winmm_targets+0],0
  je .Lslow0
  jmp qword ptr [rip+winmm_targets+0]
.Lslow0:
  mov r10d,0
  jmp WinmmResolveBridge
.globl WinmmStub1
WinmmStub1:
  cmp qword ptr [rip+winmm_targets+8],0
  je .Lslow1
  jmp qword ptr [rip+winmm_targets+8]
.Lslow1:
  mov r10d,1
  jmp WinmmResolveBridge
.globl WinmmStub2
WinmmStub2:
  cmp qword ptr [rip+winmm_targets+16],0
  je .Lslow2
  jmp qword ptr [rip+winmm_targets+16]
.Lslow2:
  mov r10d,2
  jmp WinmmResolveBridge
.globl WinmmStub3
WinmmStub3:
  cmp qword ptr [rip+winmm_targets+24],0
  je .Lslow3
  jmp qword ptr [rip+winmm_targets+24]
.Lslow3:
  mov r10d,3
  jmp WinmmResolveBridge
.globl WinmmStub4
WinmmStub4:
  cmp qword ptr [rip+winmm_targets+32],0
  je .Lslow4
  jmp qword ptr [rip+winmm_targets+32]
.Lslow4:
  mov r10d,4
  jmp WinmmResolveBridge
.globl WinmmStub5
WinmmStub5:
  cmp qword ptr [rip+winmm_targets+40],0
  je .Lslow5
  jmp qword ptr [rip+winmm_targets+40]
.Lslow5:
  mov r10d,5
  jmp WinmmResolveBridge
.globl WinmmStub6
WinmmStub6:
  cmp qword ptr [rip+winmm_targets+48],0
  je .Lslow6
  jmp qword ptr [rip+winmm_targets+48]
.Lslow6:
  mov r10d,6
  jmp WinmmResolveBridge
.globl WinmmStub7
WinmmStub7:
  cmp qword ptr [rip+winmm_targets+56],0
  je .Lslow7
  jmp qword ptr [rip+winmm_targets+56]
.Lslow7:
  mov r10d,7
  jmp WinmmResolveBridge
.globl WinmmStub8
WinmmStub8:
  cmp qword ptr [rip+winmm_targets+64],0
  je .Lslow8
  jmp qword ptr [rip+winmm_targets+64]
.Lslow8:
  mov r10d,8
  jmp WinmmResolveBridge
.globl WinmmStub9
WinmmStub9:
  cmp qword ptr [rip+winmm_targets+72],0
  je .Lslow9
  jmp qword ptr [rip+winmm_targets+72]
.Lslow9:
  mov r10d,9
  jmp WinmmResolveBridge
.globl WinmmStub10
WinmmStub10:
  cmp qword ptr [rip+winmm_targets+80],0
  je .Lslow10
  jmp qword ptr [rip+winmm_targets+80]
.Lslow10:
  mov r10d,10
  jmp WinmmResolveBridge
.globl WinmmStub11
WinmmStub11:
  cmp qword ptr [rip+winmm_targets+88],0
  je .Lslow11
  jmp qword ptr [rip+winmm_targets+88]
.Lslow11:
  mov r10d,11
  jmp WinmmResolveBridge
.globl WinmmStub12
WinmmStub12:
  cmp qword ptr [rip+winmm_targets+96],0
  je .Lslow12
  jmp qword ptr [rip+winmm_targets+96]
.Lslow12:
  mov r10d,12
  jmp WinmmResolveBridge
.globl WinmmStub13
WinmmStub13:
  cmp qword ptr [rip+winmm_targets+104],0
  je .Lslow13
  jmp qword ptr [rip+winmm_targets+104]
.Lslow13:
  mov r10d,13
  jmp WinmmResolveBridge
.globl WinmmStub14
WinmmStub14:
  cmp qword ptr [rip+winmm_targets+112],0
  je .Lslow14
  jmp qword ptr [rip+winmm_targets+112]
.Lslow14:
  mov r10d,14
  jmp WinmmResolveBridge
.globl WinmmStub15
WinmmStub15:
  cmp qword ptr [rip+winmm_targets+120],0
  je .Lslow15
  jmp qword ptr [rip+winmm_targets+120]
.Lslow15:
  mov r10d,15
  jmp WinmmResolveBridge
.globl WinmmStub16
WinmmStub16:
  cmp qword ptr [rip+winmm_targets+128],0
  je .Lslow16
  jmp qword ptr [rip+winmm_targets+128]
.Lslow16:
  mov r10d,16
  jmp WinmmResolveBridge
.globl WinmmStub17
WinmmStub17:
  cmp qword ptr [rip+winmm_targets+136],0
  je .Lslow17
  jmp qword ptr [rip+winmm_targets+136]
.Lslow17:
  mov r10d,17
  jmp WinmmResolveBridge
.globl WinmmStub18
WinmmStub18:
  cmp qword ptr [rip+winmm_targets+144],0
  je .Lslow18
  jmp qword ptr [rip+winmm_targets+144]
.Lslow18:
  mov r10d,18
  jmp WinmmResolveBridge
.globl WinmmStub19
WinmmStub19:
  cmp qword ptr [rip+winmm_targets+152],0
  je .Lslow19
  jmp qword ptr [rip+winmm_targets+152]
.Lslow19:
  mov r10d,19
  jmp WinmmResolveBridge
.globl WinmmStub20
WinmmStub20:
  cmp qword ptr [rip+winmm_targets+160],0
  je .Lslow20
  jmp qword ptr [rip+winmm_targets+160]
.Lslow20:
  mov r10d,20
  jmp WinmmResolveBridge
.globl WinmmStub21
WinmmStub21:
  cmp qword ptr [rip+winmm_targets+168],0
  je .Lslow21
  jmp qword ptr [rip+winmm_targets+168]
.Lslow21:
  mov r10d,21
  jmp WinmmResolveBridge
.globl WinmmStub22
WinmmStub22:
  cmp qword ptr [rip+winmm_targets+176],0
  je .Lslow22
  jmp qword ptr [rip+winmm_targets+176]
.Lslow22:
  mov r10d,22
  jmp WinmmResolveBridge
.globl WinmmStub23
WinmmStub23:
  cmp qword ptr [rip+winmm_targets+184],0
  je .Lslow23
  jmp qword ptr [rip+winmm_targets+184]
.Lslow23:
  mov r10d,23
  jmp WinmmResolveBridge
.globl WinmmStub24
WinmmStub24:
  cmp qword ptr [rip+winmm_targets+192],0
  je .Lslow24
  jmp qword ptr [rip+winmm_targets+192]
.Lslow24:
  mov r10d,24
  jmp WinmmResolveBridge
.globl WinmmStub25
WinmmStub25:
  cmp qword ptr [rip+winmm_targets+200],0
  je .Lslow25
  jmp qword ptr [rip+winmm_targets+200]
.Lslow25:
  mov r10d,25
  jmp WinmmResolveBridge
.globl WinmmStub26
WinmmStub26:
  cmp qword ptr [rip+winmm_targets+208],0
  je .Lslow26
  jmp qword ptr [rip+winmm_targets+208]
.Lslow26:
  mov r10d,26
  jmp WinmmResolveBridge
.globl WinmmStub27
WinmmStub27:
  cmp qword ptr [rip+winmm_targets+216],0
  je .Lslow27
  jmp qword ptr [rip+winmm_targets+216]
.Lslow27:
  mov r10d,27
  jmp WinmmResolveBridge
.globl WinmmStub28
WinmmStub28:
  cmp qword ptr [rip+winmm_targets+224],0
  je .Lslow28
  jmp qword ptr [rip+winmm_targets+224]
.Lslow28:
  mov r10d,28
  jmp WinmmResolveBridge
.globl WinmmStub29
WinmmStub29:
  cmp qword ptr [rip+winmm_targets+232],0
  je .Lslow29
  jmp qword ptr [rip+winmm_targets+232]
.Lslow29:
  mov r10d,29
  jmp WinmmResolveBridge
.globl WinmmStub30
WinmmStub30:
  cmp qword ptr [rip+winmm_targets+240],0
  je .Lslow30
  jmp qword ptr [rip+winmm_targets+240]
.Lslow30:
  mov r10d,30
  jmp WinmmResolveBridge
.globl WinmmStub31
WinmmStub31:
  cmp qword ptr [rip+winmm_targets+248],0
  je .Lslow31
  jmp qword ptr [rip+winmm_targets+248]
.Lslow31:
  mov r10d,31
  jmp WinmmResolveBridge
.globl WinmmStub32
WinmmStub32:
  cmp qword ptr [rip+winmm_targets+256],0
  je .Lslow32
  jmp qword ptr [rip+winmm_targets+256]
.Lslow32:
  mov r10d,32
  jmp WinmmResolveBridge
.globl WinmmStub33
WinmmStub33:
  cmp qword ptr [rip+winmm_targets+264],0
  je .Lslow33
  jmp qword ptr [rip+winmm_targets+264]
.Lslow33:
  mov r10d,33
  jmp WinmmResolveBridge
.globl WinmmStub34
WinmmStub34:
  cmp qword ptr [rip+winmm_targets+272],0
  je .Lslow34
  jmp qword ptr [rip+winmm_targets+272]
.Lslow34:
  mov r10d,34
  jmp WinmmResolveBridge
.globl WinmmStub35
WinmmStub35:
  cmp qword ptr [rip+winmm_targets+280],0
  je .Lslow35
  jmp qword ptr [rip+winmm_targets+280]
.Lslow35:
  mov r10d,35
  jmp WinmmResolveBridge
.globl WinmmStub36
WinmmStub36:
  cmp qword ptr [rip+winmm_targets+288],0
  je .Lslow36
  jmp qword ptr [rip+winmm_targets+288]
.Lslow36:
  mov r10d,36
  jmp WinmmResolveBridge
.globl WinmmStub37
WinmmStub37:
  cmp qword ptr [rip+winmm_targets+296],0
  je .Lslow37
  jmp qword ptr [rip+winmm_targets+296]
.Lslow37:
  mov r10d,37
  jmp WinmmResolveBridge
.globl WinmmStub38
WinmmStub38:
  cmp qword ptr [rip+winmm_targets+304],0
  je .Lslow38
  jmp qword ptr [rip+winmm_targets+304]
.Lslow38:
  mov r10d,38
  jmp WinmmResolveBridge
.globl WinmmStub39
WinmmStub39:
  cmp qword ptr [rip+winmm_targets+312],0
  je .Lslow39
  jmp qword ptr [rip+winmm_targets+312]
.Lslow39:
  mov r10d,39
  jmp WinmmResolveBridge
.globl WinmmStub40
WinmmStub40:
  cmp qword ptr [rip+winmm_targets+320],0
  je .Lslow40
  jmp qword ptr [rip+winmm_targets+320]
.Lslow40:
  mov r10d,40
  jmp WinmmResolveBridge
.globl WinmmStub41
WinmmStub41:
  cmp qword ptr [rip+winmm_targets+328],0
  je .Lslow41
  jmp qword ptr [rip+winmm_targets+328]
.Lslow41:
  mov r10d,41
  jmp WinmmResolveBridge
.globl WinmmStub42
WinmmStub42:
  cmp qword ptr [rip+winmm_targets+336],0
  je .Lslow42
  jmp qword ptr [rip+winmm_targets+336]
.Lslow42:
  mov r10d,42
  jmp WinmmResolveBridge
.globl WinmmStub43
WinmmStub43:
  cmp qword ptr [rip+winmm_targets+344],0
  je .Lslow43
  jmp qword ptr [rip+winmm_targets+344]
.Lslow43:
  mov r10d,43
  jmp WinmmResolveBridge
.globl WinmmStub44
WinmmStub44:
  cmp qword ptr [rip+winmm_targets+352],0
  je .Lslow44
  jmp qword ptr [rip+winmm_targets+352]
.Lslow44:
  mov r10d,44
  jmp WinmmResolveBridge
.globl WinmmStub45
WinmmStub45:
  cmp qword ptr [rip+winmm_targets+360],0
  je .Lslow45
  jmp qword ptr [rip+winmm_targets+360]
.Lslow45:
  mov r10d,45
  jmp WinmmResolveBridge
.globl WinmmStub46
WinmmStub46:
  cmp qword ptr [rip+winmm_targets+368],0
  je .Lslow46
  jmp qword ptr [rip+winmm_targets+368]
.Lslow46:
  mov r10d,46
  jmp WinmmResolveBridge
.globl WinmmStub47
WinmmStub47:
  cmp qword ptr [rip+winmm_targets+376],0
  je .Lslow47
  jmp qword ptr [rip+winmm_targets+376]
.Lslow47:
  mov r10d,47
  jmp WinmmResolveBridge
.globl WinmmStub48
WinmmStub48:
  cmp qword ptr [rip+winmm_targets+384],0
  je .Lslow48
  jmp qword ptr [rip+winmm_targets+384]
.Lslow48:
  mov r10d,48
  jmp WinmmResolveBridge
.globl WinmmStub49
WinmmStub49:
  cmp qword ptr [rip+winmm_targets+392],0
  je .Lslow49
  jmp qword ptr [rip+winmm_targets+392]
.Lslow49:
  mov r10d,49
  jmp WinmmResolveBridge
.globl WinmmStub50
WinmmStub50:
  cmp qword ptr [rip+winmm_targets+400],0
  je .Lslow50
  jmp qword ptr [rip+winmm_targets+400]
.Lslow50:
  mov r10d,50
  jmp WinmmResolveBridge
.globl WinmmStub51
WinmmStub51:
  cmp qword ptr [rip+winmm_targets+408],0
  je .Lslow51
  jmp qword ptr [rip+winmm_targets+408]
.Lslow51:
  mov r10d,51
  jmp WinmmResolveBridge
.globl WinmmStub52
WinmmStub52:
  cmp qword ptr [rip+winmm_targets+416],0
  je .Lslow52
  jmp qword ptr [rip+winmm_targets+416]
.Lslow52:
  mov r10d,52
  jmp WinmmResolveBridge
.globl WinmmStub53
WinmmStub53:
  cmp qword ptr [rip+winmm_targets+424],0
  je .Lslow53
  jmp qword ptr [rip+winmm_targets+424]
.Lslow53:
  mov r10d,53
  jmp WinmmResolveBridge
.globl WinmmStub54
WinmmStub54:
  cmp qword ptr [rip+winmm_targets+432],0
  je .Lslow54
  jmp qword ptr [rip+winmm_targets+432]
.Lslow54:
  mov r10d,54
  jmp WinmmResolveBridge
.globl WinmmStub55
WinmmStub55:
  cmp qword ptr [rip+winmm_targets+440],0
  je .Lslow55
  jmp qword ptr [rip+winmm_targets+440]
.Lslow55:
  mov r10d,55
  jmp WinmmResolveBridge
.globl WinmmStub56
WinmmStub56:
  cmp qword ptr [rip+winmm_targets+448],0
  je .Lslow56
  jmp qword ptr [rip+winmm_targets+448]
.Lslow56:
  mov r10d,56
  jmp WinmmResolveBridge
.globl WinmmStub57
WinmmStub57:
  cmp qword ptr [rip+winmm_targets+456],0
  je .Lslow57
  jmp qword ptr [rip+winmm_targets+456]
.Lslow57:
  mov r10d,57
  jmp WinmmResolveBridge
.globl WinmmStub58
WinmmStub58:
  cmp qword ptr [rip+winmm_targets+464],0
  je .Lslow58
  jmp qword ptr [rip+winmm_targets+464]
.Lslow58:
  mov r10d,58
  jmp WinmmResolveBridge
.globl WinmmStub59
WinmmStub59:
  cmp qword ptr [rip+winmm_targets+472],0
  je .Lslow59
  jmp qword ptr [rip+winmm_targets+472]
.Lslow59:
  mov r10d,59
  jmp WinmmResolveBridge
.globl WinmmStub60
WinmmStub60:
  cmp qword ptr [rip+winmm_targets+480],0
  je .Lslow60
  jmp qword ptr [rip+winmm_targets+480]
.Lslow60:
  mov r10d,60
  jmp WinmmResolveBridge
.globl WinmmStub61
WinmmStub61:
  cmp qword ptr [rip+winmm_targets+488],0
  je .Lslow61
  jmp qword ptr [rip+winmm_targets+488]
.Lslow61:
  mov r10d,61
  jmp WinmmResolveBridge
.globl WinmmStub62
WinmmStub62:
  cmp qword ptr [rip+winmm_targets+496],0
  je .Lslow62
  jmp qword ptr [rip+winmm_targets+496]
.Lslow62:
  mov r10d,62
  jmp WinmmResolveBridge
.globl WinmmStub63
WinmmStub63:
  cmp qword ptr [rip+winmm_targets+504],0
  je .Lslow63
  jmp qword ptr [rip+winmm_targets+504]
.Lslow63:
  mov r10d,63
  jmp WinmmResolveBridge
.globl WinmmStub64
WinmmStub64:
  cmp qword ptr [rip+winmm_targets+512],0
  je .Lslow64
  jmp qword ptr [rip+winmm_targets+512]
.Lslow64:
  mov r10d,64
  jmp WinmmResolveBridge
.globl WinmmStub65
WinmmStub65:
  cmp qword ptr [rip+winmm_targets+520],0
  je .Lslow65
  jmp qword ptr [rip+winmm_targets+520]
.Lslow65:
  mov r10d,65
  jmp WinmmResolveBridge
.globl WinmmStub66
WinmmStub66:
  cmp qword ptr [rip+winmm_targets+528],0
  je .Lslow66
  jmp qword ptr [rip+winmm_targets+528]
.Lslow66:
  mov r10d,66
  jmp WinmmResolveBridge
.globl WinmmStub67
WinmmStub67:
  cmp qword ptr [rip+winmm_targets+536],0
  je .Lslow67
  jmp qword ptr [rip+winmm_targets+536]
.Lslow67:
  mov r10d,67
  jmp WinmmResolveBridge
.globl WinmmStub68
WinmmStub68:
  cmp qword ptr [rip+winmm_targets+544],0
  je .Lslow68
  jmp qword ptr [rip+winmm_targets+544]
.Lslow68:
  mov r10d,68
  jmp WinmmResolveBridge
.globl WinmmStub69
WinmmStub69:
  cmp qword ptr [rip+winmm_targets+552],0
  je .Lslow69
  jmp qword ptr [rip+winmm_targets+552]
.Lslow69:
  mov r10d,69
  jmp WinmmResolveBridge
.globl WinmmStub70
WinmmStub70:
  cmp qword ptr [rip+winmm_targets+560],0
  je .Lslow70
  jmp qword ptr [rip+winmm_targets+560]
.Lslow70:
  mov r10d,70
  jmp WinmmResolveBridge
.globl WinmmStub71
WinmmStub71:
  cmp qword ptr [rip+winmm_targets+568],0
  je .Lslow71
  jmp qword ptr [rip+winmm_targets+568]
.Lslow71:
  mov r10d,71
  jmp WinmmResolveBridge
.globl WinmmStub72
WinmmStub72:
  cmp qword ptr [rip+winmm_targets+576],0
  je .Lslow72
  jmp qword ptr [rip+winmm_targets+576]
.Lslow72:
  mov r10d,72
  jmp WinmmResolveBridge
.globl WinmmStub73
WinmmStub73:
  cmp qword ptr [rip+winmm_targets+584],0
  je .Lslow73
  jmp qword ptr [rip+winmm_targets+584]
.Lslow73:
  mov r10d,73
  jmp WinmmResolveBridge
.globl WinmmStub74
WinmmStub74:
  cmp qword ptr [rip+winmm_targets+592],0
  je .Lslow74
  jmp qword ptr [rip+winmm_targets+592]
.Lslow74:
  mov r10d,74
  jmp WinmmResolveBridge
.globl WinmmStub75
WinmmStub75:
  cmp qword ptr [rip+winmm_targets+600],0
  je .Lslow75
  jmp qword ptr [rip+winmm_targets+600]
.Lslow75:
  mov r10d,75
  jmp WinmmResolveBridge
.globl WinmmStub76
WinmmStub76:
  cmp qword ptr [rip+winmm_targets+608],0
  je .Lslow76
  jmp qword ptr [rip+winmm_targets+608]
.Lslow76:
  mov r10d,76
  jmp WinmmResolveBridge
.globl WinmmStub77
WinmmStub77:
  cmp qword ptr [rip+winmm_targets+616],0
  je .Lslow77
  jmp qword ptr [rip+winmm_targets+616]
.Lslow77:
  mov r10d,77
  jmp WinmmResolveBridge
.globl WinmmStub78
WinmmStub78:
  cmp qword ptr [rip+winmm_targets+624],0
  je .Lslow78
  jmp qword ptr [rip+winmm_targets+624]
.Lslow78:
  mov r10d,78
  jmp WinmmResolveBridge
.globl WinmmStub79
WinmmStub79:
  cmp qword ptr [rip+winmm_targets+632],0
  je .Lslow79
  jmp qword ptr [rip+winmm_targets+632]
.Lslow79:
  mov r10d,79
  jmp WinmmResolveBridge
.globl WinmmStub80
WinmmStub80:
  cmp qword ptr [rip+winmm_targets+640],0
  je .Lslow80
  jmp qword ptr [rip+winmm_targets+640]
.Lslow80:
  mov r10d,80
  jmp WinmmResolveBridge
.globl WinmmStub81
WinmmStub81:
  cmp qword ptr [rip+winmm_targets+648],0
  je .Lslow81
  jmp qword ptr [rip+winmm_targets+648]
.Lslow81:
  mov r10d,81
  jmp WinmmResolveBridge
.globl WinmmStub82
WinmmStub82:
  cmp qword ptr [rip+winmm_targets+656],0
  je .Lslow82
  jmp qword ptr [rip+winmm_targets+656]
.Lslow82:
  mov r10d,82
  jmp WinmmResolveBridge
.globl WinmmStub83
WinmmStub83:
  cmp qword ptr [rip+winmm_targets+664],0
  je .Lslow83
  jmp qword ptr [rip+winmm_targets+664]
.Lslow83:
  mov r10d,83
  jmp WinmmResolveBridge
.globl WinmmStub84
WinmmStub84:
  cmp qword ptr [rip+winmm_targets+672],0
  je .Lslow84
  jmp qword ptr [rip+winmm_targets+672]
.Lslow84:
  mov r10d,84
  jmp WinmmResolveBridge
.globl WinmmStub85
WinmmStub85:
  cmp qword ptr [rip+winmm_targets+680],0
  je .Lslow85
  jmp qword ptr [rip+winmm_targets+680]
.Lslow85:
  mov r10d,85
  jmp WinmmResolveBridge
.globl WinmmStub86
WinmmStub86:
  cmp qword ptr [rip+winmm_targets+688],0
  je .Lslow86
  jmp qword ptr [rip+winmm_targets+688]
.Lslow86:
  mov r10d,86
  jmp WinmmResolveBridge
.globl WinmmStub87
WinmmStub87:
  cmp qword ptr [rip+winmm_targets+696],0
  je .Lslow87
  jmp qword ptr [rip+winmm_targets+696]
.Lslow87:
  mov r10d,87
  jmp WinmmResolveBridge
.globl WinmmStub88
WinmmStub88:
  cmp qword ptr [rip+winmm_targets+704],0
  je .Lslow88
  jmp qword ptr [rip+winmm_targets+704]
.Lslow88:
  mov r10d,88
  jmp WinmmResolveBridge
.globl WinmmStub89
WinmmStub89:
  cmp qword ptr [rip+winmm_targets+712],0
  je .Lslow89
  jmp qword ptr [rip+winmm_targets+712]
.Lslow89:
  mov r10d,89
  jmp WinmmResolveBridge
.globl WinmmStub90
WinmmStub90:
  cmp qword ptr [rip+winmm_targets+720],0
  je .Lslow90
  jmp qword ptr [rip+winmm_targets+720]
.Lslow90:
  mov r10d,90
  jmp WinmmResolveBridge
.globl WinmmStub91
WinmmStub91:
  cmp qword ptr [rip+winmm_targets+728],0
  je .Lslow91
  jmp qword ptr [rip+winmm_targets+728]
.Lslow91:
  mov r10d,91
  jmp WinmmResolveBridge
.globl WinmmStub92
WinmmStub92:
  cmp qword ptr [rip+winmm_targets+736],0
  je .Lslow92
  jmp qword ptr [rip+winmm_targets+736]
.Lslow92:
  mov r10d,92
  jmp WinmmResolveBridge
.globl WinmmStub93
WinmmStub93:
  cmp qword ptr [rip+winmm_targets+744],0
  je .Lslow93
  jmp qword ptr [rip+winmm_targets+744]
.Lslow93:
  mov r10d,93
  jmp WinmmResolveBridge
.globl WinmmStub94
WinmmStub94:
  cmp qword ptr [rip+winmm_targets+752],0
  je .Lslow94
  jmp qword ptr [rip+winmm_targets+752]
.Lslow94:
  mov r10d,94
  jmp WinmmResolveBridge
.globl WinmmStub95
WinmmStub95:
  cmp qword ptr [rip+winmm_targets+760],0
  je .Lslow95
  jmp qword ptr [rip+winmm_targets+760]
.Lslow95:
  mov r10d,95
  jmp WinmmResolveBridge
.globl WinmmStub96
WinmmStub96:
  cmp qword ptr [rip+winmm_targets+768],0
  je .Lslow96
  jmp qword ptr [rip+winmm_targets+768]
.Lslow96:
  mov r10d,96
  jmp WinmmResolveBridge
.globl WinmmStub97
WinmmStub97:
  cmp qword ptr [rip+winmm_targets+776],0
  je .Lslow97
  jmp qword ptr [rip+winmm_targets+776]
.Lslow97:
  mov r10d,97
  jmp WinmmResolveBridge
.globl WinmmStub98
WinmmStub98:
  cmp qword ptr [rip+winmm_targets+784],0
  je .Lslow98
  jmp qword ptr [rip+winmm_targets+784]
.Lslow98:
  mov r10d,98
  jmp WinmmResolveBridge
.globl WinmmStub99
WinmmStub99:
  cmp qword ptr [rip+winmm_targets+792],0
  je .Lslow99
  jmp qword ptr [rip+winmm_targets+792]
.Lslow99:
  mov r10d,99
  jmp WinmmResolveBridge
.globl WinmmStub100
WinmmStub100:
  cmp qword ptr [rip+winmm_targets+800],0
  je .Lslow100
  jmp qword ptr [rip+winmm_targets+800]
.Lslow100:
  mov r10d,100
  jmp WinmmResolveBridge
.globl WinmmStub101
WinmmStub101:
  cmp qword ptr [rip+winmm_targets+808],0
  je .Lslow101
  jmp qword ptr [rip+winmm_targets+808]
.Lslow101:
  mov r10d,101
  jmp WinmmResolveBridge
.globl WinmmStub102
WinmmStub102:
  cmp qword ptr [rip+winmm_targets+816],0
  je .Lslow102
  jmp qword ptr [rip+winmm_targets+816]
.Lslow102:
  mov r10d,102
  jmp WinmmResolveBridge
.globl WinmmStub103
WinmmStub103:
  cmp qword ptr [rip+winmm_targets+824],0
  je .Lslow103
  jmp qword ptr [rip+winmm_targets+824]
.Lslow103:
  mov r10d,103
  jmp WinmmResolveBridge
.globl WinmmStub104
WinmmStub104:
  cmp qword ptr [rip+winmm_targets+832],0
  je .Lslow104
  jmp qword ptr [rip+winmm_targets+832]
.Lslow104:
  mov r10d,104
  jmp WinmmResolveBridge
.globl WinmmStub105
WinmmStub105:
  cmp qword ptr [rip+winmm_targets+840],0
  je .Lslow105
  jmp qword ptr [rip+winmm_targets+840]
.Lslow105:
  mov r10d,105
  jmp WinmmResolveBridge
.globl WinmmStub106
WinmmStub106:
  cmp qword ptr [rip+winmm_targets+848],0
  je .Lslow106
  jmp qword ptr [rip+winmm_targets+848]
.Lslow106:
  mov r10d,106
  jmp WinmmResolveBridge
.globl WinmmStub107
WinmmStub107:
  cmp qword ptr [rip+winmm_targets+856],0
  je .Lslow107
  jmp qword ptr [rip+winmm_targets+856]
.Lslow107:
  mov r10d,107
  jmp WinmmResolveBridge
.globl WinmmStub108
WinmmStub108:
  cmp qword ptr [rip+winmm_targets+864],0
  je .Lslow108
  jmp qword ptr [rip+winmm_targets+864]
.Lslow108:
  mov r10d,108
  jmp WinmmResolveBridge
.globl WinmmStub109
WinmmStub109:
  cmp qword ptr [rip+winmm_targets+872],0
  je .Lslow109
  jmp qword ptr [rip+winmm_targets+872]
.Lslow109:
  mov r10d,109
  jmp WinmmResolveBridge
.globl WinmmStub110
WinmmStub110:
  cmp qword ptr [rip+winmm_targets+880],0
  je .Lslow110
  jmp qword ptr [rip+winmm_targets+880]
.Lslow110:
  mov r10d,110
  jmp WinmmResolveBridge
.globl WinmmStub111
WinmmStub111:
  cmp qword ptr [rip+winmm_targets+888],0
  je .Lslow111
  jmp qword ptr [rip+winmm_targets+888]
.Lslow111:
  mov r10d,111
  jmp WinmmResolveBridge
.globl WinmmStub112
WinmmStub112:
  cmp qword ptr [rip+winmm_targets+896],0
  je .Lslow112
  jmp qword ptr [rip+winmm_targets+896]
.Lslow112:
  mov r10d,112
  jmp WinmmResolveBridge
.globl WinmmStub113
WinmmStub113:
  cmp qword ptr [rip+winmm_targets+904],0
  je .Lslow113
  jmp qword ptr [rip+winmm_targets+904]
.Lslow113:
  mov r10d,113
  jmp WinmmResolveBridge
.globl WinmmStub114
WinmmStub114:
  cmp qword ptr [rip+winmm_targets+912],0
  je .Lslow114
  jmp qword ptr [rip+winmm_targets+912]
.Lslow114:
  mov r10d,114
  jmp WinmmResolveBridge
.globl WinmmStub115
WinmmStub115:
  cmp qword ptr [rip+winmm_targets+920],0
  je .Lslow115
  jmp qword ptr [rip+winmm_targets+920]
.Lslow115:
  mov r10d,115
  jmp WinmmResolveBridge
.globl WinmmStub116
WinmmStub116:
  cmp qword ptr [rip+winmm_targets+928],0
  je .Lslow116
  jmp qword ptr [rip+winmm_targets+928]
.Lslow116:
  mov r10d,116
  jmp WinmmResolveBridge
.globl WinmmStub117
WinmmStub117:
  cmp qword ptr [rip+winmm_targets+936],0
  je .Lslow117
  jmp qword ptr [rip+winmm_targets+936]
.Lslow117:
  mov r10d,117
  jmp WinmmResolveBridge
.globl WinmmStub118
WinmmStub118:
  cmp qword ptr [rip+winmm_targets+944],0
  je .Lslow118
  jmp qword ptr [rip+winmm_targets+944]
.Lslow118:
  mov r10d,118
  jmp WinmmResolveBridge
.globl WinmmStub119
WinmmStub119:
  cmp qword ptr [rip+winmm_targets+952],0
  je .Lslow119
  jmp qword ptr [rip+winmm_targets+952]
.Lslow119:
  mov r10d,119
  jmp WinmmResolveBridge
.globl WinmmStub120
WinmmStub120:
  cmp qword ptr [rip+winmm_targets+960],0
  je .Lslow120
  jmp qword ptr [rip+winmm_targets+960]
.Lslow120:
  mov r10d,120
  jmp WinmmResolveBridge
.globl WinmmStub121
WinmmStub121:
  cmp qword ptr [rip+winmm_targets+968],0
  je .Lslow121
  jmp qword ptr [rip+winmm_targets+968]
.Lslow121:
  mov r10d,121
  jmp WinmmResolveBridge
.globl WinmmStub122
WinmmStub122:
  cmp qword ptr [rip+winmm_targets+976],0
  je .Lslow122
  jmp qword ptr [rip+winmm_targets+976]
.Lslow122:
  mov r10d,122
  jmp WinmmResolveBridge
.globl WinmmStub123
WinmmStub123:
  cmp qword ptr [rip+winmm_targets+984],0
  je .Lslow123
  jmp qword ptr [rip+winmm_targets+984]
.Lslow123:
  mov r10d,123
  jmp WinmmResolveBridge
.globl WinmmStub124
WinmmStub124:
  cmp qword ptr [rip+winmm_targets+992],0
  je .Lslow124
  jmp qword ptr [rip+winmm_targets+992]
.Lslow124:
  mov r10d,124
  jmp WinmmResolveBridge
.globl WinmmStub125
WinmmStub125:
  cmp qword ptr [rip+winmm_targets+1000],0
  je .Lslow125
  jmp qword ptr [rip+winmm_targets+1000]
.Lslow125:
  mov r10d,125
  jmp WinmmResolveBridge
.globl WinmmStub126
WinmmStub126:
  cmp qword ptr [rip+winmm_targets+1008],0
  je .Lslow126
  jmp qword ptr [rip+winmm_targets+1008]
.Lslow126:
  mov r10d,126
  jmp WinmmResolveBridge
.globl WinmmStub127
WinmmStub127:
  cmp qword ptr [rip+winmm_targets+1016],0
  je .Lslow127
  jmp qword ptr [rip+winmm_targets+1016]
.Lslow127:
  mov r10d,127
  jmp WinmmResolveBridge
.globl WinmmStub128
WinmmStub128:
  cmp qword ptr [rip+winmm_targets+1024],0
  je .Lslow128
  jmp qword ptr [rip+winmm_targets+1024]
.Lslow128:
  mov r10d,128
  jmp WinmmResolveBridge
.globl WinmmStub129
WinmmStub129:
  cmp qword ptr [rip+winmm_targets+1032],0
  je .Lslow129
  jmp qword ptr [rip+winmm_targets+1032]
.Lslow129:
  mov r10d,129
  jmp WinmmResolveBridge
.globl WinmmStub130
WinmmStub130:
  cmp qword ptr [rip+winmm_targets+1040],0
  je .Lslow130
  jmp qword ptr [rip+winmm_targets+1040]
.Lslow130:
  mov r10d,130
  jmp WinmmResolveBridge
.globl WinmmStub131
WinmmStub131:
  cmp qword ptr [rip+winmm_targets+1048],0
  je .Lslow131
  jmp qword ptr [rip+winmm_targets+1048]
.Lslow131:
  mov r10d,131
  jmp WinmmResolveBridge
.globl WinmmStub132
WinmmStub132:
  cmp qword ptr [rip+winmm_targets+1056],0
  je .Lslow132
  jmp qword ptr [rip+winmm_targets+1056]
.Lslow132:
  mov r10d,132
  jmp WinmmResolveBridge
.globl WinmmStub133
WinmmStub133:
  cmp qword ptr [rip+winmm_targets+1064],0
  je .Lslow133
  jmp qword ptr [rip+winmm_targets+1064]
.Lslow133:
  mov r10d,133
  jmp WinmmResolveBridge
.globl WinmmStub134
WinmmStub134:
  cmp qword ptr [rip+winmm_targets+1072],0
  je .Lslow134
  jmp qword ptr [rip+winmm_targets+1072]
.Lslow134:
  mov r10d,134
  jmp WinmmResolveBridge
.globl WinmmStub135
WinmmStub135:
  cmp qword ptr [rip+winmm_targets+1080],0
  je .Lslow135
  jmp qword ptr [rip+winmm_targets+1080]
.Lslow135:
  mov r10d,135
  jmp WinmmResolveBridge
.globl WinmmStub136
WinmmStub136:
  cmp qword ptr [rip+winmm_targets+1088],0
  je .Lslow136
  jmp qword ptr [rip+winmm_targets+1088]
.Lslow136:
  mov r10d,136
  jmp WinmmResolveBridge
.globl WinmmStub137
WinmmStub137:
  cmp qword ptr [rip+winmm_targets+1096],0
  je .Lslow137
  jmp qword ptr [rip+winmm_targets+1096]
.Lslow137:
  mov r10d,137
  jmp WinmmResolveBridge
.globl WinmmStub138
WinmmStub138:
  cmp qword ptr [rip+winmm_targets+1104],0
  je .Lslow138
  jmp qword ptr [rip+winmm_targets+1104]
.Lslow138:
  mov r10d,138
  jmp WinmmResolveBridge
.globl WinmmStub139
WinmmStub139:
  cmp qword ptr [rip+winmm_targets+1112],0
  je .Lslow139
  jmp qword ptr [rip+winmm_targets+1112]
.Lslow139:
  mov r10d,139
  jmp WinmmResolveBridge
.globl WinmmStub140
WinmmStub140:
  cmp qword ptr [rip+winmm_targets+1120],0
  je .Lslow140
  jmp qword ptr [rip+winmm_targets+1120]
.Lslow140:
  mov r10d,140
  jmp WinmmResolveBridge
.globl WinmmStub141
WinmmStub141:
  cmp qword ptr [rip+winmm_targets+1128],0
  je .Lslow141
  jmp qword ptr [rip+winmm_targets+1128]
.Lslow141:
  mov r10d,141
  jmp WinmmResolveBridge
.globl WinmmStub142
WinmmStub142:
  cmp qword ptr [rip+winmm_targets+1136],0
  je .Lslow142
  jmp qword ptr [rip+winmm_targets+1136]
.Lslow142:
  mov r10d,142
  jmp WinmmResolveBridge
.globl WinmmStub143
WinmmStub143:
  cmp qword ptr [rip+winmm_targets+1144],0
  je .Lslow143
  jmp qword ptr [rip+winmm_targets+1144]
.Lslow143:
  mov r10d,143
  jmp WinmmResolveBridge
.globl WinmmStub144
WinmmStub144:
  cmp qword ptr [rip+winmm_targets+1152],0
  je .Lslow144
  jmp qword ptr [rip+winmm_targets+1152]
.Lslow144:
  mov r10d,144
  jmp WinmmResolveBridge
.globl WinmmStub145
WinmmStub145:
  cmp qword ptr [rip+winmm_targets+1160],0
  je .Lslow145
  jmp qword ptr [rip+winmm_targets+1160]
.Lslow145:
  mov r10d,145
  jmp WinmmResolveBridge
.globl WinmmStub146
WinmmStub146:
  cmp qword ptr [rip+winmm_targets+1168],0
  je .Lslow146
  jmp qword ptr [rip+winmm_targets+1168]
.Lslow146:
  mov r10d,146
  jmp WinmmResolveBridge
.globl WinmmStub147
WinmmStub147:
  cmp qword ptr [rip+winmm_targets+1176],0
  je .Lslow147
  jmp qword ptr [rip+winmm_targets+1176]
.Lslow147:
  mov r10d,147
  jmp WinmmResolveBridge
.globl WinmmStub148
WinmmStub148:
  cmp qword ptr [rip+winmm_targets+1184],0
  je .Lslow148
  jmp qword ptr [rip+winmm_targets+1184]
.Lslow148:
  mov r10d,148
  jmp WinmmResolveBridge
.globl WinmmStub149
WinmmStub149:
  cmp qword ptr [rip+winmm_targets+1192],0
  je .Lslow149
  jmp qword ptr [rip+winmm_targets+1192]
.Lslow149:
  mov r10d,149
  jmp WinmmResolveBridge
.globl WinmmStub150
WinmmStub150:
  cmp qword ptr [rip+winmm_targets+1200],0
  je .Lslow150
  jmp qword ptr [rip+winmm_targets+1200]
.Lslow150:
  mov r10d,150
  jmp WinmmResolveBridge
.globl WinmmStub151
WinmmStub151:
  cmp qword ptr [rip+winmm_targets+1208],0
  je .Lslow151
  jmp qword ptr [rip+winmm_targets+1208]
.Lslow151:
  mov r10d,151
  jmp WinmmResolveBridge
.globl WinmmStub152
WinmmStub152:
  cmp qword ptr [rip+winmm_targets+1216],0
  je .Lslow152
  jmp qword ptr [rip+winmm_targets+1216]
.Lslow152:
  mov r10d,152
  jmp WinmmResolveBridge
.globl WinmmStub153
WinmmStub153:
  cmp qword ptr [rip+winmm_targets+1224],0
  je .Lslow153
  jmp qword ptr [rip+winmm_targets+1224]
.Lslow153:
  mov r10d,153
  jmp WinmmResolveBridge
.globl WinmmStub154
WinmmStub154:
  cmp qword ptr [rip+winmm_targets+1232],0
  je .Lslow154
  jmp qword ptr [rip+winmm_targets+1232]
.Lslow154:
  mov r10d,154
  jmp WinmmResolveBridge
.globl WinmmStub155
WinmmStub155:
  cmp qword ptr [rip+winmm_targets+1240],0
  je .Lslow155
  jmp qword ptr [rip+winmm_targets+1240]
.Lslow155:
  mov r10d,155
  jmp WinmmResolveBridge
.globl WinmmStub156
WinmmStub156:
  cmp qword ptr [rip+winmm_targets+1248],0
  je .Lslow156
  jmp qword ptr [rip+winmm_targets+1248]
.Lslow156:
  mov r10d,156
  jmp WinmmResolveBridge
.globl WinmmStub157
WinmmStub157:
  cmp qword ptr [rip+winmm_targets+1256],0
  je .Lslow157
  jmp qword ptr [rip+winmm_targets+1256]
.Lslow157:
  mov r10d,157
  jmp WinmmResolveBridge
.globl WinmmStub158
WinmmStub158:
  cmp qword ptr [rip+winmm_targets+1264],0
  je .Lslow158
  jmp qword ptr [rip+winmm_targets+1264]
.Lslow158:
  mov r10d,158
  jmp WinmmResolveBridge
.globl WinmmStub159
WinmmStub159:
  cmp qword ptr [rip+winmm_targets+1272],0
  je .Lslow159
  jmp qword ptr [rip+winmm_targets+1272]
.Lslow159:
  mov r10d,159
  jmp WinmmResolveBridge
.globl WinmmStub160
WinmmStub160:
  cmp qword ptr [rip+winmm_targets+1280],0
  je .Lslow160
  jmp qword ptr [rip+winmm_targets+1280]
.Lslow160:
  mov r10d,160
  jmp WinmmResolveBridge
.globl WinmmStub161
WinmmStub161:
  cmp qword ptr [rip+winmm_targets+1288],0
  je .Lslow161
  jmp qword ptr [rip+winmm_targets+1288]
.Lslow161:
  mov r10d,161
  jmp WinmmResolveBridge
.globl WinmmStub162
WinmmStub162:
  cmp qword ptr [rip+winmm_targets+1296],0
  je .Lslow162
  jmp qword ptr [rip+winmm_targets+1296]
.Lslow162:
  mov r10d,162
  jmp WinmmResolveBridge
.globl WinmmStub163
WinmmStub163:
  cmp qword ptr [rip+winmm_targets+1304],0
  je .Lslow163
  jmp qword ptr [rip+winmm_targets+1304]
.Lslow163:
  mov r10d,163
  jmp WinmmResolveBridge
.globl WinmmStub164
WinmmStub164:
  cmp qword ptr [rip+winmm_targets+1312],0
  je .Lslow164
  jmp qword ptr [rip+winmm_targets+1312]
.Lslow164:
  mov r10d,164
  jmp WinmmResolveBridge
.globl WinmmStub165
WinmmStub165:
  cmp qword ptr [rip+winmm_targets+1320],0
  je .Lslow165
  jmp qword ptr [rip+winmm_targets+1320]
.Lslow165:
  mov r10d,165
  jmp WinmmResolveBridge
.globl WinmmStub166
WinmmStub166:
  cmp qword ptr [rip+winmm_targets+1328],0
  je .Lslow166
  jmp qword ptr [rip+winmm_targets+1328]
.Lslow166:
  mov r10d,166
  jmp WinmmResolveBridge
.globl WinmmStub167
WinmmStub167:
  cmp qword ptr [rip+winmm_targets+1336],0
  je .Lslow167
  jmp qword ptr [rip+winmm_targets+1336]
.Lslow167:
  mov r10d,167
  jmp WinmmResolveBridge
.globl WinmmStub168
WinmmStub168:
  cmp qword ptr [rip+winmm_targets+1344],0
  je .Lslow168
  jmp qword ptr [rip+winmm_targets+1344]
.Lslow168:
  mov r10d,168
  jmp WinmmResolveBridge
.globl WinmmStub169
WinmmStub169:
  cmp qword ptr [rip+winmm_targets+1352],0
  je .Lslow169
  jmp qword ptr [rip+winmm_targets+1352]
.Lslow169:
  mov r10d,169
  jmp WinmmResolveBridge
.globl WinmmStub170
WinmmStub170:
  cmp qword ptr [rip+winmm_targets+1360],0
  je .Lslow170
  jmp qword ptr [rip+winmm_targets+1360]
.Lslow170:
  mov r10d,170
  jmp WinmmResolveBridge
.globl WinmmStub171
WinmmStub171:
  cmp qword ptr [rip+winmm_targets+1368],0
  je .Lslow171
  jmp qword ptr [rip+winmm_targets+1368]
.Lslow171:
  mov r10d,171
  jmp WinmmResolveBridge
.globl WinmmStub172
WinmmStub172:
  cmp qword ptr [rip+winmm_targets+1376],0
  je .Lslow172
  jmp qword ptr [rip+winmm_targets+1376]
.Lslow172:
  mov r10d,172
  jmp WinmmResolveBridge
.globl WinmmStub173
WinmmStub173:
  cmp qword ptr [rip+winmm_targets+1384],0
  je .Lslow173
  jmp qword ptr [rip+winmm_targets+1384]
.Lslow173:
  mov r10d,173
  jmp WinmmResolveBridge
.globl WinmmStub174
WinmmStub174:
  cmp qword ptr [rip+winmm_targets+1392],0
  je .Lslow174
  jmp qword ptr [rip+winmm_targets+1392]
.Lslow174:
  mov r10d,174
  jmp WinmmResolveBridge
.globl WinmmStub175
WinmmStub175:
  cmp qword ptr [rip+winmm_targets+1400],0
  je .Lslow175
  jmp qword ptr [rip+winmm_targets+1400]
.Lslow175:
  mov r10d,175
  jmp WinmmResolveBridge
.globl WinmmStub176
WinmmStub176:
  cmp qword ptr [rip+winmm_targets+1408],0
  je .Lslow176
  jmp qword ptr [rip+winmm_targets+1408]
.Lslow176:
  mov r10d,176
  jmp WinmmResolveBridge
.globl WinmmStub177
WinmmStub177:
  cmp qword ptr [rip+winmm_targets+1416],0
  je .Lslow177
  jmp qword ptr [rip+winmm_targets+1416]
.Lslow177:
  mov r10d,177
  jmp WinmmResolveBridge
.globl WinmmStub178
WinmmStub178:
  cmp qword ptr [rip+winmm_targets+1424],0
  je .Lslow178
  jmp qword ptr [rip+winmm_targets+1424]
.Lslow178:
  mov r10d,178
  jmp WinmmResolveBridge
.globl WinmmStub179
WinmmStub179:
  cmp qword ptr [rip+winmm_targets+1432],0
  je .Lslow179
  jmp qword ptr [rip+winmm_targets+1432]
.Lslow179:
  mov r10d,179
  jmp WinmmResolveBridge
.globl WinmmStub180
WinmmStub180:
  cmp qword ptr [rip+winmm_targets+1440],0
  je .Lslow180
  jmp qword ptr [rip+winmm_targets+1440]
.Lslow180:
  mov r10d,180
  jmp WinmmResolveBridge
.globl WinmmResolveBridge
.seh_proc WinmmResolveBridge
WinmmResolveBridge:
  sub rsp,200
  .seh_stackalloc 200
  .seh_endprologue
  mov [rsp+32],rcx
  mov [rsp+40],rdx
  mov [rsp+48],r8
  mov [rsp+56],r9
  movdqu [rsp+64],xmm0
  movdqu [rsp+80],xmm1
  movdqu [rsp+96],xmm2
  movdqu [rsp+112],xmm3
  movdqu [rsp+128],xmm4
  movdqu [rsp+144],xmm5
  mov ecx,r10d
  call ResolveWinmm
  mov r11,rax
  mov rcx,[rsp+32]
  mov rdx,[rsp+40]
  mov r8,[rsp+48]
  mov r9,[rsp+56]
  movdqu xmm0,[rsp+64]
  movdqu xmm1,[rsp+80]
  movdqu xmm2,[rsp+96]
  movdqu xmm3,[rsp+112]
  movdqu xmm4,[rsp+128]
  movdqu xmm5,[rsp+144]
  add rsp,200
  jmp r11
.seh_endproc
