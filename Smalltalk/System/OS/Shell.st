" class definition for Shell "
+Application subclass: #Shell variables: #( ) classVariables: #( )
" class methods for Shell "
" instance methods for Shell "
!Shell
start
	| command |
	'Welcome to Little Smalltalk!' printNl.
	" main execution loop "
	[ '-> ' print. command <- String input. command notNil ]
		whileTrue: [ command isEmpty
			ifFalse: [ command doIt printNl ] ].
!
