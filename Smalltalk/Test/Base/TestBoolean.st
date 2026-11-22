" unit tests for Boolean "
+Test subclass: #TestBoolean variables: #( ) classVariables: #( )
!TestBoolean
testLogic
	self assert: ( true and: [ true ] ).
	self assert: ( true and: [ false ] ) not.
	self assert: ( false and: [ true ] ) not.
	self assert: ( false or: [ true ] ).
	self assert: ( false or: [ false ] ) not.
	self assert: ( true or: [ false ] ).
	self assert: true not = false.
	self assert: false not = true.
!
!TestBoolean
testConditionals
	self assert: ( true ifTrue: [ 1 ] ifFalse: [ 2 ] ) = 1.
	self assert: ( false ifTrue: [ 1 ] ifFalse: [ 2 ] ) = 2.
	self assert: ( false ifTrue: [ 1 ] ) isNil.
	self assert: ( true ifFalse: [ 1 ] ) isNil.
	self assert: ( 3 > 2 ifTrue: [ #yes ] ) = #yes.
!
!TestBoolean
testClasses
	self assert: ( true isKindOf: Boolean ).
	self assert: ( false isKindOf: Boolean ).
	self assert: True superclass == Boolean.
	self assert: False superclass == Boolean.
!
