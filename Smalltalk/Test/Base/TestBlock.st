" unit tests for Block "
+Test subclass: #TestBlock variables: #( ) classVariables: #( )
!TestBlock
testValue
	| counter |
	self assert: [ 3 ] value = 3.
	self assert: ( [ :number | number * 2 ] value: 21 ) = 42.
	self assert: ( [ :number :secondNumber | number + secondNumber ] value: 1 value: 2 ) = 3.
	self assert: [ ] value isNil.
	counter <- 1.
	[ counter <- counter + 1 ] value.
	self assert: counter = 2.
!
!TestBlock
testLoops
	| counter |
	counter <- 0.
	[ counter < 5 ] whileTrue: [ counter <- counter + 1 ].
	self assert: counter = 5.
	[ counter >= 8 ] whileFalse: [ counter <- counter + 1 ].
	self assert: counter = 8.
!
!TestBlock
testReturn
	self assert: ( self firstOver: 2 in: #( 1 3 5 ) ) = 3.
	self assert: ( self firstOver: 9 in: #( 1 3 5 ) ) isNil.
!
!TestBlock
testShadowing
	| value sum |
	" a block argument hides a method variable of the same name "
	value <- 100.
	sum <- 0.
	#( 1 2 3 ) do: [ :value | sum <- sum + value ].
	self assert: sum = 6.
	self assert: value = 100.
	#( 4 ) do: [ :value | value <- value + 1. sum <- value ].
	self assert: sum = 5.
	self assert: value = 100.
!
!TestBlock
firstOver: limit in: list
	" a ^ in a block returns from the method "
	list do: [ :element | element > limit ifTrue: [ ^ element ] ].
	^ nil.
!
