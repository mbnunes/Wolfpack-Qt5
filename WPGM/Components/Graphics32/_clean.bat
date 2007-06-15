del /S *.~*
del /S *.dcu
del /S *.elf
del /S *.dpu
del /S *.dsk
del /S *.cfg
del /S *.dof
del /S *.obj
del /S *.hpp
del /S *.ddp
del /S *.mps
del /S *.mpt
del /S *.map
del /S *.log
del /s *.exe
del /s *.so
del /s *.stat
del /s *.tci
del /s /A Thumbs.db
cd Examples\Clx
REM dfm files aren't needed for Kylix and CLX...
del /s *.dfm
pause
