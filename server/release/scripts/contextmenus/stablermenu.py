
def onContextEntry(char, target, tag):
	if tag == 200:
		aiengine = target.aiengine()
		if aiengine:
			aiengine.onSpeechInput(char, 'vendor stable')
		return True

	elif tag == 201:
		aiengine = target.aiengine()
		if aiengine:
			aiengine.onSpeechInput(char, 'vendor claim')
		return True

	return False
