from xml.dom.minidom import *

import sys
sys.path[:0] = ['../python-lib/']
import Pmw
import Tkinter

class Option:
	def __init__(self,name,defvalue, choices):
		self.text = name
		self.value = defvalue
		self.choices = choices

class ConfigGroup:
	def __init__(self, parent, y, text, options):
		self.entries = []
		self.text = text
		self.gr = Pmw.Group( parent.interior(),
		tag_text = text,
		tag_font = Pmw.logicalfont('Helvetica', 4),
		)
		self.y = y
		self.height = 20
		for op in options:
			if len( op.choices ) > 0:
				entry = Pmw.ComboBox(self.gr.interior(),
				labelpos = 'w',
				label_text = op.text,
				listheight = len(op.choices)*40,
				selectioncommand = self.onSelect,
				scrolledlist_items = op.choices,
				)
				entry.selectitem(str(op.value))
				entry.pack(fill='both',expand=1,pady=5,padx=5)
				self.height += 40
				self.entries.append( entry )
			else:
				entry = Pmw.EntryField(self.gr.interior(),
				labelpos = 'w',
				value = str(op.value),
				label_text = op.text,
				validate = None
				)
				entry.pack(fill='both',expand=1,pady=5,padx=5)
				self.height += 40
				self.entries.append( entry )

		parent.create_window(1, y,width=300,height=self.height,anchor='nw',window = self.gr)

	def onSelect(self,text):
		print text

class XMLConfig:
	def __init__(self, parent):
		self.resutl = ''
		self.parent = parent
		self.groups = []
		self.xmlname = '../wolfpack.xml'
		self.sc = Pmw.ScrolledCanvas(parent,
		borderframe = 1,
		usehullsize = 1,
		hull_width = 400,
		hull_height = 300,
		)

		try:
			wolfconfig = parse('../wolfpack.xml')
			childnodes = wolfconfig.childNodes
		except:
			print 'Could not parse wolfpack.xml!'
			return

		groups = wolfconfig.getElementsByTagName('group')

		opts = []
		y = 1
		for gr in groups:
			options = gr.getElementsByTagName('option')
			for op in options:
				value = op.getAttribute('value')
				if ( value == 'true' or value == 'false' ):
					choices = ('true','false')

				elif( value == 'mysql' or value == 'sqlite' ):
					choices = ('mysql','sqlite')

				else:
					choices = ()

				opts.append( Option( op.getAttribute('key'), value, choices ) )
			cfg = ConfigGroup( self.sc, y, gr.getAttribute('name'), opts )
			self.groups.append( cfg )
			y += cfg.height + 5
			opts = []

		self.sc.pack(padx = 5, pady = 5, fill = 'both', expand = 1)
		self.sc.resizescrollregion()

	def writexml(self):
		self.dialog = Pmw.MessageDialog(self.parent,
			buttons = ('OK','Cancel'),
			message_text = 'Save changes ?',
			defaultbutton = 'OK',
			title = 'My dialog',
			command = self.execute)
		self.dialog.withdraw()
		self.dialog.activate(geometry = 'centerscreenalways')

		if self.result == 'OK':
			self.dialog.deactivate(self.result)
			dom = Document()
			all = dom.createElement("preferences")
			all.setAttribute('version','1.0')
			all.setAttribute('application','Wolfpack')
			for gr in self.groups:
				entries = gr.entries
				groupname = gr.text
				xgr = dom.createElement('group')
				xgr.setAttribute('name',groupname)
				for en in entries:
					labeltext = en['label_text']
					try:
						value = en['value']
					except:
						value = str(en.get())
					xop = dom.createElement('option')
					xop.setAttribute('key',labeltext)
					xop.setAttribute('value',value)
					xgr.appendChild(xop)

				all.appendChild(xgr)

			all.appendChild(xgr)
			dom.appendChild(all)
			file = open(self.xmlname,'w')
			file.write("<!DOCTYPE preferences>\n")
			file.write(dom.toprettyxml())
			file.flush()
			file.close()

	def execute(self, result):
		self.result = result
		self.dialog.deactivate(result)


if __name__ == '__main__':
	root = Tkinter.Tk()
	Pmw.initialise(root)
	root.title('Wolfpack configurator')

	config = XMLConfig(root)
	saveButton = Tkinter.Button(root, text = 'Save changes', command = config.writexml)
	saveButton.pack(pady = 5, padx = 5)
	exitButton = Tkinter.Button(root, text = 'Exit configurator', command = root.destroy)
	exitButton.pack(pady = 5, padx = 5)

	root.mainloop()
