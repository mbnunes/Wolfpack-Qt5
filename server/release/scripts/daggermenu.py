import wolfpack

def onContextEntry( char, item, tag  ):
    char.message( str( tag )  )
    return 1
