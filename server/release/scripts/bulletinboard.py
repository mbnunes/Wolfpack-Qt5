
import wolfpack
from wolfpack.consts import *
from struct import unpack
import time
from math import floor

def onLoad():
  wolfpack.registerglobal(EVENT_BULLETINBOARD, "bulletinboard")

def onBulletinBoard(char, packet):
  try:
    subcommand = packet.getbyte(3)
    board = wolfpack.finditem(packet.getint(4))

    if not board or not char.canreach(board, 3):
      char.socket.clilocmessage(1019045)
      return
    
    # Post a new message
    if subcommand == 5:
      replyto = packet.getint(8)
      subjectlen = packet.getbyte(12)
      subject = ""
      for i in range(0, subjectlen):
        byte = packet.getbyte(13+i)
        if byte != 0:
          subject += chr(byte)
      subject = subject.decode('utf-8')
      
      lines = []
      offset = 13 + subjectlen

      numlines = packet.getbyte(offset)
      offset += 1
    
      for i in range(0,numlines):
        linelen = packet.getbyte(offset)
        offset += 1

        line = ""
        for i in range(0, linelen):
          byte = packet.getbyte(offset+i)
          if byte != 0:
            line += chr(byte)
        lines.append(line.decode('utf-8'))
        offset += linelen

      if replyto != 0:
        replyto = wolfpack.finditem(replyto)

        if replyto and replyto.getoutmostitem() != board:
          replyto = board
      else:
        replyto = board

      if len(subject) > 0 and len(lines) > 0:
        message = wolfpack.newitem(1)
        message.id = 0xeb0 # Deed
        message.container = replyto
        
        # Set all necessary tags
        message.settag('poster_name', char.name)
        message.settag('poster_serial', char.serial)
        message.settag('poster_hue', char.skin)
        message.settag('poster_body', char.id)
        
        # Store Equipment
        equipmentcount = 0

        for i in range(1, LAYER_NPCRESTOCK):
          item = char.itemonlayer(i)
          if item:
            message.settag('equipment%uid' % equipmentcount, item.id)
            message.settag('equipment%uhue' % equipmentcount, item.color)
            equipmentcount += 1

        message.settag('equipmentcount', equipmentcount)

        message.settag('time', time.time())
        message.settag('subject', subject)
        message.settag('lines', len(lines))
        for i in range(0, len(lines)):
          message.settag('line%u' % i, lines[i])

        message.update()
    
    # Request the whole message
    elif subcommand == 3:
      message = wolfpack.finditem(packet.getint(8))
      if message and message.getoutmostitem() == board:
        subject = ''
        subjectlen = len(subject)+1
        poster = ''
        posterlen = len(poster)+1
        timestr = ''
        timestrlen = len(timestr)+1
        lineslen = 1 # Byte value of how many lines are there = 1 byte
        lines = []

        if message.hastag('subject'):
          subject = message.gettag('subject')
          subjectlen = len(subject.encode('utf-8')) + 1
  
        if message.hastag('poster_name'):
          poster = message.gettag('poster_name')
          posterlen = len(poster.encode('utf-8')) + 1

        if message.hastag('time'):
          message_time = int(message.gettag('time'))
          current_time = time.time()

          # Calculate the difference and print out in days + minutes
          diff = current_time - message_time
          if diff < 0:
            diff = 0

          days = 0

          if diff >= 86400:
            days = floor(diff/86400)
            diff %= 86400

          hours = floor(diff / 1440)
          minutes = floor(diff % 1440 / 60)

          if (days != 0):
            timestr = '%u Days %02u:%02u' % (days, hours, minutes)
          else:
            timestr = '%02u:%02u' % (hours, minutes)

          timestrlen = 2 + len(timestr.encode('utf-8'))

        if message.hastag('lines'):
          linecount = int(message.gettag('lines'))

          for i in range(0, linecount):
            if message.hastag('line%u' % i):
              line = message.gettag('line%u' % i)
              lines.append(line)              
              lineslen += 2 + len(line.encode('utf-8'))

        equipment = []

        if message.hastag('equipmentcount'):
          equipmentcount = int(message.gettag('equipmentcount'))

          for i in range(0,equipmentcount):
            id = int(message.gettag('equipment%uid'%i))
            hue = int(message.gettag('equipment%uhue'%i))
            equipment.append([id, hue])

        packet_len = 15 + posterlen + subjectlen + timestrlen
        # Whole Message mode
        packet_len += lineslen + 5 + len(equipment) * 4

        packet = wolfpack.packet(0x71, packet_len)
        packet.setshort(1, packet_len)
        packet.setbyte(3, 2) # Whole message
        packet.setint(4, board.serial)
        packet.setint(8, message.serial)
        packet.setbyte(12, posterlen)
        packet.setascii(13, poster)
        packet.setbyte(13 + posterlen, subjectlen)
        packet.setascii(14 + posterlen, subject)
        packet.setbyte(14 + posterlen + subjectlen, timestrlen)
        packet.setascii(15 + posterlen + subjectlen, timestr)

        # Skip 5 byte (character data???)
        offset = 15 + posterlen + subjectlen + timestrlen
        
        packet.setshort(offset, 0x191) # Poster Body
        packet.setshort(offset + 2, 0x840a) # Poster Hue
        packet.setbyte(offset + 4, len(equipment)) # Count of Equipment (Short Id, Short Hue)
        offset += 5

        for data in equipment:
          packet.setshort(offset, data[0])
          packet.setshort(offset+2, data[1])
          offset += 4

        packet.setbyte(offset, len(lines))
        offset += 1
        
        for line in lines:
          linelen = len(line.encode('utf-8')) + 1

          packet.setbyte(offset, linelen)
          packet.setascii(offset + 1, line)
          offset += linelen + 1

        packet.send(char.socket)

      else:
        char.socket.sysmessage("You can't read this message.")

    # Request message summary
    elif subcommand == 4:
      message = wolfpack.finditem(packet.getint(8))

      if message and message.getoutmostitem() == board:
        subject = ''
        subjectlen = len(subject)+1
        poster = ''
        posterlen = len(poster)+1
        timestr = 'Unknown'
        timestrlen = len(timestr)+1

        if message.hastag('subject'):
          subject = message.gettag('subject')          
          subjectlen = len(subject.encode('utf-8')) + 1
  
        if message.hastag('poster_name'):
          poster = message.gettag('poster_name')
          posterlen = len(poster.encode('utf-8')) + 1

        if message.hastag('time'):
          message_time = int(message.gettag('time'))
          current_time = time.time()

          # Calculate the difference and print out in days + minutes
          diff = current_time - message_time
          if diff < 0:
            diff = 0

          days = 0

          if diff >= 86400:
            days = floor(diff/86400)
            diff %= 86400

          hours = floor(diff / 1440)
          minutes = floor(diff % 1440 / 60)

          if days == 1:
            timestr = '%u day %02u:%02u' % (days, hours, minutes)
          elif days > 1:
            timestr = '%u days %02u:%02u' % (days, hours, minutes)
          else:
            timestr = '%02u:%02u' % (hours, minutes)

          timestrlen = 1 + len(timestr.encode('utf-8'))

        packet_len = 19 + posterlen + subjectlen + timestrlen

        packet = wolfpack.packet(0x71, packet_len)
        packet.setshort(1, packet_len)
        packet.setbyte(3, 1) # Message summary
        packet.setint(4, board.serial)
        packet.setint(8, message.serial)

        if message.container != board:
          packet.setint(12, message.container.serial) # parent

        packet.setbyte(16, posterlen)
        packet.setascii(17, poster)
        packet.setbyte(17 + posterlen, subjectlen)
        packet.setascii(18 + posterlen, subject)
        packet.setbyte(18 + posterlen + subjectlen, timestrlen)
        packet.setascii(19 + posterlen + subjectlen, timestr)
        packet.send(char.socket)

      else:
        char.socket.sysmessage("You can't read this message.")

  except:
    char.socket.sysmessage('Invalid bulletin board packet.')
    raise

