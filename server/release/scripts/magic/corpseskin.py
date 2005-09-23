
from wolfpack.utilities import changeResistance

def dispel(char, args, source, dispelargs):
	expire(char, dispelargs)

def expire(char, args):
	if char.socket and not args:
		char.socket.clilocmessage( 1061688 ) # Your skin returns to normal.
	char.removescript('magic.corpseskin')
	changeResistance( char, 'res_fire', 10 )
	changeResistance( char, 'res_poison', 10 )
	changeResistance( char, 'res_cold', - 10 )
	changeResistance( char, 'res_physical', - 10 )
	char.updatestats()
	return
