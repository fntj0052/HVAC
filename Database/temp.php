<?php
include('top-cache.php');
$sql = "SELECT * FROM hvac ORDER BY id DESC LIMIT 1";

$connect = mysqli_connect("localhost", "justlik6_hvacusr", "8XtY@$7C", "justlik6_hvac");

$result = mysqli_query($connect, $sql);

$response = array();

while($row = mysqli_fetch_array($result)){
	array_push($response, array("id"=>$row[0], "temp"=>$row[3], "timestamp"=>$row[5]));
}

echo json_encode(array("hvac_response"=>$response));
include('bottom-cache.php');
?>