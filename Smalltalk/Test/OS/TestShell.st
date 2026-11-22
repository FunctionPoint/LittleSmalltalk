" unit tests for Shell "
+Test subclass: #TestShell variables: #( ) classVariables: #( )
!TestShell
test
	self assert: Shell superclass == Application.
	self assert: ( Shell new respondsTo: #start ).
	self assert: ( Shell new respondsTo: #args ).
!
