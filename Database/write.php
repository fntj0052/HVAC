<?php
$connect = new mysqli("localhost", "username", "password", "database");
if ($conn->connect_error) {
    die("Connection failed: " . $conn->connect_error);
} 

$sql = "INSERT INTO data (email, moisture, temp, sound) VALUES ('".$_POST['email']."', '".$_POST['moisture']."', '".$_POST['temp']."', '".$_POST['sound']."')";
echo $sql;

if ($connect->query($sql) === TRUE) {
    echo "New record created successfully";
} else {
    echo "Error: " . $sql . "<br>" . $conn->error;
}

$connect->close();
?>