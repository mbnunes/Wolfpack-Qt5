
import wolfpack
from wolfpack import tr
import wolfpack.gumps
from wolfpack.consts import *
from guilds import *
from math import ceil, floor
from time import strftime, localtime

# Main Menu Constants
MAINMENU_GENERALPROPERTIES = 1
MAINMENU_MEMBERS = 2
MAINMENU_CANIDATES = 3
MAINMENU_DIPLOMACY = 4
MAINMENU_MEMBERSHIP = 5
MAINMENU_DISBANDGUILD = 6

def guildMemberDetailsResponse(player, arguments, response):
  guild = wolfpack.findguild(arguments[0])

  if not guild:
    player.socket.sysmessage(tr('The guild you try to manage has already been disbanded.'))
    return

  char = wolfpack.findchar(arguments[1])

  if not char or not char in guild.members:
    player.socket.sysmessage(tr('The character has already left this guild.'))
    guildMembers(player, guild)
    return

  if response.button == 0:
    guildMembers(player, guild)

  elif response.button == 1:
    # Edit the permissions
    if checkPermission(player, guild, PERMISSION_GRANT) and player != char and guild.leader != char:
      permissions = 0
      for switch in response.switches:
        permissions |= switch

      #player.message(tr('Original Switches: %x') % permissions)

      # We may only award permissions we have for ourself
      if not player.gm and player != guild.leader:
        permissions &= getPermissions(player, guild)
      org_permissions = getPermissions(char, guild)

      #player.message('After Own Permissions: %x' % permissions)

      # Make sure that the permissions we cannot grant are inherited from his
      # current permissions
      permissions |= org_permissions & ~getPermissions(player, guild)

      #player.message('Including Inherited: %x' % permissions)

      # Only change if neccesary
      if org_permissions != permissions:
        setPermissions(char, guild, permissions)
        player.socket.sysmessage(tr('The permissions for %s have been modified.') % char.name)
        sendMessage(char, tr("Your guild permissions have been modified by %s. See your guildstone for details.") % player.name)

    # Awards a guildtitle
    if checkPermission(player, guild, PERMISSION_TITLES):
      title = response.text[1]
      info = guild.getmemberinfo(char)
      if title != info['guildtitle']:
        info['guildtitle'] = title
        guild.setmemberinfo(char, info)
        sendMessage(char, tr("You have been granted the title '%s' by %s.") % (title, player.name))
        player.socket.sysmessage(tr('The guildtitle for %s has been changed.') % char.name)

    char.resendtooltip()
    guildMembers(player, guild)

  # Send a message to char
  elif response.button == 2:
    message = response.text[2]

    if len(message) > 0 and message != tr('Enter message...'):
      if char.socket:
        player.socket.sysmessage(tr('Your message to %s has been sent.') % char.name)
        char.socket.sysmessage(tr('%s: %s') % (player.name, message), player.saycolor)
      else:
        setPrivateMessage(char, player, message)
        player.socket.sysmessage(tr('Your message has been queued up. You may edit your message on the guildstone until the recipient logs in.'))
    else:
      setPrivateMessage(char, player, None)
      guildMemberDetails(player, guild, char)

    guildMembers(player, guild)

  # Kick a member out of the guild
  elif response.button == 3:
    if not checkPermission(player, guild, PERMISSION_KICK):
      player.socket.sysmessage(tr('You lack the permission to kick members out of this guild.'))
    elif char != player and char != guild.leader:
      guild.removemember(char)
      char.resendtooltip()
      sendMessage(char, tr('You have been removed from your guild by %s.') % player.name)
      for member in guild.members:
        sendMessage(member, tr('%s has been removed from your guild by %s.') % (char.name, player.name))
      if guild.guildstone:
        guild.guildstone.resendtooltip()
    guildMembers(player, guild)

  # Change your vote for a leader voting
  elif response.button == 4:
    # Voting is only possible if there actually _is_ an election going on
    if not guild.leader:
      # Set the vote and then see if there are enough votes to promote a new leader
      vote = getVote(player, guild)

      # Place a new vote
      if 0x8001 in response.switches and vote != char.serial:
        # If we previously voted for someone else, tell him he doesnt have
        # our vote anymore.
        previous = wolfpack.findchar(vote)
        if previous and previous in guild.members:
          sendMessage(previous, tr('%s has withdrawn the vote for your leadership position.') % player.name)

        if char != player:
          player.socket.sysmessage(tr('You place your vote for %s.') % char.name)
          sendMessage(char, tr('%s has voted for your leadership position.') % player.name)
        else:
          player.socket.sysmessage(tr('You place your vote for yourself.'))

        setVote(player, guild, char.serial)

      # Withdraw a specific vote
      elif 0x8001 not in response.switches and vote == char.serial:
        if char != player:
          player.socket.sysmessage(tr('You withdraw your vote for %s.') % char.name)
          sendMessage(char, tr('%s has withdrawn the vote for your leadership position.') % player.name)
        else:
          player.socket.sysmessage(tr('You withdraw your vote for yourself.'))

        setVote(player, guild, 0)

      # Count votes and see if someone has more than 50% of all members votes
      # NOTE: He has to vote for himself to accept the leadership offer
      membercount = len(guild.members)
      canidates = {}
      members = guild.members
      for member in members:
        canidate = wolfpack.findchar(getVote(member, guild))
        if canidate in members:
          if canidates.has_key(canidate):
            canidates[canidate] += 1
          else:
            canidates[canidate] = 1

      for (canidate, votes) in canidates.items():
        if votes > floor(membercount * 0.50):
          # Member has enough votes to be a leader
          # Check if he voted for himself
          canidatevote = getVote(canidate, guild)
          if canidatevote == canidate.serial:
            for member in guild.members:
              if member != canidate:
                sendMessage(member, tr('%s has been elected as the new leader of your guild.') % canidate.name)
              else:
                sendMessage(canidate, tr('You have been elected as the new leader of your guild.'))
            guild.leader = canidate
            canidate.resendtooltip()
            if guild.guildstone:
              guild.guildstone.resendtooltip()
          else:
            sendMessage(canidate, tr('You have enough votes to become the new leader of your guild. To accept this result, please vote for yourself at your guildstone.'))
          break

      guildMembers(player, guild)

