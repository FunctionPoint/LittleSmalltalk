" unit tests for Class "
+Test subclass: #TestClass variables: #( ) classVariables: #( )
!TestClass
testNames
	self assert: Object name = #Object.
	self assert: Object printString = 'Object'.
	self assert: Object superclass isNil.
	self assert: Array superclass == Collection.
	self assert: Object class class == Class.
	self assert: Class instanceVariables = #( name parentClass methods size variables ).
	self assert: ( Array subclasses asArray includes: String ).
!
!TestClass
testCompiling
	| result |
	result <- Object compile: 'foo ^ bar' variables: #( ).
	self assert: ( result isKindOf: String ).
	result <- Object compile: 'foo ^ 1' variables: #( ).
	self assert: ( result at: 1 ) = 'foo'.
	self assert: ( Object parseMethod: 'foo ^ 1' ) class == Method.
	self assert: ( Object parseMethod: 'foo ^ 1' ) name == #foo.
!
!TestClass
testNewClass
	| sampleClass instance |
	Object subclass: #TestClassSample variables: #( a b ) classVariables: #( c ).
	sampleClass <- globals at: #TestClassSample.
	self assert: sampleClass name = #TestClassSample.
	self assert: sampleClass superclass == Object.
	self assert: sampleClass instanceVariables = #( a b ).
	sampleClass addMethod: 'a: x a <- x'.
	sampleClass addMethod: 'a ^ a'.
	sampleClass class addMethod: 'with: x ^ self new a: x'.
	instance <- sampleClass with: 5.
	self assert: instance a = 5.
	self assert: ( instance respondsTo: #a: ).
	self assert: ( instance isKindOf: sampleClass ).
	self assert: ( sampleClass addMethod: 'b ^ b' ) = 'method inserted: b'.
!
