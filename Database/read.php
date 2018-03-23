<?php
$sql = "SELECT * FROM hvac ORDER BY id DESC LIMIT 1";

$connect = mysqli_connect("localhost", "username", "password", "database");

$result = mysqli_query($connect, $sql);

$response = array();

while($row = mysqli_fetch_array($result)){
	array_push($response, array("id"=>$row[0], "email"=>$row[1], "moisture"=>$row[2], "temp"=>$row[3], "sound"=>$row[4], "timestamp"=>$row[5]));
}

echo json_encode(array("hvac_response"=>$response));
?>