def guildMemberDetails(player, guild, char):
  name = guild.name
  abbreviation = ''
  if len(guild.abbreviation) > 0:
    abbreviation = tr(' [%s]') % guild.abbreviation
  info = guild.getmemberinfo(char)

  if guild.leader != char:
    status = tr('Member')
  else:
    status = tr('Leader')

  # Show detail menu
  dialog = wolfpack.gumps.cGump()
  dialog.setCallback("guilds.stone.guildMemberDetailsResponse")
  dialog.setArgs([guild.serial, char.serial])

  dialog.startPage(0)
  dialog.addResizeGump(64, 34, 9260, 467, 516)
  dialog.addGump(30, 33, 10421, 0)
  dialog.addGump(59, 22, 10420, 0)
  dialog.addGump(43, 172, 10422, 0)
  dialog.addResizeGump(100, 79, 9200, 405, 65)
  dialog.addTilePic(106, 49, 3805)
  dialog.addText(160, 90, tr("Guildstone for %s%s") % (name, abbreviation), 380)
  dialog.addText(160, 113, tr("Member Details"), 2100)
  dialog.addResizeGump(100, 148, 9200, 405, 355)
  dialog.addGump(14, 468, 10402, 0)
  dialog.addButton(103, 511, 247, 248, 1)
  dialog.addButton(178, 511, 242, 241, 0)
  dialog.addGump(14, 324, 10401, 0)

  dialog.startPage(1)
  dialog.addText(120, 190, tr("Member Name: %s") % char.name, 2100)
  dialog.addText(240, 160, tr("General Information"), 380)
  dialog.addText(120, 220, tr("Member Since: %s") % strftime(FORMAT_DATETIME, localtime(info['joined'])), 2100)
  dialog.addText(120, 250, tr("Membership Status: %s") % status, 2100)

  if checkPermission(player, guild, PERMISSION_TITLES):
    dialog.addText(120, 280, tr("Guild Title:"), 2100)
    dialog.addResizeGump(196, 276, 9300, 252, 29)
    dialog.addInputField(200, 279, 244, 23, 2100, 1, info['guildtitle'])
  else:
    dialog.addText(120, 280, tr("Guild Title: %s") % info['guildtitle'], 2100)

  if info['showsign']:
    dialog.addText(120, 310, tr('Shows Guildsign: Yes'), 2100)
  else:
    dialog.addText(120, 310, tr('Shows Guildsign: No'), 2100)

  dialog.addHtmlGump(120, 345, 366, 108, tr("<basefont color=\"#666666\"><h3><u>Profile Information:</u></h3><basefont color=\"#333333\"><br>%s") % char.profile, 1, 1)
  dialog.addPageButton(459, 468, 9903, 9905, 2)
  dialog.addText(377, 469, tr("Permissions"), 980)

  dialog.startPage(2)
  dialog.addText(264, 160, tr("Permissions"), 380)
  dialog.addText(149, 468, tr("General Information"), 905)
  dialog.addPageButton(122, 468, 9909, 9911, 1)
  dialog.addPageButton(459, 468, 9903, 9905, 3)
  dialog.addText(317, 469, tr("Miscellaneous Actions"), 980)

  # NOTE: You may NOT modify your own permissions AND you may only grant permissions you have yourself
  color = [2100, 2100, 2100, 2100, 2100, 2100, 2100, 2100]
  permissions = getPermissions(player, guild)

  # Naturally as a gm or leader of this guild, we may modify the permissions
  # of all others
  if player.gm or player == guild.leader:
    permissions = 0xff

  # You may not modify your own or the leaders permissions
  if char == player or char == guild.leader:
    permissions = 0

  if permissions & (PERMISSION_GRANT|0x01) == PERMISSION_GRANT|0x01:
    dialog.addCheckbox(116, 187, 9721, 9724, 1, checkPermission(char, guild, 1, 1))
  else:
    if checkPermission(char, guild, 0x01):
      dialog.addGump(116, 187, 9724, 980)
    else:
      dialog.addGump(116, 187, 9721, 980)
    color[0] = 980

  if permissions & (PERMISSION_GRANT|0x02) == PERMISSION_GRANT|0x02:
    dialog.addCheckbox(116, 219, 9721, 9724, 2, checkPermission(char, guild, 2, 1))
  else:
    if checkPermission(char, guild, 0x02, 1):
      dialog.addGump(116, 219, 9724, 980)
    else:
      dialog.addGump(116, 219, 9721, 980)
    color[1] = 980

  if permissions & (PERMISSION_GRANT|0x04) == PERMISSION_GRANT|0x04:
    dialog.addCheckbox(116, 251, 9721, 9724, 4, checkPermission(char, guild, 4, 1))
  else:
    if checkPermission(char, guild, 0x04, 1):
      dialog.addGump(116, 251, 9724, 980)
    else:
      dialog.addGump(116, 251, 9721, 980)
    color[2] = 980

  if permissions & (PERMISSION_GRANT|0x08) == PERMISSION_GRANT|0x08:
    dialog.addCheckbox(116, 283, 9721, 9724, 8, checkPermission(char, guild, 8, 1))
  else:
    if checkPermission(char, guild, 0x08, 1):
      dialog.addGump(116, 283, 9724, 980)
    else:
      dialog.addGump(116, 283, 9721, 980)
    color[3] = 980

  if permissions & (PERMISSION_GRANT|0x10) == PERMISSION_GRANT|0x10:
    dialog.addCheckbox(116, 315, 9721, 9724, 16, checkPermission(char, guild, 16, 1))
  else:
    if checkPermission(char, guild, 0x10, 1):
      dialog.addGump(116, 315, 9724, 980)
    else:
      dialog.addGump(116, 315, 9721, 980)
    color[4] = 980

  if permissions & (PERMISSION_GRANT|0x20) == PERMISSION_GRANT|0x20:
    dialog.addCheckbox(116, 347, 9721, 9724, 32, checkPermission(char, guild, 32, 1))
  else:
    if checkPermission(char, guild, 0x20, 1):
      dialog.addGump(116, 347, 9724, 980)
    else:
      dialog.addGump(116, 347, 9721, 980)
    color[5] = 980

  if permissions & (PERMISSION_GRANT|0x40) == PERMISSION_GRANT|0x40:
    dialog.addCheckbox(116, 379, 9721, 9724, 64, checkPermission(char, guild, 64, 1))
  else:
    if checkPermission(char, guild, 0x40, 1):
      dialog.addGump(116, 379, 9724, 980)
    else:
      dialog.addGump(116, 379, 9721, 980)
    color[6] = 980

  if permissions & (PERMISSION_GRANT|0x80) == PERMISSION_GRANT|0x80:
    dialog.addCheckbox(116, 411, 9721, 9724, 128, checkPermission(char, guild, 128, 1))
  else:
    if checkPermission(char, guild, 0x80, 1):
      dialog.addGump(116, 411, 9724, 980)
    else:
      dialog.addGump(116, 411, 9721, 980)
    color[7] = 980

  dialog.addText(150, 190, tr("May edit the guild properties."), color[0])
  dialog.addText(152, 224, tr("May recruit new canidates for the guild."), color[1])
  dialog.addText(152, 256, tr("May promote canidates to full member status."), color[2])
  dialog.addText(152, 288, tr("May act as an diplomat for this guild."), color[3])
  dialog.addText(152, 320, tr("May kick members out of this guild."), color[4])
  dialog.addText(152, 352, tr("May expel canidates from the guild."), color[5])
  dialog.addText(152, 384, tr("May grant guild titles to members of this guild."), color[6])
  dialog.addText(152, 416, tr("May grant permissions to other guild members."), color[7])

  dialog.startPage(3)
  dialog.addText(232, 160, tr("Miscellaneous Actions"), 380)
  dialog.addResizeGump(112, 184, 9200, 380, 100)

  # Sending yourself a message is silly
  if char != player:
    dialog.addText(127, 192, tr("Send this member a private message:"), 2123)
    dialog.addResizeGump(124, 217, 9300, 355, 27)

    message = getPrivateMessage(char, player)
    if message:
      dialog.addInputField(128, 220, 350, 20, 2100, 2, message[2])
    else:
      dialog.addInputField(128, 220, 350, 20, 2100, 2, tr("Enter message..."))
    dialog.addButton(124, 252, 247, 248, 2)
  else:
    dialog.addText(127, 192, tr("Send this member a private message:"), 980)
    dialog.addResizeGump(124, 217, 9350, 355, 27)
    dialog.addText(128, 220, tr("Enter message..."), 980)
    dialog.addGump(124, 252, 247, 980)

  # Noone can kick himself or the leader out of the guild
  dialog.addResizeGump(112, 290, 9200, 380, 66)
  if char != player and guild.leader != char:
    dialog.addText(127, 298, tr("Kick this member out of the guild:"), 2117)
    dialog.addButton(124, 324, 247, 248, 3)
  else:
    dialog.addText(127, 298, tr("Kick this member out of the guild:"), 980)
    dialog.addGump(124, 324, 247, 980)

  # If the guild already has a leader, no voting is in progress
  dialog.addResizeGump(112, 363, 9200, 380, 94)
  if not guild.leader:
    dialog.addText(127, 371, tr("Vote for a new leader:"), 2106)
    dialog.addButton(124, 425, 247, 248, 4)
    dialog.addText(163, 395, tr("You vote for this member."), 2100)
    dialog.addCheckbox(127, 390, 9721, 9724, 0x8001, getVote(player, guild) == char.serial)
  else:
    dialog.addText(127, 371, tr("Vote for a new leader:"), 980)
    dialog.addGump(124, 425, 247, 980)
    dialog.addGump(127, 390, 9721, 980)
    dialog.addText(163, 395, tr("You vote for this member."), 980)

  dialog.addPageButton(122, 468, 9909, 9911, 2)
  dialog.addText(149, 468, tr("Permissions"), 905)

  dialog.send(player)

