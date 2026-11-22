" class definition for Integer "
+Number subclass: #Integer variables: #( ) classVariables: #( )
" class methods for Integer "
=Integer
new: low
	<32 low>
	low <- low asSmallInt.
	<32 low>
	self primitiveFailed.
!
" instance methods for Integer "
!Integer
* aNumber
	<28 self aNumber>
	( aNumber isMemberOf: Integer ) ifFalse: [ ^self * aNumber asInteger ].
	self primitiveFailed.
!
!Integer
+ aNumber
	<27 self aNumber>
	( aNumber isMemberOf: Integer ) ifFalse: [ ^self + aNumber asInteger ].
	self primitiveFailed.
!
!Integer
- aNumber
	<29 self aNumber>
	( aNumber isMemberOf: Integer ) ifFalse: [ ^self - aNumber asInteger ].
	self primitiveFailed.
!
!Integer
< aNumber
	<30 self aNumber>
	( aNumber isMemberOf: Integer ) ifFalse: [ ^self < aNumber asInteger ].
	self primitiveFailed.
!
!Integer
= aNumber
	<31 self aNumber>
	( aNumber isMemberOf: Integer ) ifFalse: [ ^self = aNumber asInteger ].
	self primitiveFailed.
!
!Integer
asInteger
	^self.
!
!Integer
asSmallInt
	<33 self>.
	self overflow.
!
!Integer
hash
	<33 self>.
	^ ( self rem: 65536 ) asSmallInt.
!
!Integer
quo: aNumber
	<25 self aNumber>
	( aNumber isMemberOf: Integer ) ifFalse: [ ^self quo: aNumber asInteger ].
	( 0 = aNumber ) ifTrue: [ ^ self error: 'division by zero' ].
	self primitiveFailed.
!
!Integer
rem: aNumber
	<26 self aNumber>
	( aNumber isMemberOf: Integer ) ifFalse: [ ^self rem: aNumber asInteger ].
	( 0 = aNumber ) ifTrue: [ ^ self error: 'division by zero' ].
	self primitiveFailed.
!
!Integer
truncSmallInt
	<40 self>.
	self primitiveFailed.
!
