
import wolfpack

#
# Teleport someone trough
#
def onCollide(player, item):
  if item.hastag('playersonly') and player.npc:
    return 0

  if not item.hastag('target'):    
    player.socket.sysmessage('This gate leads nowhere...')
    return 0
    
  target = item.gettag('target').split(',')

  # Convert the target of the gate.
  try:
    target = map(int, target)
  except:
    player.socket.sysmessage('This gate leads nowhere...')
    return 0

  # Move the player
  pos = player.pos
  pos.x = target[0]
  pos.y = target[1]
  pos.z = target[2]
  player.removefromview()
  player.moveto(pos)
  player.update(0, 1)
  if player.socket:
    player.socket.updateplayer()
    player.socket.resendworld()

  # show some nice effects
  if not item.hastag('silent'):  
    player.soundeffect(0x1fe)
    item.soundeffect(0x1fe)
    wolfpack.effect(0x3728, player.pos, 10, 30)
    pos = item.pos
    wolfpack.effect(0x3728, pos, 10, 30)
    
  return 1

#
# Here a gump should pop up and allow customization.
#
def onUse(player, item):
  if not player.gm:
    return 1

  return 1
