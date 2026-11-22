" the startup code of the concordance example: lst runs Main>>start, then exits "
" it reads the text from standard input: lst concordance.img < Smalltalk/Examples/Concordance/Input.txt "
+Application subclass: #Main variables: #( ) classVariables: #( )
!Main
start
	Concordance new run.
!
