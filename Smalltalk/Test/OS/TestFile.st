" unit tests for File "
+Test subclass: #TestFile variables: #( ) classVariables: #( )
!TestFile
testReading
	| file |
	" the image this test runs in: a file that is there whatever the current folder is "
	file <- File openRead: self imageFileName.
	self assert: file opened.
	" an image starts with the header lst! "
	self assert: file readChar = $l.
	self assert: file readChar = $s.
	self assert: file readChar = $t.
	self assert: file readChar = $!.
	file close.
	self assert: file opened not.
!
!TestFile
testMissing
	self assert: ( File openRead: 'no such file' ) opened not.
!
!TestFile
testClosing
	| file fileNumber |
	file <- File openRead: self imageFileName.
	fileNumber <- self fileNumberOf: file.
	self assert: ( self readCharacterFrom: fileNumber ) = $l value.
	file close.
	" a closed file's number no longer reads from it "
	self assert: ( self readCharacterFrom: fileNumber ) = #failed.
	" and the number is free again for the next file "
	file <- File openRead: self imageFileName.
	self assert: ( self fileNumberOf: file ) = fileNumber.
	file close.
!
!TestFile
fileNumberOf: aFile
	" the file number of an open File (its first instance variable) "
	^ self in: aFile at: 1.
!
!TestFile
readCharacterFrom: fileNumber
	" read a character with primitive 101: its code, nil at the end, #failed if fileNumber is not an open file "
	<101 fileNumber>.
	^ #failed.
!
!TestFile
imageFileName
	| arguments name index |
	" the image on the command line lst [-v] [image]: the argument that is not an "
	" option, else lst.img "
	arguments <- Application new args.
	name <- 'lst.img'.
	index <- 2.
	[ index <= arguments size ] whileTrue: [
		( ( arguments at: index ) at: 1 ) = $- ifFalse: [ name <- arguments at: index ].
		index <- index + 1 ].
	^ name.
!
