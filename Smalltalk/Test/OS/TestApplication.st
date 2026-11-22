" unit tests for Application "
+Test subclass: #TestApplication variables: #( ) classVariables: #( )
!TestApplication
test
	self assert: ( Application new args isKindOf: Array ).
	self assert: Application new args size >= 1.
	self assert: ( ( Application new args at: 1 ) isKindOf: String ).
	self assert: Application args size = Application new args size.
!
