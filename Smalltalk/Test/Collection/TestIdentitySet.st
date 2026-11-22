" unit tests for IdentitySet "
+Test subclass: #TestIdentitySet variables: #( ) classVariables: #( )
!TestIdentitySet
test
	| set string |
	set <- IdentitySet new.
	string <- 'a'.
	set add: string.
	set add: string.
	self assert: set size = 1.
	set add: string copy.
	self assert: set size = 2.
	self assert: ( set includes: string ).
!
