
#
# A sphere-style script parser
#
class Parser:
	def __init__(self):
		self.lines = []
		self.position = 0 # The line we currently are at.

	#
	# Token types
	#
	TOKEN_HEADER = 1
	TOKEN_ASSIGNMENT = 2

	#
	# Load a file and skip all unwanted tokens
	# and newlines.
	#
	def load(filename):
		file = open(filename, 'rU') 
		self.lines = file.readlines()
		file.close()

		# Delete all newlines or lines starting with //
		

	#
	# Get the next token type and value
	#
	def gettoken():
		pass

	#
	# Return a string.
	#
	def getstring():
		pass
