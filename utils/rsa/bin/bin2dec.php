<?php
function bin($c,$sz)
{
	$b="0";
	for ($i=0;$i<$sz;$i++)
	{
		$b=bcadd(bcmul($b,"256"),ord($c{$i}));
	}
	return $b;
}
function binfile($f)
{
	return bin(file_get_contents($f),128);
}
echo "E: ".binfile('rsakey.e').chr(10);
echo "D: ".binfile('rsakey.d').chr(10);
echo "N: ".binfile('rsakey.n').chr(10);
$f=file_get_contents($argv[1]);
echo "H: ".bin(substr($f,16,64),64).chr(10);
echo "S: ".bin(substr($f,80,176),176).chr(10);
?>
