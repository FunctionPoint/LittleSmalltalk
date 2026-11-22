" class definition for File "
+Object subclass: #File variables: #( fileID ) classVariables: #( )
" class methods for File "
=File
doOpen: fileName mode: mode
	<100 fileName mode>.
	" the file cannot be opened "
	^ nil.
!
=File
fileIn: fileName
	| file |
	file <- self openRead: fileName.
	file opened ifFalse: [ ^ self error: 'cannot open file ' + fileName ].
	file fileIn.
	file close.
	^ 'file in completed'.
!
=File
image: fileName
	| file |
	" open a file, write the image, then close "
	file <- self openWrite: fileName.
	file opened ifFalse: [ ^ self error: 'cannot open file ' + fileName ].
	file writeImage.
	file close.
!
=File
openRead: fileName
	" open new file for reading "
	^ self in: ( self new ) at: 1 put: ( self doOpen: fileName mode: 'r' ).
!
=File
openUpdate: fileName
	" open new file for reading and writing "
	^ self in: ( self new ) at: 1 put: ( self doOpen: fileName mode: 'r+' ).
!
=File
openWrite: fileName
	" open new file for writing "
	^ self in: ( self new ) at: 1 put: ( self doOpen: fileName mode: 'w' ).
!
" instance methods for File "
!File
at: position
	<108 fileID position>.
	self primitiveFailed.
!
!File
at: position get: buffer
	| size |
	self at: position.
	size <- buffer size.
	<106 fileID buffer size>.
!
!File
at: position put: buffer
	self at: position.
	self write: buffer size: buffer size.
!
!File
at: position size: count
	| buffer readCount |
	buffer <- ByteArray new: count.
	readCount <- self at: position get: buffer.
	( readCount < count ) ifTrue: [ buffer <- buffer from: 1 to: readCount ].
	^ buffer.
!
!File
close
	" close file, return file descriptor "
	fileID notNil ifTrue: [
		self close: fileID.
		fileID <- nil ].
!
!File
close: aFileID
	<103 aFileID>.
!
!File
doRead
	<101 fileID>.
	fileID isNil ifTrue: [ self notOpened ].
	self primitiveFailed.
!
!File
fileIn
	| command |
	[ command <- self readChar. command notNil ] whileTrue: [
		self fileInDispatch: command ].
!
!File
fileInDispatch: command
	| character |
	" Immediate execution "
	command = $+ ifTrue: [
		self readLine doIt printNl.
		^ self ].

	" Method definition, '!' -> instance method, '=' -> class method "
	( command = $! or: [ command = $= ] ) ifTrue: [
		self methodCommand: ( command = $! ).
		^ self ].

	" Comment enclosed in quotes... find matching quote "
	( command = $" ) ifTrue: [
		[ character <- self readChar. character ~= $" ] whileTrue: [
			" Consume chars until closing quote "
			nil ].
		^ self ].

	" Blank line, just return to process next line "
	( command = Char newline ) ifTrue: [
		^ self ].

	" It is random chars (treat as comment--discard) "
	self readLine.
!
!File
methodCommand: instanceSide
	| name aClass text line |
	name <- self readLine asSymbol.

	aClass <- globals at: name ifAbsent: [
		^ self error: 'unknown class name in file-in: ' + name printString ].

	text <- ''.
	[ line <- self readLine. line isNil ifTrue: [ ^ self error: 'unexpected end of input during fileIn' ]. line ~= '!' ] whileTrue: [
			text <- text + line + Char newline asString ].
	instanceSide
		ifTrue: [ ( aClass addMethod: text ) printNl ]
		ifFalse: [ ( aClass class addMethod: text ) printNl ].
!
!File
notOpened
	self error: 'file is not open'.
!
!File
opened
	^ fileID notNil.
!
!File
readChar
	| code |
	" read a single character from a file "
	code <- self doRead.
	code notNil ifTrue: [ ^ Char new: code ].
	^ code.
!
!File
readLine
	| value character newline |
	" read a line from input "
	fileID isNil ifTrue: [ self error: 'cannot read from unopened file' ].
	value <- ''.
	newline <- Char newline.
	[ character <- self doRead.
		character isNil ifTrue: [ ^ nil ].
		character <- Char new: character.
		character ~= newline ] whileTrue:
		[ value <- value + character asString ].
	^ value.
!
!File
write: buffer size: count
	<107 fileID buffer count>.
	self primitiveFailed.
!
!File
writeCharValue: code
	<102 fileID code>.
	fileID isNil ifTrue: [ self notOpened ].
	self primitiveFailed.
!
!File
writeImage
	" save the current image in a file "
	fileID notNil
		ifTrue: [ <104 fileID> ].
!
