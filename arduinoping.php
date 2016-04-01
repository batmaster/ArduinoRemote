
<?php

header("content-type:text/javascript;charset=utf-8");
$con = mysql_connect('localhost:3306','root','rootaemysql')or die(mysql_error());
mysql_select_db('arduino')or die(mysql_error());
mysql_query("SET NAMES UTF8");


// board ping
if (isset($_GET["bid"])) {
    $bid = $_GET["bid"];
    $ip = $_SERVER['REMOTE_ADDR'];
    $port = $_GET["port"];

    $sql = "INSERT INTO ping (bid, ip, port, date) VALUES ('$bid', '$ip', '$port', NOW()) ON DUPLICATE KEY UPDATE ip = '$ip', port = '$port', date = NOW()";
    mysql_query($sql);
    echo $ip;
}
// get all lost boards
else if (isset($_GET["lost5min"])) {
    $sql = "SELECT * FROM ping WHERE date < NOW() - INTERVAL 5 MINUTE";
    $res = mysql_query($sql);
    $rows = array();
    if (mysql_num_rows($res) != 0) {
        while ($r = mysql_fetch_array ($res)) {
            $rows[] = $r;
        }
    }

    echo json_encode($rows);
    mysql_close();

}
// otherwise, get 10 ping logs if `sql` not provided
else {
    $sql = "SELECT * FROM ping ORDER BY id DESC LIMIT 10";
    if (isset($_POST["sql"]))
    if ($_POST["sql"] == "") { }
    else {
        $sql = $_POST["sql"];
        $sqll = $_POST["sql"];
        $sql = str_replace("xxaxx", "'", $sql);
        $sql = str_replace("xxbxx", "(", $sql);
        $sql = str_replace("xxcxx", ")", $sql);
        $sql = str_replace("xxdxx", ">", $sql);
        $sql = str_replace("\\", "", $sql);
    }

    $res = mysql_query($sql);
    $rows = array();
    if (mysql_num_rows($res) != 0) {
        while ($r = mysql_fetch_array ($res)) {
            $rows[] = $r;
        }
    }

    echo json_encode($rows);
    mysql_close();
}
?>
