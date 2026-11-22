" unit tests for Test "
+Test subclass: #TestTest variables: #( ) classVariables: #( )
!TestTest
testSelectors
	self assert: ( Test isTestSelector: #test ).
	self assert: ( Test isTestSelector: #testFoo ).
	self assert: ( Test isTestSelector: #tes ) not.
	self assert: ( Test isTestSelector: #helper ) not.
	self assert: ( Test isTestSelector: #test:with: ) not.
!
!TestTest
testCounting
	| before |
	before <- Test totalAsserts.
	self assert: true.
	self assert: Test totalAsserts = ( before + 1 ).
	self assert: Test tests > 0.
	self assert: TestTest superclass == Test.
	self assert: ( Test subclasses asArray includes: TestTest ).
!
