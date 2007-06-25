
use XML::DOM;
use Data::Dumper;
use strict;

my %items; # Item Definition Storage

sub deep_copy
{
	my $this = shift;
	
	if( not ref $this )
	{
  		$this;
	}
	elsif( ref $this eq "ARRAY" )
	{
  		[ map deep_copy( $_ ), @$this ];
	}
	elsif( ref $this eq "HASH" )
	{
  		+{ map { $_ => deep_copy( $this->{ $_ } ) } keys %$this };
	}
	else
	{
		die "what type is $_?"
	}
}

sub parseFile
{	
	my $basepath = shift;
	my $filename = shift;
	
	print "Loading $filename...\n";
	
	my $parser = new XML::DOM::Parser;
	my $doc = $parser->parsefile( $basepath . $filename );
	
	my $definitions = ( $doc->getElementsByTagName( 'definitions', 0 ) )[0];
	
 	for( my $node = $definitions->getFirstChild(); defined( $node ); $node = $node->getNextSibling() )
 	{
 		my $name = $node->getNodeName();
 		 		
 		# Include another file
 		if( $name eq 'include' )
 		{
 			my $file = $node->getAttribute( 'file' );
 			parseFile( $basepath, $file );
 		}
 		elsif( $name eq 'item' )
 		{
 			my $id = $node->getAttribute( 'id' );

			$items{ $id } = $node->cloneNode( 1 );
 		}
 	}
 	
	$doc->dispose();
}

parseFile( 'D:/Wolfpack/', 'definitions.xml' );

my %categories;
my $lastid = 1;

sub resolveSubNode
{
	my $item = shift;
	my $nodename = shift;
	
	my @subnodes = $item->getElementsByTagName( $nodename, 0 );
	
	if( $#subnodes >= 0 )
	{
		# Return the last one
		return $subnodes[-1];
	}
	
	# Search and resolve inherit tags
	my @inherit = $item->getElementsByTagName( 'inherit', 0 );
	
	foreach my $node ( @inherit )
	{
		if( !defined( $node->getAttribute( $nodename ) ) )
		{
			next;
		}
		
		my $inheritfrom = $items{ $node->getAttribute( 'id' ) };
		my $subnode = resolveSubNode( $inheritfrom, $nodename );
		
		if( defined( $subnode ) )
		{
			return $subnode;
		}
	}
	
	return undef;
}

foreach my $key( keys %items )
{
	my $node = $items{ $key };

	my $category = ( $node->getElementsByTagName( 'category', 0 ) )[0];

	if( !defined( $category ) )
	{
		next;
	}

	$category = $category->getFirstChild()->toString() . "\n";	
	
	if( !exists $categories{ $category } )
	{
		$categories{ $category } = $lastid;
		$lastid++;
	}
	
	# Next: Display Id
	my $dispidnode = resolveSubNode( $node, 'id' );
	
	if( !defined( $dispidnode ) )
	{
		print "Missing Display Tag for Item " . $key . "\n";
		next;
	}
	
	my $dispid = $dispidnode->getFirstChild()->toString();
	
	my $color = 0;
	
	my $colornode = resolveSubNode( $node, 'color' );
	
	if( defined( $colornode ) )
	{
		$color = $colornode->getFirstChild()->toString();
	}
	
	my $name = 'NULL';
	
	my $namenode = resolveSubNode( $node, 'name' );
	
	if( defined( $namenode ) )
	{
		$name = "'" . $namenode->getFirstChild()->toString() . "'";
	}
		
	if( lc( substr( $dispid, 0, 2 ) ) eq '0x' )
	{
		$dispid = hex( substr( $dispid, 2, length( $dispid ) - 2 ) );
	}
	
	if( lc( substr( $color, 0, 2 ) ) eq '0x' )
	{
		$color = hex( substr( $color, 2, length( $color ) - 2 ) );
	}
	
	my $sql = sprintf( "INSERT INTO items VALUES(NULL,%s,%u,%u,%u,'%s');", $name, $categories{ $category }, int( $dispid ), int( $color ), $key );
	print $sql."\n";
}
