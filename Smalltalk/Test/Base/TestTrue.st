" unit tests for True "
+Test subclass: #TestTrue variables: #( ) classVariables: #( )
!TestTrue
test
	self assert: true.
	self assert: true not not.
	self assert: true printString = 'true'.
	self assert: True new == true.
	self assert: true class == True.
	self assert: ( true ifTrue: [ 1 ] ifFalse: [ 2 ] ) = 1.
!
