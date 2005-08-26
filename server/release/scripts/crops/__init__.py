
# A list with different grow stages
# baseid = [[1st stage], [2nd stage]..., id of harvested item, amount, [item which will be added after harvested]]
# stages are lists to avoid things looking the same
stages = {
	"wheat_sprouts" : [ [0xdaf], [0xdae], [0xc55, 0xc56], [0xc57, 0xc58], [0xc5a, 0xc5b], "1ebd", 1, [0x1ebe, 0x1ebf]],
	"hay" : [ [0x1a95, 0x1a96], [0x1a93], [0x1a93], [0x1a92], "1ebd", 1, [0x1ebe, 0x1ebf]],
	"cotton" : [ [0xc51, 0xc52], [0xc51, 0xc52], [0xc53, 0xc54], "df9", 1, [0xc51, 0xc52]],
	"hops" : [ [0x1a9f, 0x1aa1], [0x1a9f, 0x1aa1], [0x1a9e, 0x1aa0], "1aa2", 5, [0x1a9f, 0x1aa1]],
	"flax" : [ [0x1a99], [0x1a99], [0x1a9b], [0x1a9b], "1a9c", 3, [0x1a99]]
	}