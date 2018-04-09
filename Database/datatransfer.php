<?php
$connect = new mysqli("localhost", "username", "password", "databasename");
if ($conn->connect_error) {
    die("Connection failed: " . $conn->connect_error);
} 

$sql = "INSERT INTO tablename (email, moisture, temp, sound) VALUES ('".$_GET['email']."', '".$_GET['moisture']."', '".$_GET['temp']."', '".$_GET['sound']."')";
echo $sql;

if ($connect->query($sql) === TRUE) {
    echo "New record created successfully";
} else {
    echo "Error: " . $sql . "<br>" . $conn->error;
}

$connect->close();
?>