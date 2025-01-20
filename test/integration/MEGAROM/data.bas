FILE "data0.bin"                                ' 0
FILE "data1.bin"                                ' 1
FILE "data2.sc5"                                ' 2
FILE "data3.sc5"                                ' 3
FILE "data4.bin"                                ' 4

1 DEFINT A-Z
211 CMD RESTORE 4                                                       ' DATA on resource 4
212 IRESTORE 7                                                          ' set DATA initial byte position
213 FOR Y=0 TO 8                                                        ' read level data
214   IREAD BB : W = ((BB SHR 8) AND &hFF) OR ((BB SHL 8) AND &hFF00)    ' read integer and reverse to big endian
215   PRINT HEX$(BB), HEX$(W)
219 NEXT 
220 END

' must print:
' 8282 8282
' 8283 8382
' 7F82 827F
' 7F81 817F
' 7F81 817F
' 7E81 817E
' 8180 8081
' 8080 8080
' 7D7F 7F7D


