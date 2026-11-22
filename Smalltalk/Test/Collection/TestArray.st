" unit tests for Array "
+Test subclass: #TestArray variables: #( ) classVariables: #( )
!TestArray
testCreation
	self assert: ( Array new: 3 ) size = 3.
	self assert: ( Array new: 2 ) first isNil.
	self assert: ( Array with: 7 ) first = 7.
	self assert: ( Array with: 1 with: 2 with: 3 ) = #( 1 2 3 ).
	self assert: #( 1 $a 'b' #c ) size = 4.
!
!TestArray
testAccess
	| array |
	array <- Array new: 2.
	array at: 1 put: 9.
	self assert: ( array at: 1 ) = 9.
	self assert: ( array at: 2 ) isNil.
	self assert: #( 5 6 ) first = 5.
	self assert: ( #( 5 6 ) at: 3 ifAbsent: [ 0 ] ) = 0.
!
!TestArray
testComparing
	self assert: #( 1 2 ) = #( 1 2 ).
	self assert: ( #( 1 2 ) = #( 1 3 ) ) not.
	self assert: #( 1 2 ) < #( 1 3 ).
	self assert: #( 1 2 3 ) hash = #( 1 2 3 ) hash.
!
!TestArray
testCopying
	| array copy |
	self assert: #( 1 2 3 ) + #( 4 ) = #( 1 2 3 4 ).
	self assert: ( #( 1 2 3 ) with: 4 ) = #( 1 2 3 4 ).
	self assert: ( #( 1 2 3 ) from: 2 to: 3 ) = #( 2 3 ).
	self assert: ( #( 1 2 3 ) insert: 9 at: 2 ) = #( 1 9 2 3 ).
	self assert: ( #( 1 2 3 ) removeIndex: 2 ) = #( 1 3 ).
	array <- #( 1 2 ) copy.
	copy <- array copy.
	copy at: 1 put: 5.
	self assert: ( array at: 1 ) = 1.
	array <- Array new: 3.
	array replaceFrom: 1 to: 2 with: #( 7 8 ).
	self assert: ( array at: 2 ) = 8.
	self assert: ( array at: 3 ) isNil.
!
!TestArray
testSearching
	self assert: ( #( 1 2 3 ) includes: 3 ).
	self assert: ( #( 1 2 3 ) includes: 4 ) not.
	self assert: ( #( 1 2 3 ) indexOf: 2 ) = 2.
	self assert: ( #( 1 2 3 ) indexOf: 4 ) isNil.
!
