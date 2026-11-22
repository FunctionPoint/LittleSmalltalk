" class definition for Object "
+nil subclass: #Object variables: #( ) classVariables: #( )
" class methods for Object "
=Object
in: object at: index put: value
	" change data field in object, used during initialization "
	" returns the intialized object "
	<5 value object index>.
!
" instance methods for Object "
!Object
= anObject
	^ self == anObject.
!
!Object
== anObject
	<1 self anObject>.
!
!Object
basicDo: aBlock
	^ self do: aBlock.
!
!Object
basicSize
	<4 self>.
	self primitiveFailed.
!
!Object
become: other
	" Exchange identity with another object "
	( Array with: self ) elementsExchangeIdentityWith: ( Array with: other ).
!
!Object
class
	<2 self>.
!
!Object
doesNotUnderstand: aSelector
	self error: ( self printString + ' (class '+
		( self class printString ) +
		'): does not understand ' + aSelector printString ).
!
!Object
error: aString
	" print the message "
	aString printNl.
	" then halt "
	<19>.
!
!Object
hash
	" Most objects should generate something based on their value "
	^ self class printString hash.
!
!Object
in: object at: index
	" browse instance variable via debugger "
	<24 object index>.
	self primitiveFailed.
!
!Object
isKindOf: aClass
	| currentClass |
	currentClass <- self class.
	[ currentClass notNil ] whileTrue:
		[ currentClass == aClass ifTrue: [ ^ true ].
			currentClass <- currentClass superclass ].
	^ false.
!
!Object
isMemberOf: aClass
	^ self class == aClass.
!
!Object
isNil
	^ false.
!
!Object
notNil
	^ true.
!
!Object
primitiveFailed
	self error: 'Primitive failed'.
!
!Object
print
	self printString do: [ :character | character print ].
!
!Object
printNl
	self print. Char newline print.
!
!Object
printString
	^ self class printString.
!
!Object
question: text
	| answer |
	text print.
	answer <- String input.
	( answer notNil )
		ifTrue: [ answer <- answer at: 1 ifAbsent: [ $n ] ].
	^ answer = $y or: [ answer = $Y ].
!
!Object
respondsTo: aMessage
	^ self class allMethods includes: aMessage.
!
!Object
species
	" By default, we re-instantiate ourselves as our own Class "
	^ self class.
!
!Object
subclassResponsibility
	self error: 'Subclass responsibility'.
!
!Object
~= anObject
	^ ( self = anObject ) not.
!
!Object
~~ anObject
	^ ( self == anObject ) not.
!
