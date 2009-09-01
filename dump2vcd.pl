#!/usr/bin/perl

my @data=();

my @names=();
my @lens=();

my $i;
my $s;

@_=split(/,/,$ARGV[1]);
foreach(@_) {
    ($name, $len) = split(/:/,$_);
    push(@names, $name);
    push(@lens, $len);
}

open(IN,$ARGV[0]);
while(<IN>) {
    chomp;
    push(@data,$_);
}
close(IN);

print "\$timescale 1ns \$end\n";
print "\$scope module logic \$end\n";
$s=0x21;
for($i=0;$i<=$#names;$i++) {
    if ($names[$i] ne "null") {
	print "\$var wire $lens[$i] ";
	print chr($s++);
	print " $names[$i] \$end\n";
    }
}
print "\$upscope \$end\n";
print "\$enddefinitions \$end\n";

my $cnt=0;
while($_ = pop(@data)) {
    #print "$_\n";
    s/\s+//g;
    @_=split(/,/,$_);
    print "\#$cnt\n";
    $cnt++;
    $i=1;
    $s=0x21;
    for($symbol=0;$symbol<=$#names;$symbol++) {
	if ($names[$symbol] ne "null") {
	    print "b" if $lens[$symbol] > 1;
	    for($len=0;$len<$lens[$symbol];$len++) {
		print "$_[$i]";
		$i++;
	    }
	    print " " if $len>1;
	    print chr($s++);
	    print "\n";
	}
	else {
	    $i += $lens[$symbol];
	}
    }
}
