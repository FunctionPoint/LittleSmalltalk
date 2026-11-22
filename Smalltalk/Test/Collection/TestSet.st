" unit tests for Set "
+Test subclass: #TestSet variables: #( ) classVariables: #( )
!TestSet
testAdding
	| set |
	set <- Set new.
	self assert: set isEmpty.
	set add: 3.
	set add: 4.
	set add: 3.
	self assert: set size = 2.
	self assert: ( set includes: 3 ).
	self assert: ( set includes: 5 ) not.
	set add: 'a'.
	set add: 'a' copy.
	self assert: set size = 3.
!
!TestSet
testRemoving
	| set |
	set <- Set new.
	set add: 3.
	set add: 4.
	set remove: 3.
	self assert: set size = 1.
	self assert: ( set includes: 3 ) not.
!
!TestSet
testGrowing
	| set |
	set <- Set new.
	1 to: 100 do: [ :number | set add: number ].
	self assert: set size = 100.
	self assert: ( set includes: 77 ).
!
