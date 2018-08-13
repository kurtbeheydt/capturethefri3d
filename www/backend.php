<?php
$filename = 'data.json';

$data = (isset($_GET['data'])) ? json_decode($_GET['data']) : false;

if (!$data) {
    die('no data');
}

//print_r($data);

$handle = fopen($filename, 'r') or die('Cannot open file: ' . $filename);
$currentdata = json_decode(fread($handle, filesize($filename)));
fclose($handle);

$handle = fopen($filename, 'w') or die('Cannot open file: ' . $filename);

$currentdata->towers->{$data->name} = $data;

if ($data->currentLeadingTeam != 'niemand') {
    $currentdata->scores->{$data->currentLeadingTeam}++;
}

fwrite($handle, json_encode($currentdata));
fclose($handle);

echo "ok";
?>