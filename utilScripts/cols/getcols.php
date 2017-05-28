<?php
error_reporting(E_ALL);

function exception_error_handler($errno, $errstr, $errfile, $errline ) {
    echo implode(' - ', array($errno, $errstr, $errfile, $errline ))."\n";
//    throw new ErrorException($errstr, 0, $errno, $errfile, $errline);
}
set_error_handler("exception_error_handler");
set_time_limit(10000);

$file = file_get_contents('cols.html');

$b = preg_match_all('#<a href=[^>]+acm=coll[^i]+id=([\d]+?)[^>]+>(.*)</a>#imU', $file, $m);

// <a href="http://wrappers.ru/?act=coll&amp;acm=coll&amp;id=167"><b>Turbo Sport 2003 turkish</b></a>
// <a href="http://wrappers.ru/?act=coll&amp;acm=coll&amp;id=90" target="_blank">Turbo ATAKA</a>

$ids_got = array();
$refresh = isset($_REQUEST['refresh'])? (int)$_REQUEST['refresh'] : 0;

function getRemoteSize($url) {
    $ch = curl_init($url);

    curl_setopt($ch, CURLOPT_RETURNTRANSFER, TRUE);
    curl_setopt($ch, CURLOPT_HEADER, TRUE);
    curl_setopt($ch, CURLOPT_NOBODY, TRUE);

    $data = curl_exec($ch);
    $size = curl_getinfo($ch, CURLINFO_CONTENT_LENGTH_DOWNLOAD);

    curl_close($ch);

//    $head = array_change_key_case(get_headers($url, TRUE));
//    $size = $head['content-length'];

    return $size;
}

function getFileTricky($url) {
    while (!($file = file_get_contents($url))) {
        usleep(250000);// wait 0.25 s on error
    }
    return $file;
}

// ignore @TODO: !!!!
//Белка и Стрелка. Озорная семейка. ТурбоПёс
//Turbo S - Love is... - Pedro Fundy
//Tom &amp; Jerry 4
//Street Fighter II Turbo (2)






foreach ($m[1] as $key => $id) {
    if (!in_array($id, $ids_got)) {
        $imagesHtml = getFileTricky('http://wrappers.ru/modules/toprint.php?coll='.$id);
        preg_match_all('#<img src="([^"]+)"#', $imagesHtml, $images);
        // <img src="http://wrappers.ru/collections/2010/pct_182401.jpg" height="250">
        $dirname = 'cols/'.str_replace('/', '-', strip_tags($m[2][$key]));

        if (!is_dir($dirname)) {
            mkdir($dirname);
        }

        $files = glob($dirname . "/*");
        if (count($images[1]) == count($files) && !$refresh) {
            echo "Dir $dirname already downloaded, skipping...\n";
            continue;
        } else {
            echo "!!! Refreshing dir $dirname !!!\n";
        }

        $i = 1;
        $num = count($images[1]);
        foreach ($images[1] as $imgUrl) {
            echo "  refreshing $i / $num \n";
            $localName = $dirname.'/'.$i++.'.jpg';
            $exists = file_exists($localName);
            if ($refresh || !$exists || (!$refresh && $exists && (($rsize = getRemoteSize($imgUrl)) != filesize($localName)))) {
                file_put_contents($localName, getFileTricky($imgUrl)); // basename($imgUrl)
            }
        }
        $ids_got[] = $id; // save ID as it can be duplicates here
    }
}

echo 'asdasdsad';



