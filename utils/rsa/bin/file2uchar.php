<?php
$f=file_get_contents($argv[1]);
$l=strlen($f);
echo "unsigned char abFile[$l] = { 0x".substr(chunk_split(bin2hex($f),2,", 0x"),0,-2)."};\n";
?>