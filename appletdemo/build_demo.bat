rem this batch file can be executed after you run compile.bat, compile64.bat
rem rename the "personalkey" to your keyname if necessary 
rem
rem If you prefer ant, do it yourself. 

   cd ..
   jar cvf appletdemo/hello.jar helloapplet.class
   jar uvf appletdemo/hello.jar -C src_java net

   jar cvf appletdemo/win_x86_dll.jar  xfiledialog.dll xfiledialog.dll.manifest
   jar cvf appletdemo/win_x64_dll.jar  xfiledialog64.dll xfiledialog64.dll.manifest

   cd appletdemo 
   jarsigner hello.jar personalkey
   jarsigner win_x86_dll.jar personalkey 
   jarsigner win_x64_dll.jar personalkey
   


