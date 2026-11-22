" the startup code of the palindrome example: lst runs Main>>start, then exits "
+Application subclass: #Main variables: #( ) classVariables: #( )
!Main
start
	Palindrome new runTests.
!
