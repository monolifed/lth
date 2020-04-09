@IF %1==CLEAN (
@del /Q obj\*.o
@del /Q build\LastTrain.exe
)

@IF %1==POST (
@echo POST BUILD
@copy libs\soft_oal.dll build\OpenAL32.dll
)
