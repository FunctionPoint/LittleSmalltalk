" the startup code of the unit tests: lst runs Main>>start, then exits "
" lst -o bin/test.img Smalltalk/System Smalltalk/Test, then lst bin/test.img (from any folder) "
+Application subclass: #Main variables: #( ) classVariables: #( )
!Main
start
	Test all.
!