def guildMembersResponse(player, arguments, response):
  guild = wolfpack.findguild(arguments[0])

  if not guild:
    player.socket.sysmessage(tr('The guild you try to manage has already been disbanded.'))
    return

  if response.button == 0 or len(response.switches) != 1:
    mainMenu(player, guild)
  else:
    # Show a detail menu if possible
    char = wolfpack.findchar(response.switches[0])

    if not char or not char.player or char not in guild.members:
      guildMembers(player, guild)
    else:
      guildMemberDetails(player, guild, char)

def guildMembers(player, guild):
  name = guild.name
  abbreviation = ''
  if len(guild.abbreviation) > 0:
    abbreviation = tr(' [%s]') % guild.abbreviation

  dialog = wolfpack.gumps.cGump()
  dialog.setCallback("guilds.stone.guildMembersResponse")
  dialog.setArgs([guild.serial])

  dialog.startPage(0)
  dialog.addResizeGump(64, 34, 9260, 467, 516)
  dialog.addGump(30, 33, 10421, 0)
  dialog.addGump(59, 22, 10420, 0)
  dialog.addGump(43, 172, 10422, 0)
  dialog.addResizeGump(100, 79, 9200, 405, 65)
  dialog.addTilePic(106, 49, 3805)
  dialog.addText(160, 90, tr("Guildstone for %s%s") % (name, abbreviation), 380)
  dialog.addText(160, 113, tr("Member Overview"), 2100)
  dialog.addResizeGump(100, 148, 9200, 405, 355)
  dialog.addGump(14, 468, 10402, 0)
  dialog.addButton(103, 511, 247, 248, 1)
  dialog.addButton(178, 511, 242, 241, 0)
  dialog.addGump(14, 324, 10401, 0)
  dialog.addText(114, 158, tr("Select a member you wish to see details about:"), 2100)
  dialog.startGroup(0)

  members = guild.members
  offset = 0
  page = 0
  pages = ceil(len(members) / 4.0)

  for member in members:
    if offset == 0:
      page += 1
      dialog.startPage(page)

      if page > 1:
        dialog.addPageButton(112, 468, 9909, 9911, page - 1)
        dialog.addText(139, 468, tr("Previous Page"), 905)

      if page < pages:
        dialog.addPageButton(470, 468, 9903, 9905, page + 1)
        dialog.addText(401, 468, tr("Next Page"), 980)

    info = guild.getmemberinfo(member)

    dialog.addResizeGump(110, 184 + offset, 9350, 384, 64)
    dialog.addText(154, 195 + offset, member.name, 2100)
    dialog.addRadioButton(118, 190 + offset, 9721, 9724, member.serial, 0)
    dialog.addText(118, 223 + offset, tr("Joined: ") + strftime(FORMAT_DATE, localtime(info['joined'])), 2100)

    if len(info['guildtitle']) > 0:
      dialog.addText(259, 223 + offset, tr("Title: %s") % info['guildtitle'], 2100)
    else:
      dialog.addText(259, 223 + offset, tr("No Title"), 2100)

    if offset == 210:
      offset = 0
    else:
      offset += 70

  dialog.send(player)

