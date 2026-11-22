" unit tests for Object "
+Test subclass: #TestObject variables: #( ) classVariables: #( )
!TestObject
testIdentity
	| object |
	object <- Object new.
	self assert: object == object.
	self assert: object = object.
	self assert: object ~~ Object new.
	self assert: object ~= Object new.
	self assert: #a == #a.
	self assert: ( 'a' == 'a' ) not.
!
!TestObject
testNil
	self assert: nil isNil.
	self assert: Object new notNil.
	self assert: Object new isNil not.
!
!TestObject
testClass
	self assert: Object new class == Object.
	self assert: ( 3 isKindOf: Number ).
	self assert: ( 3 isKindOf: Object ).
	self assert: ( 3 isMemberOf: SmallInt ).
	self assert: ( 3 isMemberOf: Number ) not.
	self assert: ( 3 respondsTo: #+ ).
	self assert: ( 3 respondsTo: #foo ) not.
	self assert: Object new species == Object.
!
!TestObject
testPrinting
	self assert: Object new printString = 'Object'.
	self assert: Object new hash = Object new hash.
!
