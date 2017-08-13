set proto_path=.\
set export_path=.\

dir /b %proto_path%*.proto > proto_list
for /f %%i in (proto_list) do ( 
	protoc.exe -I=%proto_path% --cpp_out=%export_path% %proto_path%%%i
)
del proto_list

set export_path=..\..\CodfiyAsteriatedGrailClient\protocol\

dir /b %proto_path%*.proto > proto_list
for /f %%i in (proto_list) do ( 
	protoc.exe -I=%proto_path% --cpp_out=%export_path% %proto_path%%%i
)
del proto_list

pause