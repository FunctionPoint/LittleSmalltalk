" class definition for Association "
+Magnitude subclass: #Association variables: #( key value ) classVariables: #( )
" class methods for Association "
=Association
key: aKey
	"key is set once, value is resettable"
	^ self in: self new at: 1 put: aKey.
!
=Association
key: aKey value: aValue
	| association |
	"key is set once, value is resettable"
	association <- self new.
	self in: association at: 1 put: aKey.
	self in: association at: 2 put: aValue.
	^ association.
!
" instance methods for Association "
!Association
< other
	"compare both with keys and associations"
	( other class == Association )
		ifTrue: [ ^ key < other key ]
		ifFalse: [ ^ key < other ].
!
!Association
= other
	"compare both with keys and associations"
	( other class == Association )
		ifTrue: [ ^ key = other key ]
		ifFalse: [ ^ key = other ].
!
!Association
hash
	^ key hash.
!
!Association
key
	^ key.
!
!Association
printString
	^ '(' + key printString + ' -> ' + value printString + ')'.
!
!Association
value
	^ value.
!
!Association
value: aValue
	value <- aValue.
!
