
import _wolfpack.accounts

def find( name ):
	return _wolfpack.accounts.find( name )

def list():
	return _wolfpack.accounts.list()

def acls():
	return _wolfpack.accounts.acls()

def acl( name ):
	return _wolfpack.accounts.acl( name )

def add( username, password ):
	return _wolfpack.accounts.add( username, password )