# Simply return to the main menu
def guildCanidatesOverviewResponse(player, arguments, response):
  guild = wolfpack.findguild(arguments[0])
  if guild:
    mainMenu(player, guild)

# Display a read only list of canidates
def guildCanidatesOverview(player, guild):
  name = guild.name
  abbreviation = ''
  if len(guild.abbreviation) > 0:
    abbreviation = ' [%s]' % guild.abbreviation

  offline = []
  online = []
  for canidate in guild.canidates:
    if canidate.socket:
      online.append(canidate.name)
    else:
      offline.append(canidate.name)

  html = ''

  if len(online) > 0:
    html += tr('<basefont color="#666666"><h3><u>Online:</u></h3><basefont color="#333333"><br>')
    for name in online:
      html += tr('%s<br>') % name
    html += tr('<br>')

  if len(offline) > 0:
    html += tr('<basefont color="#666666"><h3><u>Offline:</u></h3><basefont color="#333333"><br>')
    for name in offline:
      html += tr('%s<br>') % name
    html += tr('<br>')

  if len(offline) == 0 and len(online) == 0:
    html += tr('There are currently no known canidates for a membership in this guild.')
    html += tr('<br><br>')

  # Append a list of Members responsible for recruitment
  recruitment = []
  for member in guild.members:
    if member == guild.leader:
      recruitment.append(member.name + tr(' (Leader)'))
    elif checkPermission(member, guild, PERMISSION_RECRUIT, 1):
      recruitment.append(member.name)

  if len(recruitment) != 0:
    html += tr('<basefont color="#666666"><h3><u>Responsible For Recruitment:</u></h3><basefont color="#333333"><br>')
    for member in recruitment:
      html += tr('%s<br>') % member
    html += tr('<br><br>')
  else:
    html += tr('There is currently no one responsible for recruiting canidates.')
    html += tr('<br><br>')

  dialog = wolfpack.gumps.cGump()
  dialog.setCallback("guilds.stone.guildCanidatesOverviewResponse")
  dialog.setArgs([guild.serial])
  dialog.startPage(0)
  dialog.addResizeGump(64, 34, 9260, 464, 462)
  dialog.addGump(30, 33, 10421, 0)
  dialog.addGump(59, 22, 10420, 0)
  dialog.addGump(43, 173, 10422, 0)
  dialog.addResizeGump(100, 79, 9200, 405, 65)
  dialog.addTilePic(106, 49, 3805)
  dialog.addText(160, 90, tr("Guildstone for %s%s") % (name, abbreviation), 380)
  dialog.addText(160, 113, tr("Canidate Overview"), 2100)
  dialog.addResizeGump(100, 149, 9200, 404, 283)
  dialog.addGump(13, 339, 10402, 0)
  dialog.addButton(102, 450, 247, 248, 0)
  dialog.addText(113, 158, tr("Canidates"), 2100)
  dialog.addHtmlGump(110, 180, 384, 240, html, 1, 1)

  dialog.send(player)

def recruitResponse(player, arguments, target):
  guild = wolfpack.findguild(arguments[0])

  if not guild:
    player.socket.sysmessage(tr('The guild you try to manage has already been disbanded.'))
    return

  if not checkPermission(player, guild, PERMISSION_RECRUIT):
    player.socket.sysmessage(tr('You are not allowed to recruit canidates for this guild.'))
  else:
    if not target.char or not target.char.player or not target.char.socket:
      player.socket.sysmessage(tr('You can only recruit players for your guild.'))
    else:
      if target.char.guild:
        player.socket.sysmessage(tr('That character is already in a guild.'))
      else:
        for member in guild.members:
          sendMessage(member, tr("A new canidate %s has been recruited into your guild by %s.") % (target.char.name, player.name))

        sendMessage(target.char, tr('You have been recruited as a canidate for %s by %s.') % (guild.name, player.name))
        guild.addcanidate(target.char)
        target.char.addscript( 'guilds.member' )
        target.char.resendtooltip()
        if guild.guildstone:
          guild.guildstone.resendtooltip()
        guildCanidates(player, guild)

