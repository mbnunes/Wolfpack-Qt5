from wolfpack import tr
import wolfpack

def onSpeech(listener, speaker, text, keywords):
	if speaker.distanceto(listener) > 3:
		return False
	
	if len(keywords) > 0:
		if 158 in keywords: # time
			listener.say(tr("It is now %s hours and %s minutes.") %( wolfpack.time.hour(),  wolfpack.time.minute()))
			return True
	
	text = text.lower()
	
	if "name" in text:
		listener.say(tr( "Hello, my name is %s." ) %listener.name)
		return True
	if "location" in text:
		if speaker.region:
			listener.say( tr( "You are in %s" ) % speaker.region.name )
		else:
			listener.say( tr( "You are in the wilderness" ) )
		return True
	return False
	
	