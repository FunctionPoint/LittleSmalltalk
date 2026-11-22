" class definition for Test "
" the superclass of the unit tests: a test class TestName tests class Name in its methods "
" test... (several groups: testCreation, testAccess...). A test checks with self assert: "
" Test all runs every test method of every subclass, each in its own Process, so an "
" error ends only that test. The class variables count the tests, the failures, the "
" errors and the asserts (in all tests, and in the current test method) "
+Object subclass: #Test variables: #( ) classVariables: #( tests failures errors totalAsserts asserts testName failed )
" class methods for Test "
=Test
all
	| names |
	" run the test methods of all subclasses of Test, in the order of their names, and report "
	tests <- 0.
	failures <- 0.
	errors <- 0.
	totalAsserts <- 0.
	names <- OrderedArray new.
	self subclasses do: [ :testClass | names <- names add: testClass name ].
	names do: [ :name | self run: ( globals at: name ) ].
	self report.
!
=Test
assert: aBoolean
	" count an assert of the current test method; report it if aBoolean is not true "
	asserts <- asserts + 1.
	totalAsserts <- totalAsserts + 1.
	aBoolean == true ifFalse: [
		failed ifFalse: [
			failures <- failures + 1.
			failed <- true ].
		( testName + ': assert ' + asserts printString + ' failed' ) printNl ].
!
=Test
errors
	^ errors.
!
=Test
failures
	^ failures.
!
=Test
isTestSelector: selector
	| name |
	" a test method: test... without arguments "
	name <- selector asString.
	name size < 4 ifTrue: [ ^ false ].
	( name includes: $: ) ifTrue: [ ^ false ].
	^ ( name from: 1 to: 4 ) = 'test'.
!
=Test
report
	( failures = 0 and: [ errors = 0 ] )
		ifTrue: [ ( 'All ' + tests printString + ' tests passed (' + totalAsserts printString + ' asserts)' ) printNl ]
		ifFalse: [ ( ( failures + errors ) printString + ' of ' + tests printString + ' tests failed: ' + failures printString + ' with a failed assert, ' + errors printString + ' with an error' ) printNl ].
!
=Test
run: aClass
	| selectors |
	" run the test methods of aClass, in the order of their names "
	selectors <- OrderedArray new.
	aClass methods keysDo: [ :selector |
		( self isTestSelector: selector ) ifTrue: [ selectors <- selectors add: selector ] ].
	selectors do: [ :selector | self run: aClass method: selector ].
!
=Test
run: aClass method: selector
	| result |
	" run one test method on a new instance of aClass, in a Process of its own "
	testName <- aClass name asString + '>>' + selector asString.
	tests <- tests + 1.
	asserts <- 0.
	failed <- false.
	result <- Context new
		perform: ( aClass methods at: selector )
		withArguments: ( Array with: aClass new ).
	" a test method returns itself; nil: the Process stopped with an error "
	result isNil ifTrue: [
		errors <- errors + 1.
		( testName + ': error' ) printNl ].
!
=Test
tests
	^ tests.
!
=Test
totalAsserts
	^ totalAsserts.
!
" instance methods for Test "
!Test
assert: aBoolean
	" the check of a test: aBoolean must be true "
	Test assert: aBoolean.
!
