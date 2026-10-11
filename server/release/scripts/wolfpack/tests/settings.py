from wolfpack import *

def runTests():
    result = True
    settings.setbool("testing", "testingbool", True)
    if not settings.getbool("testing", "testingbool", False, True):
        console.send(" - settings.setbool or settings.getbool fail\n")
        result = False
    settings.removekey("testing", "testingbool");
    if settings.getbool("testing", "testingbool", False, False):
        console.send(" - settings.removekey fail\n")
        result = False

    settings.setnumber("testing", "testingnumber", 2594)
    if settings.getnumber("testing", "testingnumber", 4952) != 2594:
        console.send(" - settings.setnumber or settings.getnumber fail\n")
        result = False
    settings.removekey("testing", "testingnumber");

    settings.setstring("testing", "testingstring", "wolfpack is great")
    if settings.getstring("testing", "testingstring", "srpu is great") != "wolfpack is great":
        console.send(" - settings.setstring or settings.getstring fail\n")
        result = False
    settings.removekey("testing", "testingbool");
    return result

def runAll():
    console.sendprogress("Testing wolfpack.settings functions")
    if not runTests():
        console.sendfail()
        return False
    else:
        console.senddone()
        return True
    
    
