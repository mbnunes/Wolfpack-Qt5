
import wolfpack

#
# Check the dyes
#
def checkdyes(char, item, wearout = 0):
  if not item:
    return 0

  # Has to be in our posession
  if item.getoutmostchar() != char:
    char.socket.clilocmessage(500364)
    return 0

  # We do not allow "invulnerable" tools.
  if not item.hastag('remaining_uses'):
    char.socket.clilocmessage(500858)
    item.delete()
    return 0

  if wearout:
    uses = int(item.gettag('remaining_uses'))
    uses -= 1
    if uses <= 0:
      char.socket.clilocmessage(500858)
      item.delete()
      return 0
    else:
      item.settag('remaining_uses', uses)
      item.resendtooltip()
    
  return 1

#
# Process the target response from the client
#
def target(player, arguments, target):
  # Check the target
  if not target.item or target.item.id != 0xfab:
    player.socket.clilocmessage(500857)
    return
    
  # Needs to in our belongings
  if target.item.getoutmostchar() != player:
    player.socket.clilocmessage()
    return
  
  # Wear out the tools
  dyes = wolfpack.finditem(arguments[0])
  if not checkdyes(player, dyes):
    return
    
  checkdyes(player, dyes, 1) # Wear out
  
  # Dye and play a soundeffect
  target.item.color = arguments[1]
  target.item.update()
  player.soundeffect(0x023e)

#
# Process the client response
#
def dyeresponse(socket, packet):
  serial = packet.getint(1)
  color = packet.getshort(7)

  # See if the color is invalid.
  if color < 2 or color > 0x3e9:
    socket.sysmessage("You selected an invalid color.")
    return 1
  
  # See if the serial is pointing to a valid object.
  dyes = wolfpack.finditem(serial)
  if checkdyes(socket.player, dyes):
    socket.clilocmessage(500856)
    socket.attachtarget("dyes.target", [dyes.serial, color])

  return 1

#
# Register our packet handler for the
# dye packet.
#
def onLoad():
  wolfpack.registerpackethook(0x95, dyeresponse)

#
# Show the hue selection gump.
#
def onUse(player, item):
  if checkdyes(player, item):
    packet = wolfpack.packet(0x95, 9)
    packet.setint(1, item.serial)
    packet.setshort(7, 0xfab)
    packet.send(player.socket)
  return 1

#
# Add the remaining uses to the tooltip
#
def onShowTooltip(viewer, object, tooltip):
  if object.hastag('remaining_uses'):
    tooltip.add(1060584, str(object.gettag('remaining_uses')))
