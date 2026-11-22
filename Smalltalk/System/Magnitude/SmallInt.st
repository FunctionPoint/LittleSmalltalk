" class definition for SmallInt "
+Number subclass: #SmallInt variables: #( ) classVariables: #( seed )
" class methods for SmallInt "
=SmallInt
atRandom
	" Set up seed one time.  TBD: init from something external;
	getpid() or time() "
	seed isNil ifTrue: [ seed <- 17 ].

	" Rotate the random number generator. "
	seed <- ( ( seed * 1103515245 + 12345 ) truncSmallInt )
		bitAnd: 268435455.
	^ seed.
!
" instance methods for SmallInt "
!SmallInt
* aNumber
	<15 self aNumber>
	( aNumber isMemberOf: SmallInt ) ifFalse: [ ^self * aNumber asSmallInt ].
	self primitiveFailed.
!
!SmallInt
+ aNumber
	<10 self aNumber>
	( aNumber isMemberOf: SmallInt ) ifFalse: [ ^self + aNumber asSmallInt ].
	self primitiveFailed.
!
!SmallInt
- aNumber
	<16 self aNumber>
	( aNumber isMemberOf: SmallInt ) ifFalse: [ ^self - aNumber asSmallInt ].
	self primitiveFailed.
!
!SmallInt
< aNumber
	<13 self aNumber>
	( aNumber isMemberOf: SmallInt ) ifFalse: [ ^self < aNumber asSmallInt ].
	self primitiveFailed.
!
!SmallInt
= aNumber
	<14 self aNumber>
	( aNumber isMemberOf: SmallInt ) ifFalse: [ ^self = aNumber asSmallInt ].
	self primitiveFailed.
!
!SmallInt
asInteger
	^Integer new: self.
!
!SmallInt
asSmallInt
	^self.
!
!SmallInt
bitAnd: aNumber
	<37 self aNumber>.
	^ ( self bitAnd: aNumber asSmallInt ).
!
!SmallInt
bitOr: aNumber
	<36 self aNumber>.
	^ ( self bitOr: aNumber asSmallInt ).
!
!SmallInt
bitShift: aNumber
	<39 self aNumber>.
	( aNumber isKindOf: SmallInt ) ifTrue: [ self overflow ].
	^ ( self bitShift: aNumber asSmallInt ).
!
!SmallInt
hash
	^ self.
!
!SmallInt
quo: aNumber
	<11 self aNumber>
	( aNumber isMemberOf: SmallInt ) ifFalse: [ ^self quo: aNumber asSmallInt ].
	( 0 = aNumber ) ifTrue: [ ^ self error: 'division by zero' ].
	self primitiveFailed.
!
!SmallInt
rem: aNumber
	<12 self aNumber>
	( aNumber isMemberOf: SmallInt ) ifFalse: [ ^self rem: aNumber asSmallInt ].
	( 0 = aNumber ) ifTrue: [ ^ self error: 'division by zero' ].
	self primitiveFailed.
!
!SmallInt
truncSmallInt
	^self.
!
