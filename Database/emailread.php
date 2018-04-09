<?php
include('top-cache.php');
$sql = "SELECT * FROM tablename ORDER BY id DESC LIMIT 1";

$connect = mysqli_connect("localhost", "username", "password", "databasename");

$result = mysqli_query($connect, $sql);

$response = array();

while($row = mysqli_fetch_array($result)){
	array_push($response, array("id"=>$row[0], "email"=>$row[1], "timestamp"=>$row[5]));
}

echo json_encode(array("tablename_response"=>$response));
include('bottom-cache.php');
?>