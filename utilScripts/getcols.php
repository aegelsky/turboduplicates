<?php
error_reporting(E_ALL);

function exception_error_handler($errno, $errstr, $errfile, $errline ) {
    throw new ErrorException($errstr, 0, $errno, $errfile, $errline);
}
set_error_handler("exception_error_handler");

$file = file_get_contents('cols.html');

$b = preg_match_all('#<a href=[^>]+acm=coll[^i]+id=([\d]+?)[^>]+>(.*)</a>#imU', $file, $m);

// <a href="http://wrappers.ru/?act=coll&amp;acm=coll&amp;id=167"><b>Turbo Sport 2003 turkish</b></a>
// <a href="http://wrappers.ru/?act=coll&amp;acm=coll&amp;id=90" target="_blank">Turbo ATAKA</a>

$ids_got = array();

foreach ($m[1] as $key => $id) {
    if (!in_array($id, $ids_got)) {
        $imagesHtml = file_get_contents('http://wrappers.ru/modules/toprint.php?coll='.$id);
        preg_match_all('#<img src="([^"]+)"#', $imagesHtml, $images);
        // <img src="http://wrappers.ru/collections/2010/pct_182401.jpg" height="250">
        $dirname = 'cols/'.str_replace('/', '-', strip_tags($m[2][$key]));

        if (!is_dir($dirname)) {
            mkdir($dirname);
        }

        $files = glob($dirname . "/*");
        if (count($images[1]) == count($files)) {
            echo "Dir $dirname already downloaded, skipping...\n";
            continue;
        } else {
            echo "!!! Refreshing dir $dirname !!!\n";
        }

        $i = 1;
        foreach ($images[1] as $imgUrl) {
            file_put_contents($dirname.'/'.$i++.'.jpg', file_get_contents($imgUrl)); // basename($imgUrl)
        }
        $ids_got[] = $id; // save ID as it can be duplicates here
    }
}

echo 'asdasdsad';



