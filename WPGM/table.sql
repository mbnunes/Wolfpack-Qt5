
CREATE TABLE categories (
	id INTEGER PRIMARY KEY,
	name varchar(255) NULL,
	parent int NOT NULL,
	type int
);

CREATE TABLE items (
	id INTEGER PRIMARY KEY,
	name varchar(255) NULL,
	parent int NOT NULL,
	artid int,
	color int,
	addid varchar(255)
);

CREATE TABLE locationcategories (
	id INTEGER PRIMARY KEY,
	name varchar(255) NULL,
	parent int NOT NULL,
	type int
);

CREATE TABLE locations (
	id INTEGER PRIMARY KEY,
	name varchar(255) NULL,
	parent INT NOT NULL,
	posx INT NOT NULL,
	posy INT NOT NULL,
	posz INT NOT NULL,
	posmap INT NOT NULL,	
	location varchar(255)
);