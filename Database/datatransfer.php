<?php
$connect = new mysqli("localhost", "justlik6_hvacusr", "8XtY@$7C", "justlik6_hvac");
if ($conn->connect_error) {
    die("Connection failed: " . $conn->connect_error);
} 

$sql = "INSERT INTO hvac (email, moisture, temp, sound) VALUES ('".$_GET['email']."', '".$_GET['moisture']."', '".$_GET['temp']."', '".$_GET['sound']."')";
echo $sql;

if ($connect->query($sql) === TRUE) {
    echo "New record created successfully";
} else {
    echo "Error: " . $sql . "<br>" . $conn->error;
}

$connect->close();
?>