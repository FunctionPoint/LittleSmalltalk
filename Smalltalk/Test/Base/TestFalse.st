" unit tests for False "
+Test subclass: #TestFalse variables: #( ) classVariables: #( )
!TestFalse
test
	self assert: false not.
	self assert: false printString = 'false'.
	self assert: False new == false.
	self assert: false class == False.
	self assert: ( false ifTrue: [ 1 ] ifFalse: [ 2 ] ) = 2.
!
