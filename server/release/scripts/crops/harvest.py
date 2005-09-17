
import crops
import crops.growing
import wolfpack.utilities
from wolfpack.consts import *
import random

stages = crops.stages

def onUse(char, item):
	if not char.canreach(item, 2):
		char.socket.clilocmessage(500312) # You cannot reach that.
	elif char.ismounted():
		char.socket.clilocmessage(1040016) # You cannot use this while riding a mount
	else:
		doharvest(char, item)
	return True

def doharvest(char, item):
	# Pick the plant
	char.turnto(item)
	char.action(ANIM_BOW)

	harvested_item = wolfpack.additem( stages[item.baseid][-3] )
	harvested_item.amount = stages[item.baseid][-2]
	harvested_item.movable = 1
	harvested_item.decay = 1
	# Move into backpack
	if not wolfpack.utilities.tobackpack(harvested_item, char):
		harvested_item.update()

	restore(item)

def restore(item):
	# Change the look of the harvested item
	item.id = random.choice(stages[item.baseid][-1])
	item.removescript("crops.harvest")
	item.addscript("crops.growing") # restart growing
	item.update()

	return True
