
"""
	\library wolfpack.accounts
	\description Contains several server functions for accounts.
"""

import _wolfpack.accounts

"""
	\function wolfpack.accounts.find
	\param name
	\return Account
	\description Finds an account by its account name.
"""
def find( name ):
	return _wolfpack.accounts.find( name )

"""
	\function wolfpack.accounts.list
	\return List of Accounts
	\description Returns a list of all accounts.
"""
def list():
	return _wolfpack.accounts.list()

"""
	\function wolfpack.accounts.acls
	\return List of ACLs
	\description Returns a list of all account access levels.
"""
def acls():
	return _wolfpack.accounts.acls()

"""
	\function wolfpack.accounts.acl
	\param name
	\return True or False
	\description Returns true or false if the given ACL name exists.
"""
def acl( name ):
	return _wolfpack.accounts.acl( name )

"""
	\function wolfpack.accounts.add
	\param username
	\param password
	\return True or False
	\description Creates an account with the given username and password.
"""
def add( username, password ):
	return _wolfpack.accounts.add( username, password )

"""
	\function wolfpack.accounts.save
	\return None
	\description Saves the accounts.
"""
def save():
	return _wolfpack.accounts.save()

"""
	\function wolfpack.accounts.reload
	\return None
	\description Reloads the accounts.
"""
def reload():
	return _wolfpack.accounts.reload()
