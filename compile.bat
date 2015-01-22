rem Author: stevpan@gmail.com 
rem JDK 6 and Visual Studio 2008 (or higher) are required.


rem Before running this batch file, 
rem you need to setup vs2008 enviroment at first
rem c:\vs2008\VC\bin\vcvars32.bat


rem clean old build
set jp=c:\Program Files (x86)\Java\jdk1.7.0_71\
set vsp=C:\Program Files (x86)\Microsoft Visual Studio 10.0\VC\

del xfiledialog.dll
del *.class 

"%jp%bin\javac" src_java\net\tomahawk\*.java
"%jp%bin\javac" -cp .\src_java helloworld.java
"%jp%bin\javac" -cp .\src_java helloapplet.java

cd src_java
"%jp%bin\jar" cvf ..\xfiledialog.jar net
cd ..


rem clean vim editor's temp files 
rem
del src_cpp\*~ 
del src_java\net\tomahawk\*~

rem please change the include directory according to your 
rem JDK home and Visual studio home in the following command

cl  /D UNICODE /D _UNICODE /D _CRT_NON_CONFORMING_SWPRINTFS /D OS_ARCH_X86 -I "%jp%include" -I "%jp%include\win32" -I "%vsp%atlmfc\include" /W3 /O1 /LD /EHsc -Fexfiledialog.dll src_cpp\*.cpp 

rem clean temp files in compilation 
del *.exp
del *.lib
del *.obj

