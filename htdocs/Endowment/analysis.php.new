<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.01 Transitional//EN">
<HTML>
  <HEAD>
    <TITLE>Seasonal Forecast</TITLE>
    <META NAME="AUTHOR" CONTENT="Dave Flory">
    <META http-equiv="Content-Type" content="text/html; charset=utf-8">
    <LINK rel="stylesheet" type="text/css" href="./include/index.css">
    <SCRIPT LANGUAGE="JavaScript">

      function displayImage(form){
         var image = form.fdata.value + '_' + form.fweek.value + '.gif';
         var site = 'http://mesonet.agron.iastate.edu/~mm5/Endow/Images/' + image;
         newWindow = window.open(site); 
      }
    </SCRIPT>
  </HEAD>

<?php include("./include/header.php");
      include("./include/navigate.php");
?>    
      <TD class="content">
      <font class="rubric" style="text-align: center;"><br />
      Model Output<br /><br /><br />Run Date: 09.07.2002</font><br /><br />
      <form method="GET" action="cgi-bin/display_pan.py" name="first">
      <TABLE cellspacing="0" cellpadding="0" border="0">
      <TR>
              <TD><font class="rubric">Data Plot:</font>
              <SELECT name="fdata">
                      <option value="mm5_1">Soil Temperature, Layer 1
                      <option value="mm5_2">Soil Temperature Layer 2
                      <option value="mm5_3">Soil Temperature, Layer 3
                      <option value="mm5_4">Soil Temperature, Layer 4
                      <option value="mm5_5">Soil Moisture, Layer 1
                      <option value="mm5_6">Soil Moisture, Layer 2
                      <option value="mm5_7">Soil Moisture, Layer 3
                      <option value="mm5_8">Soil Moisture, Layer 4
                      <option value="mm5_9">Weekly Accumulated Precip.
              </SELECT>
              </TD>
 
              <TD><font class="rubric">Forecast Week:</font>
              <SELECT name="fweek">
                     <option value="0">Initialization
                     <option value="1">Week 1
                     <option value="2">Week 2
                     <option value="3">Week 3
                     <option value="4">Week 4
                     <option value="5">Week 5
                     <option value="6">Week 6
                     <option value="7">Week 7
                     <option value="8">Week 8
                     <option value="9">Week 9
                     <option value="10">Week 10
                     <option value="11">Week 11
                     <option value="12">Week 12
                     <option value="13">Week 13
                     <option value="14">Week 14
                     <option value="15">Week 15
              </SELECT>
              </TD>
     </TR>
     <TR>
     <TD><font class="rubric"><br /><br /><input type="SUBMIT" value="Create Plot">
        <input type="RESET"></font>
     </TD></TR></TABLE>
     </FORM></TD></TR>
     <?php include("./include/footer.php"); ?>

