!include "../global.mak"

ALL : "$(OUTDIR)\MQ2RaidUtils.dll"

CLEAN :
	-@erase "$(INTDIR)\MQ2RaidUtils.obj"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(OUTDIR)\MQ2RaidUtils.dll"
	-@erase "$(OUTDIR)\MQ2RaidUtils.exp"
	-@erase "$(OUTDIR)\MQ2RaidUtils.lib"
	-@erase "$(OUTDIR)\MQ2RaidUtils.pdb"


LINK32=link.exe
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib $(DETLIB) ..\Release\MQ2Main.lib /nologo /dll /incremental:no /pdb:"$(OUTDIR)\MQ2RaidUtils.pdb" /debug /machine:I386 /out:"$(OUTDIR)\MQ2RaidUtils.dll" /implib:"$(OUTDIR)\MQ2RaidUtils.lib" /OPT:NOICF /OPT:NOREF 
LINK32_OBJS= \
	"$(INTDIR)\MQ2RaidUtils.obj" \
	"$(OUTDIR)\MQ2Main.lib"

"$(OUTDIR)\MQ2RaidUtils.dll" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) $(LINK32_FLAGS) $(LINK32_OBJS)


!IF "$(NO_EXTERNAL_DEPS)" != "1"
!IF EXISTS("MQ2RaidUtils.dep")
!INCLUDE "MQ2RaidUtils.dep"
!ELSE 
!MESSAGE Warning: cannot find "MQ2RaidUtils.dep"
!ENDIF 
!ENDIF 


SOURCE=.\MQ2RaidUtils.cpp

"$(INTDIR)\MQ2RaidUtils.obj" : $(SOURCE) "$(INTDIR)"

