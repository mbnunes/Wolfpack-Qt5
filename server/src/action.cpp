
#include "action.h"
#include "accounts.h"
#include "server.h"
#include "world.h"

cAction::cAction() {
}

cAction::~cAction() {
}

void cActionReloadScripts::execute() {
	Server::instance()->reload( "definitions" );
}

void cActionReloadPython::execute() {
	Server::instance()->reload( "scripts" );
}

void cActionReloadAccounts::execute() {
	Server::instance()->reload( "accounts" );
}

void cActionReloadConfiguration::execute() {
	Server::instance()->reload( "configuration" );
}

void cActionSaveWorld::execute() {
	World::instance()->save();
}

void cActionSaveAccounts::execute() {
	Accounts::instance()->save();
}
