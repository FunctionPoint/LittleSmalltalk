" unit tests for List "
+Test subclass: #TestList variables: #( ) classVariables: #( )
!TestList
testAdding
	| list |
	list <- List new.
	self assert: list isEmpty.
	list add: 1.
	list add: 2.
	self assert: list asArray = #( 2 1 ).
	list addLast: 3.
	self assert: list asArray = #( 2 1 3 ).
	self assert: list size = 3.
	list addAll: #( 4 5 ).
	self assert: list size = 5.
	self assert: ( List with: 7 ) first = 7.
!
!TestList
testAccess
	| list |
	list <- List new.
	list addLast: 1.
	list addLast: 2.
	self assert: list first = 1.
	self assert: ( list at: 2 ) = 2.
	self assert: list reverse asArray = #( 2 1 ).
	list at: 1 put: 9.
	self assert: list first = 9.
!
!TestList
testRemoving
	| list |
	list <- List new.
	list addLast: 1.
	list addLast: 2.
	list addLast: 3.
	list removeFirst.
	self assert: list asArray = #( 2 3 ).
	list remove: 3.
	self assert: list asArray = #( 2 ).
	list remove: 2.
	self assert: list isEmpty.
	list addLast: 1.
	list addLast: 2.
	self assert: ( list remove: 9 ifAbsent: [ #none ] ) = #none.
	self assert: list asArray = #( 1 2 ).
!
!TestList
testSearching
	| list |
	list <- List new.
	list addLast: #a.
	list addLast: #b.
	self assert: ( list includes: #b ).
	self assert: ( list includes: #c ) not.
!
!TestList
testCopying
	| list copy |
	list <- List new.
	list addLast: 1.
	copy <- list copy.
	copy addLast: 2.
	self assert: list size = 1.
	self assert: copy size = 2.
!