def guildCanidatesResponse(player, arguments, response):
  guild = wolfpack.findguild(arguments[0])

  if not guild:
    player.socket.sysmessage(tr('The guild you try to manage has already been disbanded.'))
    return

  if response.button == 0:
    mainMenu(player, guild)
  else:
    if len(response.switches) == 1:
      switch = response.switches[0]

      # Expel a canidate
      if switch & 0x80000000:
        if checkPermission(player, guild, PERMISSION_EXPEL):
          switch &= ~0x80000000
          char = wolfpack.findchar(switch)
          if char.player and char in guild.canidates:
            guild.removecanidate(char)
            sendMessage(char, tr("You have been expelled from your guild by %s.") % player.name)
            for member in guild.members:
              sendMessage(member, tr("The canidate %s has been expelled from your guild by %s.") % (char.name, player.name))
            char.resendtooltip()
            if guild.guildstone:
              guild.guildstone.resendtooltip()
          else:
            player.socket.sysmessage(tr('The selected player is not a canidate of this guild.'))
        else:
          player.socket.sysmessage(tr('You are not allowed to expel canidates in this guild.'))

      # Promote a canidate
      elif switch & 0x40000000:
        if checkPermission(player, guild, PERMISSION_PROMOTE):
          switch &= ~0x40000000
          char = wolfpack.findchar(switch)
          if char.player and char in guild.canidates:
            guild.removecanidate(char)
            sendMessage(char, tr("You have been promoted to be a full member of your guild by %s.") % player.name)
            for member in guild.members:
              sendMessage(member, tr("The canidate %s has been promoted to full member status by %s.") % (char.name, player.name))
            guild.addmember(char)

            # Clean voting and or permissions
            char.deltag('vote_' + str(guild.serial))
            char.deltag('permissions_' + str(guild.serial))

            # Make sure our member has the neccesary event
            char.addscript( 'guilds.member' )
            char.resendtooltip()
            if guild.guildstone:
              guild.guildstone.resendtooltip()
          else:
            player.socket.sysmessage(tr('The selected player is not a canidate of this guild.'))
        else:
          player.socket.sysmessage(tr('You are not allowed to promote canidates in this guild.'))

      # Show a target for inviting someone into the guild
      elif switch == 1:
        if not checkPermission(player, guild, PERMISSION_RECRUIT):
          player.socket.sysmessage(tr('You are not allowed to recruit canidates for this guild.'))
        else:
          player.socket.sysmessage('Choose the player you want to recruit for your guild.')
          player.socket.attachtarget('guilds.stone.recruitResponse', [guild.serial])
          return

    # No switch has been selected
    else:
      mainMenu(player, guild)
      return

    guildCanidates(player, guild)

def guildCanidates(player, guild):
  if checkPermission(player, guild, PERMISSION_RECRUIT|PERMISSION_EXPEL|PERMISSION_PROMOTE):
    name = guild.name
    abbreviation = ''
    if len(guild.abbreviation) > 0:
      abbreviation = tr(' [%s]') % guild.abbreviation

    dialog = wolfpack.gumps.cGump()
    dialog.setCallback("guilds.stone.guildCanidatesResponse")
    dialog.setArgs([guild.serial])

    dialog.startPage(0)
    dialog.addResizeGump(64, 34, 9260, 464, 462)
    dialog.addGump(30, 33, 10421, 0)
    dialog.addGump(59, 22, 10420, 0)
    dialog.addGump(43, 173, 10422, 0)
    dialog.addResizeGump(100, 79, 9200, 405, 65)
    dialog.addTilePic(106, 49, 3805)
    dialog.addText(160, 90, tr("Guildstone for %s%s") % (name, abbreviation), 380)
    dialog.addText(160, 113, tr("Canidate Management"), 2100)
    dialog.addResizeGump(100, 149, 9200, 404, 240)
    dialog.addGump(13, 339, 10402, 0)
    dialog.addButton(102, 450, 247, 248, 1)
    dialog.addButton(177, 450, 242, 241, 0)
    dialog.addResizeGump(100, 394, 9200, 404, 46)
    dialog.startGroup(0)

    if checkPermission(player, guild, PERMISSION_RECRUIT):
      dialog.addText(152, 406, tr("Recruit a new canidate for this guild."), 2100)
      dialog.addRadioButton(114, 401, 9721, 9724, 1, 0)
    else:
      dialog.addText(152, 406, tr("Recruit a new member for this guild."), 980)
      dialog.addGump(114, 401, 9721, 980)

    canidates = guild.canidates
    offset = 0
    page = 0
    pages = ceil(len(canidates) / 3.0)

    for canidate in canidates:
      if offset == 0:
        page += 1
        dialog.startPage(page)

        if page > 1:
          dialog.addPageButton(111, 358, 9909, 9911, page - 1)
          dialog.addText(138, 358, tr("Previous Page"), 905)

        if page < pages:
          dialog.addPageButton(469, 358, 9903, 9905, page + 1)
          dialog.addText(388, 358, tr("Next Page"), 905)

      dialog.addResizeGump(110, 157 + offset, 9350, 387, 62)
      dialog.addText(118, 162 + offset, canidate.name, 2100)

      if checkPermission(player, guild, PERMISSION_EXPEL):
        dialog.addText(151, 189 + offset, tr("Expel this canidate"), 2100)
        dialog.addRadioButton(114, 185 + offset, 9721, 9724, 0x80000000|canidate.serial, 0)
      else:
        dialog.addText(151, 189 + offset, tr("Expel this canidate"), 980)
        dialog.addGump(114, 185 + offset, 9721, 980)

      if checkPermission(player, guild, PERMISSION_PROMOTE):
        dialog.addText(338, 189 + offset, tr("Accept this canidate"), 2100)
        dialog.addRadioButton(301, 185 + offset, 9721, 9724, 0x40000000|canidate.serial, 0)
      else:
        dialog.addText(338, 189 + offset, tr("Accept this canidate"), 980)
        dialog.addGump(301, 185 + offset, 9721, 980)

      if offset == 134:
        offset = 0
      else:
        offset += 67

    dialog.send(player)

  else:
    guildCanidatesOverview(player, guild)

def guildDiplomacy(player, guild):
  pass