def findMessages(parent, messages):
  for item in parent.content:
    messages.append(item.serial)
    
    if len(item.content) > 0:
      messages = findMessages(item, messages)

  return messages

def onUse(char, board):
  if not char.canreach(board, 3):
    char.socket.clilocmessage(1019045)
    return 1

  # Build a packet for the bulletin board index
  packet = wolfpack.packet(0x71, 38)
  packet.setshort(1, 38)
  packet.setbyte(3, 0) # Display Bulletin Board
  packet.setint(4, board.serial) # Bulletin Board Serial

  # Bulletin Board name in UTF-8 encoding (max. 29 chars)
  if len(board.name) == 0:
    name = 'bulletin board'
  else:
    name = board.name.encode('utf-8')

  for i in range(0,min(len(name),30)):
    packet.setbyte(8 + i, ord(name[i]))

  packet.send(char.socket)

  # Collect message information for this board
  messages = findMessages(board, [])

  # Send content of container to socket
  packet = wolfpack.packet(0x3c, 5 + 19 * len(messages))
  packet.setshort(1, packet.size)
  packet.setshort(3, len(messages))

  offset = 5

  for item in messages:
    packet.setint(offset, item)
    packet.setshort(offset + 4, 0xeb0)
    packet.setbyte(offset + 6, 0)
    packet.setshort(offset + 7, 0)
    packet.setshort(offset + 9, 0)
    packet.setshort(offset + 11, 0)
    packet.setint(offset + 13, board.serial)
    packet.setshort(offset + 17, 0)
    offset += 19

  packet.send(char.socket)

  return 1
