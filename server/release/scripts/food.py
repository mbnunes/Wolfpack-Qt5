
import random
from wolfpack.consts import *

#
# Eat the food
#
def onUse(player, item):
  # Has to belong to us.
  if item.getoutmostchar() != player:
    player.socket.clilocmessage(500866)
    return 1

  # Can we eat anymore?
  if player.hunger >= 6:
    player.socket.clilocmessage(500867)
    return 1
    
  player.socket.clilocmessage(min(500872, 500868 + player.hunger))
  
  if item.amount > 1:
    item.amount -= 1
    item.update()
  else:
    item.delete()
    
  # Fidget animation and munch munch sound
  player.soundeffect(random.choice([0x03a, 0x03b, 0x03c]))
  player.action(ANIM_FIDGET3)
    
  player.hunger += 1
  return 1