def guildPropertiesResponse(player, arguments, response):
  guild = wolfpack.findguild(arguments[0])

  if not guild:
    player.socket.sysmessage(tr('The guild you try to manage has already been disbanded.'))
    return

  # Save changes if button 1 was pressed
  if response.button == 1:
      readonly = not checkPermission(player, guild, PERMISSION_PROPERTIES)

      if not readonly:
        messages = []

        if response.text[1] != guild.name:
          guild.name = response.text[1]
          messages.append(tr("The name of your guild has been changed to '%s' by %s.") % (guild.name, player.name))

        abbreviation = response.text[2][:3]
        if abbreviation != guild.abbreviation:
          guild.abbreviation = abbreviation
          messages.append(tr("The abbreviation of your guild has been changed to [%s] by %s.") % (abbreviation, player.name))

        if 0 in response.switches and guild.alignment != 0:
          guild.alignment = 0
          messages.append(tr("The alignment of your guild has been changed to neutral by %s.") % player.name)
        elif 1 in response.switches and guild.alignment != 1:
          guild.alignment = 1
          messages.append(tr("The alignment of your guild has been changed to good by %s.") % player.name)
        elif 2 in response.switches and guild.alignment != 2:
          guild.alignment = 2
          messages.append(tr("The alignment of your guild has been changed to evil by %s.") % player.name)

        if response.text[3] != guild.website:
          guild.website = response.text[3]
          messages.append(tr("The website of your guild has been changed to %s by %s.") % (guild.website, player.name))

        if response.text[4] != guild.charta:
          guild.charta = response.text[4]
          messages.append(tr("The charta of your guild has been changed by %s. Check your guildstone for details.") % player.name)

        for member in guild.members:
          if member.socket:
            member.resendtooltip()

          for message in messages:
            sendMessage(member, message)

        if guild.guildstone:
          guild.guildstone.resendtooltip()

  mainMenu(player, guild)

def guildProperties(player, guild):
  name = guild.name
  abbreviation = ''
  if len(guild.abbreviation) > 0:
    abbreviation = tr(' [%s]') % guild.abbreviation

  readonly = not checkPermission(player, guild, PERMISSION_PROPERTIES)

  dialog = wolfpack.gumps.cGump()
  #dialog.setType(YOUR_TYPE_HERE)
  dialog.setCallback("guilds.stone.guildPropertiesResponse")
  dialog.setArgs([guild.serial])

  dialog.startPage(0)
  dialog.addResizeGump(64, 34, 9260, 464, 462)
  dialog.addGump(30, 33, 10421, 0)
  dialog.addGump(59, 22, 10420, 0)
  dialog.addGump(43, 173, 10422, 0)
  dialog.addResizeGump(100, 79, 9200, 405, 65)
  dialog.addTilePic(106, 49, 3805)
  dialog.addText(160, 90, tr("Guildstone for %s%s") % (name, abbreviation), 380)
  dialog.addText(160, 113, tr("General Properties"), 2100)
  dialog.addResizeGump(100, 149, 9200, 404, 283)
  dialog.addGump(13, 339, 10402, 0)

  dialog.addButton(102, 450, 247, 248, 1)
  dialog.addButton(177, 450, 242, 241, 0)

  if not readonly:
    dialog.addText(120, 164, tr("Guildname"), 2100)
    dialog.addResizeGump(120, 188, 9300, 174, 27)
    dialog.addInputField(124, 193, 160, 16, 2100, 1, guild.name)

    dialog.addText(312, 164, tr("Abbreviation (3 chars max.)"), 2100)
    dialog.addResizeGump(312, 188, 9300, 72, 27)
    dialog.addInputField(318, 192, 58, 17, 2100, 2, guild.abbreviation)
    offset = 0
  else:
    offset = -68

  dialog.addText(120, 232 + offset, tr("Alignment"), 2100)
  if not readonly:
    dialog.startGroup(1)
    dialog.addText(160, 261, tr("Good"), 2100)
    dialog.addText(286, 261, tr("Neutral"), 2100)
    dialog.addText(420, 261, tr("Evil"), 2100)
    dialog.addRadioButton(124, 256, 9721, 9724, 1, guild.alignment == 1)
    dialog.addRadioButton(248, 256, 9721, 9724, 0, guild.alignment == 0)
    dialog.addRadioButton(384, 256, 9721, 9724, 2, guild.alignment == 2)

    dialog.addText(120, 303, tr("Website"), 2100)
    dialog.addResizeGump(119, 322, 9300, 250, 27)
    dialog.addText(120, 363, tr("Charta (HTML allowed)"), 2100)
    dialog.addResizeGump(118, 386, 9300, 360, 27)
    dialog.addInputField(124, 327, 237, 17, 2100, 3, guild.website)
    dialog.addInputField(122, 390, 349, 18, 2100, 4, guild.charta)
  else:
    if guild.alignment == 1:
      dialog.addText(160, 261 + offset, tr("Good"), 2100)
      dialog.addGump(124, 256 + offset, 9724)
    else:
      dialog.addText(160, 261 + offset, tr("Good"), 980)
      dialog.addGump(124, 256 + offset, 9721, 980)

    if guild.alignment == 0:
      dialog.addText(286, 261 + offset, tr("Neutral"), 2100)
      dialog.addGump(248, 256 + offset, 9724)
    else:
      dialog.addText(286, 261 + offset, tr("Neutral"), 980)
      dialog.addGump(248, 256 + offset, 9721, 980)

    if guild.alignment == 2:
      dialog.addText(420, 261 + offset, tr("Evil"), 2100)
      dialog.addGump(384, 256 + offset, 9724)
    else:
      dialog.addText(420, 261 + offset, tr("Evil"), 980)
      dialog.addGump(384, 256 + offset, 9721, 980)

    # Add a HTML gump with additional information
    html = ''
    if len(guild.website) != 0:
      html += tr('<basefont color="#666666"><h3><u>Website:</u></h3><basefont color="#333333"><br><a href="%s">%s</a><br><br>') % (guild.website, guild.website)
    else:
      html += tr('<basefont color="#666666"><h3><u>Website:</u></h3><basefont color="#333333"><br>This guild has no website.<br><br>')

    if len(guild.charta) != 0:
      html += tr('<basefont color="#666666"><h3><u>Charta:</u></h3><basefont color="#333333"><br>%s') % (guild.charta)
    else:
      html += tr('<basefont color="#666666"><h3><u>Charta:</u></h3><basefont color="#333333"><br>This guild has not defined a charta yet.')

    dialog.addText(120, 303 + offset, tr("Additional Information"), 2100)
    dialog.addHtmlGump(120, 323 + offset, 350, 165, html, 1, 1)

  dialog.send(player)

