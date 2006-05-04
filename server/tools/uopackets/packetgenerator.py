from xml.dom.minidom import parse, parseString
import datetime

def writeCopyright( f ):
    t = datetime.datetime.now()
    f.write("\
/*\n\
 *     Wolfpack Emu (WP)\n\
 * UO Server Emulation Program\n\
 *\n\
 * Copyright 2001-%s by holders identified in AUTHORS.txt\n\
 * This program is free software; you can redistribute it and/or modify\n\
 * it under the terms of the GNU General Public License as published by\n\
 * the Free Software Foundation; either version 2 of the License, or\n\
 * (at your option) any later version.\n\
 *\n\
 * This program is distributed in the hope that it will be useful,\n\
 * but WITHOUT ANY WARRANTY; without even the implied warranty of\n\
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the\n\
 * GNU General Public License for more details.\n\
 *\n\
 * You should have received a copy of the GNU General Public License\n\
 * along with this program; if not, write to the Free Software\n\
 * Foundation, Inc., 59 Temple Palace - Suite 330, Boston, MA 02111-1307, USA.\n\
 *\n\
 * In addition to that license, if you are running this program or modified\n\
 * versions of it on a public system you HAVE TO make the complete source of\n\
 * the version used by you available or provide people with a location to\n\
 * download it.\n\
 *\n\
 * Wolfpack Homepage: http://developer.berlios.de/projects/wolfpack/\n\
 */\n\n" % t.year )
    f.write("/****************************************************************************\n"
            "** Resource object code                                                      \n"
            "**                                                                           \n"
            "** Created: %s                                                               \n"
            "**      by: The Wolfpack Packet Compiler                                     \n"
            "**                                                                           \n"
            "** WARNING! All changes made in this file will be lost!                      \n"
            "*****************************************************************************/ \n\n\n" % t.strftime("%a %d. %b %T %Y") )


class CppType:
    def __init__(self, node):
        datatype = node.getAttribute("datatype")
        self.internalDefinition = None
        if datatype == "byte":
            self.cppType = "uchar"
            self.isEnum = False
            self.isFixedSize = True
        elif datatype == "uint":
            self.cppType = "uint"
            self.isEnum = False
            self.isFixedSize = True
        elif datatype == "string":
            self.cppType = "QByteArray"
            self.isEnum = False
            self.isFixedSize = False
        elif datatype == "enum":
            self.cppType = node.getAttribute("name").title()
            self.isEnum = True
            self.isFixedSize = True
            self.internalDefinition = "\tenum %s {\n" % self.cppType
            for value in node.getElementsByTagName("value"):
                self.internalDefinition += "\t\t%s = %s,\n" % ( value.getAttribute("name"), value.nodeValue )
            self.internalDefinition += "\t\t%sCount\n\t}\n" % self.cppType
                
def getBuiltInByBitSize( num, signed ):
    if num <= 8:
        result = "char"
    elif num <= 16:
        result = "short"
    elif num <= 32:
        result = "int"
    if not signed:
        result = "u" + result
    return result
        

def buildPacket( packetNode, forwardfile, hfile, cppfile ):
    print "Processing packet %s\n" % packetNode.getAttribute("name")

    className = packetNode.getAttribute("name").replace(' ', '')
    print className
    
    # declare forward
    forwardfile.write( "class %s;\n" % className )

    hfile.write("class %s : public cUOPacket\n{\n" % className )
    hfile.write("public:\n")

    constructorImplementation = "%s::%s() : cUOPacket( " % ( className, className )
    privateData = ""
    methodsDeclaration = "// Methods\n"
    methodsDeclaration += "public:\n"
    methodsDeclaration += "\t%s::%s();\n" % ( className, className )
    methodsDeclaration += "\t%s::%s( const QDataStream& );\n\n" % ( className, className )

    methodsImplementation = ""

    for node in packetNode.getElementsByTagName("field"):

        # Build the 2 constructors, for sending and receiving
        fieldName = node.getAttribute("name")
        if fieldName == "id":
            constructorImplementation += str( node.nodeValue )
            if packetNode.getAttribute("size") != None:
                constructorImplementation += ", %s)\n{\n}\n\n" % packetNode.getAttribute("size")
            else:
                constructorImplementation += ")\n{\n}\n\n"
            constructorImplementation += "%s::%s( const QDataStream& ds )\n{\n" % (className, className)
        else:
            cppType = CppType( node )

            # Build the parsing constructor
            if cppType.isFixedSize:
                constructorImplementation += "\tds >> m_%s;\n" % fieldName
            elif cppType.isEnum:
                if node.getAttribute("size") != "32":
                    holdType = getBuiltInByBitSize( int(node.getAttribute("size")), node.getAttribute("signed") )
                    constructorImplementation += "\t%s temp%s = 0;\n" % ( holdType, holdType )
                    constructorImplementation += "\tds >> temp%s;\n" % ( holdType )
                    constructorImplementation += "\tm_%s = (%s)temp%s\;n" % (fieldName, cppType.cppType, holdType )
                else:
                    constructorImplementation += "\tds >> (uint)m_%s" % fieldName
            if cppType.internalDefinition:
                hfile.write(cppType.internalDefinition + "\n")
                
            # Declare Private members
            privateData += "\t%s m_%s;\n" % ( cppType.cppType, fieldName )
            # Declare Getter
            methodsDeclaration += ("\t%s %s() const;\n" % ( cppType.cppType, fieldName ) )
            # Declare Setter
            methodsDeclaration += ("\tvoid set%s( const %s& );\n\n" % ( fieldName.title(), cppType.cppType ) )
            # Define Getter
            methodsImplementation += "%s %s::%s() const\n{\n\treturn m_%s;\n}\n\n" % ( cppType.cppType, className, fieldName, fieldName )
            # Define Setter
            methodsImplementation += "void %s::set%s( const %s& d )\n{\n\tm_%s = d;\n}\n\n" % ( className, fieldName.title(), cppType.cppType, fieldName )

    constructorImplementation += "}\n\n"

    hfile.write( methodsDeclaration )
    hfile.write("\nprivate:\n")
    hfile.write( privateData )            

    hfile.write("};\n\n")

    cppfile.write( constructorImplementation )
    cppfile.write( methodsImplementation )
    

def main():
    forwardfile = file( "uopacketsfwd.h", "w" )
    hfile = file("uopackets.h", "w")
    cppfile = file("uopackets.cpp", "w")
    writeCopyright( forwardfile )
    writeCopyright( cppfile )
    writeCopyright( hfile )

    hfile.write("#if !defined(__UOPACKETS_H__)\n")
    hfile.write("#define __UOPACKETS_H__\n\n\n")
    forwardfile.write("#if !defined(__UOPACKETSFWD_H__)\n")
    forwardfile.write("#define __UOPACKETSFWD_H__\n\n\n")
    
    print "Parsing uoprotocol.xml\n"
    document = parse('uoprotocol.xml')
    for node in document.getElementsByTagName("packet"):
        buildPacket( node, forwardfile, hfile, cppfile )

    hfile.write("#endif // __UOPACKETS_H__\n")
    forwardfile.write("#endif // __UOPACKETSFWD_H__\n")

    forwardfile.close()
    hfile.close()
    cppfile.close()

if __name__ == '__main__':
    main()

