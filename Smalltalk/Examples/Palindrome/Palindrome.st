" A simple palindrome tester "
" first, add a method to class String "
!String
isPalindrome
	| string |
	string <- self select: [ :character | character isAlphabetic ].
	string <- string collect: [ :character | character lowerCase ].
	^ string = string reverse.
!
" next, add a new class with a few test cases "
+Object subclass: #Palindrome variables: #( ) classVariables: #( )
!Palindrome
test: aString
	aString print.
	' : ' print.
	aString isPalindrome print.
	Char newline print.
!
!Palindrome
runTests
	self test: 'A Man, A Plan, A Canal, Panama'.
	self test: 'Rats live on no Evil star'.
	self test: 'This is not a palindrome'.
!