def guildMembershipResponse(player, arguments, response):
  guild = wolfpack.findguild(arguments[0])

  if not guild:
    player.socket.sysmessage(tr('The guild you try to manage has already been disbanded.'))
    return

  if response.button == 1 and player in guild.members:
    # Toggle Guildsign
    if 1 in response.switches:
      if player in guild.members:
        guild.setmemberinfo(player, {'showsign': 0})
        player.socket.sysmessage(tr('You decide not to show your guildsign any longer.'))
        player.resendtooltip()

    elif 2 in response.switches:
      if player in guild.members:
        guild.setmemberinfo(player, {'showsign': 1})
        player.socket.sysmessage(tr('You decide to show your guildsign from now on.'))
        player.resendtooltip()

    # Resign from the guild
    elif 3 in response.switches:
      members = guild.members
      canidates = guild.canidates
      if player in members or player in canidates:
        guild.removemember(player)

        for member in members:
          if member != player:
            sendMessage(member, tr("%s has left your guild.") % player.name)

        player.socket.sysmessage(tr('You have resigned from your guild.'))
        player.resendtooltip()
        if guild.guildstone:
          guild.guildstone.resendtooltip()

    # Resign from leadership
    elif 4 in response.switches:
      if player == guild.leader:
        members = guild.members
        guild.leader = None
        player.socket.sysmessage(tr('You have resigned from your position as the leader of this guild.'))
        player.resendtooltip()
        for member in members:
          if member != player:
            sendMessage(member, tr("%s has resigned from the leadership position of your guild. You can elect a new leader by voting for a member at your guildstone.") % player.name)
        if guild.guildstone:
          guild.guildstone.resendtooltip()

  mainMenu(player, guild)

def guildMembership(player, guild):
  name = guild.name
  abbreviation = ''
  if len(guild.abbreviation) > 0:
    abbreviation = tr(' [%s]') % guild.abbreviation

  dialog = wolfpack.gumps.cGump()
  dialog.setCallback("guilds.stone.guildMembershipResponse")
  dialog.setArgs([guild.serial])

  dialog.startPage(0)
  dialog.addResizeGump(64, 34, 9260, 464, 462)
  dialog.addGump(30, 33, 10421, 0)
  dialog.addGump(59, 22, 10420, 0)
  dialog.addGump(43, 173, 10422, 0)
  dialog.addResizeGump(100, 79, 9200, 405, 65)
  dialog.addTilePic(106, 49, 3805)
  dialog.addText(160, 90, tr("Guildstone for %s%s") % (name, abbreviation), 380)
  dialog.addText(160, 113, tr("Membership Options"), 2100)
  dialog.addResizeGump(100, 149, 9200, 404, 283)
  dialog.addGump(13, 339, 10402, 0)
  dialog.addButton(102, 450, 247, 248, 1)
  dialog.addButton(177, 450, 242, 241, 0)
  dialog.startGroup(0)

  if player in guild.members:
    info = guild.getmemberinfo(player)

    if info['showsign']:
      dialog.addText(156, 172, tr("Disable your guildsign."), 2100)
      dialog.addRadioButton(116, 168, 9721, 9724, 1, 0)
    else:
      dialog.addText(156, 172, tr("Enable your guildsign."), 2100)
      dialog.addRadioButton(116, 168, 9721, 9724, 2, 0)
  else:
    dialog.addText(156, 172, tr("Enable your guildsign."), 980)
    dialog.addGump(116, 168, 9721, 980)

  if player in guild.members or player in guild.canidates:
    dialog.addText(156, 212, tr("Resign from this guild."), 2100)
    dialog.addRadioButton(116, 208, 9721, 9724, 3, 0)
  else:
    dialog.addText(156, 212, tr("Resign from this guild."), 980)
    dialog.addGump(116, 208, 9721, 980)

  if guild.leader == player:
    dialog.addText(156, 252, tr("Resign from your leadership position."), 2100)
    dialog.addRadioButton(116, 248, 9721, 9724, 4, 0)
  else:
    dialog.addText(156, 252, tr("Resign from your leadership position."), 980)
    dialog.addGump(116, 248, 9721, 980)

  dialog.send(player)

def mainMenuResponse(player, arguments, response):
  if response.button != 0:
    if len(arguments) == 0:
      return

    guild = wolfpack.findguild(arguments[0])

    if not guild:
      player.socket.sysmessage(tr('The guild you try to manage has already been disbanded.'))
      return

    # Leave the guild
    if MAINMENU_MEMBERSHIP in response.switches:
      guildMembership(player, guild)

    # Disband the guild
    elif MAINMENU_DISBANDGUILD in response.switches:
      if player != guild.leader and not player.gm:
        player.socket.sysmessage(tr('You need to be the leader of this guild or a gamemaster to disband it.'))
      else:
        members = guild.members
        canidates = guild.canidates

        if guild.guildstone:
          guild.guildstone.delete()
        guild.delete()

        for char in members + canidates:
          sendMessage(char, tr("Your guild has been disbanded by %s!") % player.name)
          if char.socket:
            char.resendtooltip()

    # Show a page with member information
    elif MAINMENU_MEMBERS in response.switches:
      guildMembers(player, guild)

    # Show a page with canidate information
    elif MAINMENU_CANIDATES in response.switches:
      guildCanidates(player, guild)

    # Show a page with ally/enemy information:
    elif MAINMENU_DIPLOMACY in response.switches:
      guildDiplomacy(player, guild)

    # Show a page with the basic properties of this guild
    elif MAINMENU_GENERALPROPERTIES in response.switches:
      guildProperties(player, guild)

    return

