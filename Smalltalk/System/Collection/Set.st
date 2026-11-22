" class definition for Set "
+Collection subclass: #Set variables: #( members growth ) classVariables: #( )
" class methods for Set "
=Set
new
	^ self new: 10.
!
=Set
new: size
	| newSet |
	newSet <- super new.
	self in: newSet at: 1 put: ( Array new: size ).
	self in: newSet at: 2 put: size.
	^ newSet.
!
" instance methods for Set "
!Set
add: element
	| position |
	" Find the appropriate slot... if none, need to grow the Set "
	position <- self location: element.
	position isNil ifTrue: [
		self grow.
		^ self add: element ].

	" If the slot is nil, this is a new entry which we put in place now.
	If it wasn't nil, we still re-store it so that if it's an
	Association, the value portion will be updated. "
	members at: position put: element.
	^ element.
!
!Set
at: value ifAbsent: aBlock
	| position |
	position <- self location: value.
	( ( position isNil ) or: [ ( members at: position ) isNil ] ) ifTrue: [
		^ aBlock value ].
	^ value.
!
!Set
compare: element and: otherElement
	^ element = otherElement.
!
!Set
do: aBlock
	members do: [ :element | element notNil ifTrue: [ aBlock value: element ] ].
!
!Set
grow
	| bigger oldMembers oldSize |
	" Re-create ourselves in place with a new, bigger storage "
	oldMembers <- members.
	members <- Array new: ( oldMembers size + growth ).

	" Re-insert each existing Set member "
	oldMembers do: [ :element | self add: element ].
!
!Set
indexOf: value
	^ self at: value ifAbsent: [ nil ].
!
!Set
location: element
	| position start member |
	start <- position <- ( element hash rem: members size ) + 1.
	[ true ] whileTrue: [
		" Return this position if we match, or have reached
			a nil slot. "
		member <- members at: position.
		( ( member isNil ) or: [ self compare: member and: element ] ) ifTrue: [
			^ position ].

		" Advance to next slot, circularly "
		position <- position + 1.
		( position > members size ) ifTrue: [
			position <- 1 ].

		" Return nil if we have scanned the whole Set "
		( position = start ) ifTrue: [ ^ nil ] ].
!
!Set
rehash: start
	| position element |
	position <- start.
	[ true ] whileTrue: [
		" Advance to next slot, ceasing when we reach our start "
		position <- position + 1.
		( position > members size ) ifTrue: [ position <- 1 ].
		( position = start ) ifTrue: [ ^ self ].

		" If we reach a nil slot, there are no further rehash
			worries. "
		element <- members at: position.
		element isNil ifTrue: [ ^ self ].

		" Nil out the slot, and then re-insert the element "
		members at: position put: nil.
		self add: element ].
!
!Set
remove: element
	^ self remove: element ifAbsent: [ self noElement ].
!
!Set
remove: element ifAbsent: aBlock
	| position |
	" If not found, return error "
	position <- self location: element.
	( ( position isNil ) or: [ ( members at: position ) isNil ] ) ifTrue: [
		aBlock value ].

	" Remove our element from the Set "
	members at: position put: nil.

	" Re-hash all that follow "
	self rehash: position.

	^ element.
!
!Set
size
	| tally |
	tally <- 0.
	members do: [ :element | element notNil ifTrue: [ tally <- tally + 1 ] ].
	^ tally.
!