def mainMenu(player, guild):
  name = guild.name
  abbreviation = ''
  if len(guild.abbreviation) > 0:
    abbreviation = tr(' [%s]') % guild.abbreviation

  members = guild.members
  canidates = guild.canidates

  dialog = wolfpack.gumps.cGump()
  dialog.setCallback("guilds.stone.mainMenuResponse")
  dialog.setArgs([guild.serial])
  #dialog.setType(YOUR_TYPE_HERE)

  dialog.startPage(0)
  dialog.addResizeGump(64, 34, 9260, 464, 462)
  dialog.addGump(30, 33, 10421, 0)
  dialog.addGump(59, 22, 10420, 0)
  dialog.addGump(43, 173, 10422, 0)
  dialog.addResizeGump(100, 79, 9200, 405, 65)
  dialog.addTilePic(106, 49, 3805)
  dialog.addText(160, 90, tr("Guildstone for %s%s") % (name, abbreviation), 380)
  dialog.addText(160, 113, tr("Main Menu"), 2100)
  dialog.addResizeGump(100, 149, 9200, 404, 283)
  dialog.addGump(13, 339, 10402, 0)
  dialog.addButton(102, 450, 247, 248, 1)
  dialog.addButton(177, 450, 242, 241, 0)
  dialog.startGroup(0)

  if checkPermission(player, guild, PERMISSION_PROPERTIES):
    dialog.addText(156, 172, tr("Modify the general properties of this guild."), 2100)
    dialog.addRadioButton(116, 168, 9721, 9724, MAINMENU_GENERALPROPERTIES, 0)
  else:
    dialog.addText(156, 172, tr("View information about this guild."), 2100)
    dialog.addRadioButton(116, 168, 9721, 9724, MAINMENU_GENERALPROPERTIES, 0)

  if checkPermission(player, guild, PERMISSION_KICK):
    dialog.addText(156, 212, tr("Manage the members of this guild."), 2100)
    dialog.addRadioButton(116, 208, 9721, 9724, MAINMENU_MEMBERS, 0)
  else:
    dialog.addText(156, 212, tr("View the members of this guild."), 2100)
    dialog.addRadioButton(116, 208, 9721, 9724, MAINMENU_MEMBERS, 0)

  if checkPermission(player, guild, PERMISSION_RECRUIT|PERMISSION_EXPEL|PERMISSION_PROMOTE):
    dialog.addText(156, 252, tr("Manage the canidates of this guild."), 2100)
    dialog.addRadioButton(116, 248, 9721, 9724, MAINMENU_CANIDATES, 0)
  else:
    dialog.addText(156, 252, tr("View the canidates of this guild."), 2100)
    dialog.addRadioButton(116, 248, 9721, 9724, MAINMENU_CANIDATES, 0)

  # Diplomatic relations are not done yet
  dialog.addText(156, 292, tr("View the diplomatic relations of this guild."), 980)
  dialog.addGump(116, 288, 9721, 980)

  #if checkPermission(player, guild, PERMISSION_DIPLOMACY):
  #  dialog.addText(156, 292, "Manage the diplomatic relations of this guild.", 2100)
  #  dialog.addRadioButton(116, 288, 9721, 9724, MAINMENU_DIPLOMACY, 0)
  #else:
  #  dialog.addText(156, 292, "View the diplomatic relations of this guild.", 2100)
  #  dialog.addRadioButton(116, 288, 9721, 9724, MAINMENU_DIPLOMACY, 0)

  # You can only resign from a guild you are a member of
  if player in members or player in canidates:
    dialog.addText(156, 332, tr("Membership options."), 2100)
    dialog.addRadioButton(116, 328, 9721, 9724, MAINMENU_MEMBERSHIP, 0)
  else:
    dialog.addText(156, 332, tr("Membership options."), 980)
    dialog.addGump(116, 328, 9721, 980)

  # Only the leader or a gamemaster may disband the guild
  if player.gm or guild.leader == player:
    dialog.addText(156, 372, tr("Disband this guild."), 2100)
    dialog.addRadioButton(116, 368, 9721, 9724, MAINMENU_DISBANDGUILD, 0)
  else:
    dialog.addText(156, 372, tr("Disband this guild."), 980)
    dialog.addGump(116, 368, 9721, 980)

  dialog.send(player)
  return 1

def onUse(player, stone):
  guild = None
  if stone.hastag('guild'):
    guild = wolfpack.findguild(int(stone.gettag('guild')))

  if not guild:
    player.socket.sysmessage(tr('This guildstone is abandoned.'))
  else:
    if not player.cansee(stone) or not player.canreach(stone, 3):
      player.socket.clilocmessage(501661)
    else:
      mainMenu(player, guild)
  return 1

# Show a nice tooltip
def onShowTooltip(sender, target, tooltip):
  tooltip.add(1042971, "#1041429")

  guild = None
  if target.hastag('guild'):
    guild = wolfpack.findguild(int(target.gettag('guild')))

  if guild:
    if guild.guildstone != target:
      tooltip.add(1060847, tr("%s\t\n") % guild.name)
    else:
      if len(guild.abbreviation) == 0:
        appendix = tr("%s") % guild.name
      else:
        appendix = tr("%s [%s]") % (guild.name, guild.abbreviation)

      appendix += "\n" + tr("Members: %u") % len(guild.members)
      appendix += "\n" + tr("Canidates: %u") % len(guild.canidates)

      if guild.leader:
        appendix += "\n" + tr("Leader: %s") % guild.leader.name
      else:
        appendix += "\n" + tr("Leader: None")
        
      tooltip.add(1060847, "%s\t\n" % appendix)
  else:
    tooltip.add(1060847, tr("Abandoned") + "\t ")

# The guildstone is being deleted
def onDelete(item):
  pass
