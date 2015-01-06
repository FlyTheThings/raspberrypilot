<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML+RDFa 1.0//EN" "http://www.w3.org/MarkUp/DTD/xhtml-rdfa-1.dtd">
<html class="js" xmlns="http://www.w3.org/1999/xhtml" xml:lang="en" version="XHTML+RDFa 1.0" dir="ltr" xmlns:content="http://purl.org/rss/1.0/modules/content/" xmlns:dc="http://purl.org/dc/terms/" xmlns:foaf="http://xmlns.com/foaf/0.1/" xmlns:og="http://ogp.me/ns#" xmlns:rdfs="http://www.w3.org/2000/01/rdf-schema#" xmlns:sioc="http://rdfs.org/sioc/ns#" xmlns:sioct="http://rdfs.org/sioc/types#" xmlns:skos="http://www.w3.org/2004/02/skos/core#" xmlns:xsd="http://www.w3.org/2001/XMLSchema#"><head profile="http://www.w3.org/1999/xhtml/vocab">
  <meta http-equiv="Content-Type" content="text/html; charset=UTF-8">
<link rel="shortcut icon" href="http://www.pieter-jan.com/sites/default/files/LOGOCOMPACT.png" type="image/png">
<link rel="shortlink" href="http://www.pieter-jan.com/node/11">
<link rel="canonical" href="http://www.pieter-jan.com/node/11">
<meta name="Generator" content="Drupal 7 (http://drupal.org)">
  <title>Reading a IMU Without Kalman: The Complementary Filter | Pieter-Jan.com</title>
  <style type="text/css" media="all">
@import url("http://www.pieter-jan.com/modules/system/system.base.css?nfdnkb");
@import url("http://www.pieter-jan.com/modules/system/system.menus.css?nfdnkb");
@import url("http://www.pieter-jan.com/modules/system/system.messages.css?nfdnkb");
@import url("http://www.pieter-jan.com/modules/system/system.theme.css?nfdnkb");
</style>
<style type="text/css" media="all">
@import url("http://www.pieter-jan.com/modules/field/theme/field.css?nfdnkb");
@import url("http://www.pieter-jan.com/sites/all/modules/mollom/mollom.css?nfdnkb");
@import url("http://www.pieter-jan.com/modules/node/node.css?nfdnkb");
@import url("http://www.pieter-jan.com/modules/search/search.css?nfdnkb");
@import url("http://www.pieter-jan.com/modules/user/user.css?nfdnkb");
@import url("http://www.pieter-jan.com/sites/all/modules/views/css/views.css?nfdnkb");
</style>
<style type="text/css" media="all">
@import url("http://www.pieter-jan.com/sites/all/modules/ctools/css/ctools.css?nfdnkb");
@import url("http://www.pieter-jan.com/sites/all/modules/geshifilter/geshifilter.css?nfdnkb");
</style>
<style type="text/css" media="all">
@import url("http://www.pieter-jan.com/themes/bartik/css/layout.css?nfdnkb");
@import url("http://www.pieter-jan.com/themes/bartik/css/style.css?nfdnkb");
@import url("http://www.pieter-jan.com/sites/default/files/color/bartik-1efff6b3/colors.css?nfdnkb");
</style>
<style type="text/css" media="print">
@import url("http://www.pieter-jan.com/themes/bartik/css/print.css?nfdnkb");
</style>

<!--[if lte IE 7]>
<link type="text/css" rel="stylesheet" href="http://www.pieter-jan.com/themes/bartik/css/ie.css?nfdnkb" media="all" />
<![endif]-->

<!--[if IE 6]>
<link type="text/css" rel="stylesheet" href="http://www.pieter-jan.com/themes/bartik/css/ie6.css?nfdnkb" media="all" />
<![endif]-->
  <script src="Reading%20a%20IMU%20Without%20Kalman:%20The%20Complementary%20Filter%20%7C%20Pieter-Jan_files/widgets.js" id="twitter-wjs"></script><script src="Reading%20a%20IMU%20Without%20Kalman:%20The%20Complementary%20Filter%20%7C%20Pieter-Jan_files/ca-pub-4243283927665000.js" type="text/javascript" async=""></script><script src="Reading%20a%20IMU%20Without%20Kalman:%20The%20Complementary%20Filter%20%7C%20Pieter-Jan_files/analytics.js" async=""></script><script type="text/javascript" src="Reading%20a%20IMU%20Without%20Kalman:%20The%20Complementary%20Filter%20%7C%20Pieter-Jan_files/jquery.js"></script>
<script type="text/javascript" src="Reading%20a%20IMU%20Without%20Kalman:%20The%20Complementary%20Filter%20%7C%20Pieter-Jan_files/jquery_002.js"></script>
<script type="text/javascript" src="Reading%20a%20IMU%20Without%20Kalman:%20The%20Complementary%20Filter%20%7C%20Pieter-Jan_files/drupal.js"></script>
<script type="text/javascript" src="Reading%20a%20IMU%20Without%20Kalman:%20The%20Complementary%20Filter%20%7C%20Pieter-Jan_files/googleanalytics.js"></script>
<script type="text/javascript">
<!--//--><![CDATA[//><!--
(function(i,s,o,g,r,a,m){i["GoogleAnalyticsObject"]=r;i[r]=i[r]||function(){(i[r].q=i[r].q||[]).push(arguments)},i[r].l=1*new Date();a=s.createElement(o),m=s.getElementsByTagName(o)[0];a.async=1;a.src=g;m.parentNode.insertBefore(a,m)})(window,document,"script","//www.google-analytics.com/analytics.js","ga");ga("create", "UA-34303938-1", "auto");ga("send", "pageview");
//--><!]]>
</script>
<script type="text/javascript">
<!--//--><![CDATA[//><!--
jQuery.extend(Drupal.settings, {"basePath":"\/","pathPrefix":"","ajaxPageState":{"theme":"bartik","theme_token":"zVP7bp9x-dv07x7cncxXV-5Ic40dgM8czCuSMpT0vZg","js":{"misc\/jquery.js":1,"misc\/jquery.once.js":1,"misc\/drupal.js":1,"sites\/all\/modules\/google_analytics\/googleanalytics.js":1,"0":1},"css":{"modules\/system\/system.base.css":1,"modules\/system\/system.menus.css":1,"modules\/system\/system.messages.css":1,"modules\/system\/system.theme.css":1,"modules\/field\/theme\/field.css":1,"sites\/all\/modules\/mollom\/mollom.css":1,"modules\/node\/node.css":1,"modules\/search\/search.css":1,"modules\/user\/user.css":1,"sites\/all\/modules\/views\/css\/views.css":1,"sites\/all\/modules\/ctools\/css\/ctools.css":1,"sites\/all\/modules\/geshifilter\/geshifilter.css":1,"themes\/bartik\/css\/layout.css":1,"themes\/bartik\/css\/style.css":1,"themes\/bartik\/css\/colors.css":1,"themes\/bartik\/css\/print.css":1,"themes\/bartik\/css\/ie.css":1,"themes\/bartik\/css\/ie6.css":1}},"googleanalytics":{"trackOutbound":1,"trackMailto":1,"trackDownload":1,"trackDownloadExtensions":"7z|aac|arc|arj|asf|asx|avi|bin|csv|doc|exe|flv|gif|gz|gzip|hqx|jar|jpe?g|js|mp(2|3|4|e?g)|mov(ie)?|msi|msp|pdf|phps|png|ppt|qtm?|ra(m|r)?|sea|sit|tar|tgz|torrent|txt|wav|wma|wmv|wpd|xls|xml|z|zip"}});
//--><!]]>
</script>
</head>
<body data-twttr-rendered="true" class="html not-front not-logged-in one-sidebar sidebar-second page-node page-node- page-node-11 node-type-article">
  <div id="skip-link">
    <a href="#main-content" class="element-invisible element-focusable">Skip to main content</a>
  </div>
    <div id="page-wrapper"><div id="page">

  <div id="header" class="without-secondary-menu"><div class="section clearfix">

          <a href="http://www.pieter-jan.com/" title="Home" rel="home" id="logo">
        <img src="Reading%20a%20IMU%20Without%20Kalman:%20The%20Complementary%20Filter%20%7C%20Pieter-Jan_files/Logotest_0.png" alt="Home">
      </a>
    
          <div id="name-and-slogan" class="element-invisible">

                              <div id="site-name" class="element-invisible">
              <strong>
                <a href="http://www.pieter-jan.com/" title="Home" rel="home"><span>Pieter-Jan.com</span></a>
              </strong>
            </div>
                  
                  <div id="site-slogan" class="element-invisible">
            Creativity In Automation &amp; More          </div>
        
      </div> <!-- /#name-and-slogan -->
    
    
          <div id="main-menu" class="navigation">
        <h2 class="element-invisible">Main menu</h2><ul id="main-menu-links" class="links clearfix"><li class="menu-198 first"><a href="http://www.pieter-jan.com/">Home</a></li>
<li class="menu-678 last"><a href="http://www.pieter-jan.com/contact" title="">Contact</a></li>
</ul>      </div> <!-- /#main-menu -->
    
    
  </div></div> <!-- /.section, /#header -->

  
  
  <div id="main-wrapper" class="clearfix"><div id="main" class="clearfix">

    
    
    <div id="content" class="column"><div class="section">
            <a id="main-content"></a>
                    <h1 class="title" id="page-title">
          Reading a IMU Without Kalman: The Complementary Filter        </h1>
                          <div class="tabs">
                  </div>
              <div class="region region-help">
    <div id="block-block-5" class="block block-block">

    
  <div class="content">
    <script type="text/javascript"><!--
google_ad_client = "ca-pub-4243283927665000";
/* AboveMainContentLeaderboard */
google_ad_slot = "8970401973";
google_ad_width = 728;
google_ad_height = 90;
//-->
</script>
<script type="text/javascript" src="Reading%20a%20IMU%20Without%20Kalman:%20The%20Complementary%20Filter%20%7C%20Pieter-Jan_files/show_ads.js">
</script><ins id="aswift_0_expand" style="display:inline-table;border:none;height:90px;margin:0;padding:0;position:relative;visibility:visible;width:728px;background-color:transparent"><ins id="aswift_0_anchor" style="display:block;border:none;height:90px;margin:0;padding:0;position:relative;visibility:visible;width:728px;background-color:transparent"><iframe marginwidth="0" marginheight="0" vspace="0" hspace="0" allowtransparency="true" allowfullscreen="true" onload="var i=this.id,s=window.google_iframe_oncopy,H=s&amp;&amp;s.handlers,h=H&amp;&amp;H[i],w=this.contentWindow,d;try{d=w.document}catch(e){}if(h&amp;&amp;d&amp;&amp;(!d.body||!d.body.firstChild)){if(h.call){setTimeout(h,0)}else if(h.match){try{h=s.upd(h,i)}catch(e){}w.location.replace(h)}}" id="aswift_0" name="aswift_0" style="left:0;position:absolute;top:0;" scrolling="no" width="728" frameborder="0" height="90"></iframe></ins></ins>  </div>
</div>
  </div>
              <div class="region region-content">
    <div id="block-system-main" class="block block-system">

    
  <div class="content">
    <div id="node-11" class="node node-article node-promoted node-full clearfix" about="/node/11" typeof="sioc:Item foaf:Document">

      <span property="dc:title" content="Reading a IMU Without Kalman: The Complementary Filter" class="rdf-meta element-hidden"></span>
      <div class="meta submitted">
            <span property="dc:date dc:created" content="2013-04-26T08:38:39-04:00" datatype="xsd:dateTime" rel="sioc:has_creator">Submitted by <span class="username" xml:lang="" about="/user/1" typeof="sioc:UserAccount" property="foaf:name" datatype="">Pieter-Jan</span> on Fri, 26/04/2013 - 08:38</span>    </div>
  
  <div class="content clearfix">
    <div class="field field-name-body field-type-text-with-summary field-label-hidden"><div class="field-items"><div class="field-item even" property="content:encoded">

<div align="justify">
<p>
These days, IMU's (Intertial Measurement Units) are used everywhere. 
They are e.g. the sensors that are responsible for keeping track of the 
oriëntation of your mobile phone. This can be very useful for automatic 
screen tilting etc. The reason I am interested in this sensor is because
 I want to use it to stabilize my quadrocopter. If you are here for 
another reason, this is not a problem as this tutorial will apply for 
everyone.
</p>
<p>When looking for the best way to make use of a IMU-sensor, thus <b>combine the accelerometer and gyroscope data</b>,
 a lot of people get fooled into using the very powerful but complex 
Kalman filter. However the Kalman filter is great, there are 2 big 
problems with it that make it hard to use:
</p><ul>
<li>Very complex to understand.</li> 
<li>Very hard, if not impossible, to implement on certain hardware (8-bit microcontroller etc.)</li>
</ul>
In this tutorial I will present a solution for both of these problems 
with another type of filter: the complementary filter. It's extremely 
easy to understand, and even easier to implement.
<p></p>
<h2>Why do I need a filter?</h2>
<p>Most IMU's have 6 DOF (Degrees Of Freedom). This means that there are
 3 accelerometers, and 3 gyrosocopes inside the unit. If you remember 
anything from a robotics class you might have taken, you might be fooled
 into thinking that the IMU will be able to measure the precise position
 and orientation of the object it is attached to. This because they have
 told you that an object in free space has 6DOF. So if we can measure 
them all, we know everything, right? Well ... not really. The sensor 
data is not good enough to be used in this way. 
</p>	
<p>
We will use both the accelerometer and gyroscope data for the same 
purpose: obtaining the angular position of the object. The gyroscope can
 do this by integrating the angular velocity over time, as was explained
 in a previous article. To obtain the angular position with the 
accelerometer, we are going to determine the position of the gravity 
vector (g-force) which is always visible on the accelerometer. This can 
easily be done by using an <a href="http://en.wikipedia.org/wiki/Atan2" target="_blank">atan2</a> function. In both these cases, there is a big problem, which makes the data very hard to use without filter.
</p>
<h3>The problem with accelerometers</h3>
<p>As an accelerometer measures all forces that are working on the 
object, it will also see a lot more than just the gravity vector. Every 
small force working on the object will disturb our measurement 
completely. If we are working on an actuated system (like the 
quadrocopter), then the forces that drive the system will be visible on 
the sensor as well. The accelerometer data is reliable only on the long 
term, so a "low pass" filter has to be used.
</p>
<h3>The problem with gyroscopes</h3>
<p><a href="http://www.pieter-jan.com/node/7" target="_blank">In one of the previous articles</a>
 I explained how to obtain the angular position by use of a gyroscope. 
We saw that it was very easy to obtain an accurate measurement that was 
not susceptible to external forces. The less good news was that, because
 of the integration over time, the measurement has the tendency to 
drift, not returning to zero when the system went back to its original 
position. The gyroscope data is reliable only on the short term, as it 
starts to drift on the long term.
</p>
<h2>The complementary filter</h2>
<p>
The complementary filter gives us a "best of both worlds" kind of deal. 
On the short term, we use the data from the gyroscope, because it is 
very precise and not susceptible to external forces. On the long term, 
we use the data from the accelerometer, as it does not drift. In it's 
most simple form, the filter looks as follows:
</p><center><img src="Reading%20a%20IMU%20Without%20Kalman:%20The%20Complementary%20Filter%20%7C%20Pieter-Jan_files/CompFilter_Eq.gif"></center>
The gyroscope data is integrated every timestep with the current angle 
value. After this it is combined with the low-pass data from the 
accelerometer (already processed with atan2). The constants (0.98 and 
0.02) have to add up to 1 but can of course be changed to tune the 
filter properly.
<p></p>
<p>
I implemented this filter on a Raspberry Pi using a MPU6050 IMU. I will 
not discuss how to read data from the MPU6050 in this article (contact 
me if you want the source code). The implementation of the filter is 
shown in the code snippet below. As you can see it is very easy in 
comparison to Kalman.
</p>
<p>
The function "ComplementaryFilter" has to be used in a infinite loop. 
Every iteration the pitch and roll angle values are updated with the new
 gyroscope values by means of integration over time. The filter then 
checks if the magnitude of the force seen by the accelerometer has a 
reasonable value that could be the real g-force vector. If the value is 
too small or too big, we know for sure that it is a disturbance we don't
 need to take into account. Afterwards, it will update the pitch and 
roll angles with the accelerometer data by taking 98% of the current 
value, and adding 2% of the angle calculated by the accelerometer. This 
will ensure that the measurement won't drift, but that it will be very 
accurate on the short term.   
</p>
<p>
It should be noted that this code snippet is only an example, and should
 not be copy pasted as it will probably not work like that without using
 the exact same settings as me. 
</p>
<div class="geshifilter"><pre class="c geshifilter-c" style="font-family:monospace;"><span style="color: #339933;">#define ACCELEROMETER_SENSITIVITY 8192.0</span>
<span style="color: #339933;">#define GYROSCOPE_SENSITIVITY 65.536</span>
&nbsp;
<span style="color: #339933;">#define M_PI 3.14159265359	    </span>
&nbsp;
<span style="color: #339933;">#define dt 0.01							// 10 ms sample rate!    </span>
&nbsp;
<span style="color: #993333;">void</span> ComplementaryFilter<span style="color: #009900;">(</span><span style="color: #993333;">short</span> accData<span style="color: #009900;">[</span><span style="color: #0000dd;">3</span><span style="color: #009900;">]</span><span style="color: #339933;">,</span> <span style="color: #993333;">short</span> gyrData<span style="color: #009900;">[</span><span style="color: #0000dd;">3</span><span style="color: #009900;">]</span><span style="color: #339933;">,</span> <span style="color: #993333;">float</span> <span style="color: #339933;">*</span>pitch<span style="color: #339933;">,</span> <span style="color: #993333;">float</span> <span style="color: #339933;">*</span>roll<span style="color: #009900;">)</span>
<span style="color: #009900;">{</span>
    <span style="color: #993333;">float</span> pitchAcc<span style="color: #339933;">,</span> rollAcc<span style="color: #339933;">;</span>               
&nbsp;
    <span style="color: #666666; font-style: italic;">// Integrate the gyroscope data -&gt; int(angularSpeed) = angle</span>
    <span style="color: #339933;">*</span>pitch <span style="color: #339933;">+=</span> <span style="color: #009900;">(</span><span style="color: #009900;">(</span><span style="color: #993333;">float</span><span style="color: #009900;">)</span>gyrData<span style="color: #009900;">[</span><span style="color: #0000dd;">0</span><span style="color: #009900;">]</span> <span style="color: #339933;">/</span> GYROSCOPE_SENSITIVITY<span style="color: #009900;">)</span> <span style="color: #339933;">*</span> dt<span style="color: #339933;">;</span> <span style="color: #666666; font-style: italic;">// Angle around the X-axis</span>
    <span style="color: #339933;">*</span>roll <span style="color: #339933;">-=</span> <span style="color: #009900;">(</span><span style="color: #009900;">(</span><span style="color: #993333;">float</span><span style="color: #009900;">)</span>gyrData<span style="color: #009900;">[</span><span style="color: #0000dd;">1</span><span style="color: #009900;">]</span> <span style="color: #339933;">/</span> GYROSCOPE_SENSITIVITY<span style="color: #009900;">)</span> <span style="color: #339933;">*</span> dt<span style="color: #339933;">;</span>    <span style="color: #666666; font-style: italic;">// Angle around the Y-axis</span>
&nbsp;
    <span style="color: #666666; font-style: italic;">// Compensate for drift with accelerometer data if !bullshit</span>
    <span style="color: #666666; font-style: italic;">// Sensitivity = -2 to 2 G at 16Bit -&gt; 2G = 32768 &amp;&amp; 0.5G = 8192</span>
    <span style="color: #993333;">int</span> forceMagnitudeApprox <span style="color: #339933;">=</span> <a href="http://www.opengroup.org/onlinepubs/009695399/functions/abs.html"><span style="color: #000066;">abs</span></a><span style="color: #009900;">(</span>accData<span style="color: #009900;">[</span><span style="color: #0000dd;">0</span><span style="color: #009900;">]</span><span style="color: #009900;">)</span> <span style="color: #339933;">+</span> <a href="http://www.opengroup.org/onlinepubs/009695399/functions/abs.html"><span style="color: #000066;">abs</span></a><span style="color: #009900;">(</span>accData<span style="color: #009900;">[</span><span style="color: #0000dd;">1</span><span style="color: #009900;">]</span><span style="color: #009900;">)</span> <span style="color: #339933;">+</span> <a href="http://www.opengroup.org/onlinepubs/009695399/functions/abs.html"><span style="color: #000066;">abs</span></a><span style="color: #009900;">(</span>accData<span style="color: #009900;">[</span><span style="color: #0000dd;">2</span><span style="color: #009900;">]</span><span style="color: #009900;">)</span><span style="color: #339933;">;</span>
    <span style="color: #b1b100;">if</span> <span style="color: #009900;">(</span>forceMagnitudeApprox <span style="color: #339933;">&gt;</span> <span style="color: #0000dd;">8192</span> <span style="color: #339933;">&amp;&amp;</span> forceMagnitudeApprox <span style="color: #339933;">&lt;</span> <span style="color: #0000dd;">32768</span><span style="color: #009900;">)</span>
    <span style="color: #009900;">{</span>
	<span style="color: #666666; font-style: italic;">// Turning around the X axis results in a vector on the Y-axis</span>
        pitchAcc <span style="color: #339933;">=</span> atan2f<span style="color: #009900;">(</span><span style="color: #009900;">(</span><span style="color: #993333;">float</span><span style="color: #009900;">)</span>accData<span style="color: #009900;">[</span><span style="color: #0000dd;">1</span><span style="color: #009900;">]</span><span style="color: #339933;">,</span> <span style="color: #009900;">(</span><span style="color: #993333;">float</span><span style="color: #009900;">)</span>accData<span style="color: #009900;">[</span><span style="color: #0000dd;">2</span><span style="color: #009900;">]</span><span style="color: #009900;">)</span> <span style="color: #339933;">*</span> <span style="color: #0000dd;">180</span> <span style="color: #339933;">/</span> M_PI<span style="color: #339933;">;</span>
        <span style="color: #339933;">*</span>pitch <span style="color: #339933;">=</span> <span style="color: #339933;">*</span>pitch <span style="color: #339933;">*</span> <span style="color:#800080;">0.98</span> <span style="color: #339933;">+</span> pitchAcc <span style="color: #339933;">*</span> <span style="color:#800080;">0.02</span><span style="color: #339933;">;</span>
&nbsp;
	<span style="color: #666666; font-style: italic;">// Turning around the Y axis results in a vector on the X-axis</span>
        rollAcc <span style="color: #339933;">=</span> atan2f<span style="color: #009900;">(</span><span style="color: #009900;">(</span><span style="color: #993333;">float</span><span style="color: #009900;">)</span>accData<span style="color: #009900;">[</span><span style="color: #0000dd;">0</span><span style="color: #009900;">]</span><span style="color: #339933;">,</span> <span style="color: #009900;">(</span><span style="color: #993333;">float</span><span style="color: #009900;">)</span>accData<span style="color: #009900;">[</span><span style="color: #0000dd;">2</span><span style="color: #009900;">]</span><span style="color: #009900;">)</span> <span style="color: #339933;">*</span> <span style="color: #0000dd;">180</span> <span style="color: #339933;">/</span> M_PI<span style="color: #339933;">;</span>
        <span style="color: #339933;">*</span>roll <span style="color: #339933;">=</span> <span style="color: #339933;">*</span>roll <span style="color: #339933;">*</span> <span style="color:#800080;">0.98</span> <span style="color: #339933;">+</span> rollAcc <span style="color: #339933;">*</span> <span style="color:#800080;">0.02</span><span style="color: #339933;">;</span>
    <span style="color: #009900;">}</span>
<span style="color: #009900;">}</span> </pre></div>
<p>
If we use the sweetness of having a <b>real</b> computer (Raspberry Pi) 
collecting our data, we can easily create a graph using GNUPlot. It's 
easily seen that the filter (red) follows the gyroscope (blue) for fast 
changes, but keeps following the mean value of the accelerometer (green)
 for slower changes, thus not feeling the noisy accelerometer data and 
not drifting away eiter. 
<br></p><center><a href="http://www.pieter-jan.com/images/Complementary_Filter.png" target="_blank"><img src="Reading%20a%20IMU%20Without%20Kalman:%20The%20Complementary%20Filter%20%7C%20Pieter-Jan_files/Complementary_Filter.png" width="100%/"></a></center>
<p></p>
<p>
The filter is very easy and light to implement making it perfect for 
embedded systems. It should be noted that for stabilization systems 
(like the quadrocopter), the angle will never be very big. The atan2 
function can then be approximated by using a small angle approximation. 
In this way, this filter could easily fit on an 8 bit system. 
</p>
<p>
Hope this article was of any help.
</p>
</div>


</div></div></div><div class="field field-name-field-tags field-type-taxonomy-term-reference field-label-above clearfix"><h3 class="field-label">Tags: </h3><ul class="links"><li class="taxonomy-term-reference-0" rel="dc:subject"><a href="http://www.pieter-jan.com/taxonomy/term/11" typeof="skos:Concept" property="rdfs:label skos:prefLabel" datatype="">Complementary Filter</a></li><li class="taxonomy-term-reference-1" rel="dc:subject"><a href="http://www.pieter-jan.com/taxonomy/term/12" typeof="skos:Concept" property="rdfs:label skos:prefLabel" datatype="">Gyroscope</a></li><li class="taxonomy-term-reference-2" rel="dc:subject"><a href="http://www.pieter-jan.com/taxonomy/term/13" typeof="skos:Concept" property="rdfs:label skos:prefLabel" datatype="">Accelerometer</a></li><li class="taxonomy-term-reference-3" rel="dc:subject"><a href="http://www.pieter-jan.com/taxonomy/term/14" typeof="skos:Concept" property="rdfs:label skos:prefLabel" datatype="">IMU</a></li><li class="taxonomy-term-reference-4" rel="dc:subject"><a href="http://www.pieter-jan.com/taxonomy/term/9" typeof="skos:Concept" property="rdfs:label skos:prefLabel" datatype="">Quadrocopter</a></li></ul></div>  </div>

  
  
</div>
  </div>
</div>
  </div>
      
    </div></div> <!-- /.section, /#content -->

          <div id="sidebar-second" class="column sidebar"><div class="section">
          <div class="region region-sidebar-second">
    <div id="block-block-2" class="block block-block">

    
  <div class="content">
    <center><img src="Reading%20a%20IMU%20Without%20Kalman:%20The%20Complementary%20Filter%20%7C%20Pieter-Jan_files/08af29c.jpg" width="100%"></center>
<br>
<p>
Welcome to my website! I'm <b>Pieter-Jan Van de Maele</b>, an engineering student living in <b>Brussels</b>, Belgium. I try to know as much as possible about <b>electronics</b>, <b>robotics</b> and <b>programming</b>. Every day is a learning experience, and I hope to use this website as a way to organize the chaos in my head. Connect with me:
</p><center>
<iframe style="width: 147px; height: 20px;" data-twttr-rendered="true" title="Twitter Follow Button" class="twitter-follow-button twitter-follow-button" src="Reading%20a%20IMU%20Without%20Kalman:%20The%20Complementary%20Filter%20%7C%20Pieter-Jan_files/follow_button.html" allowtransparency="true" id="twitter-widget-0" scrolling="no" frameborder="0"></iframe>
<script>!function(d,s,id){var js,fjs=d.getElementsByTagName(s)[0],p=/^http:/.test(d.location)?'http':'https';if(!d.getElementById(id)){js=d.createElement(s);js.id=id;js.src=p+'://platform.twitter.com/widgets.js';fjs.parentNode.insertBefore(js,fjs);}}(document, 'script', 'twitter-wjs');</script>
<a href="http://be.linkedin.com/pub/pieter-jan-van-de-maele/24/14/217">
      
          <img src="Reading%20a%20IMU%20Without%20Kalman:%20The%20Complementary%20Filter%20%7C%20Pieter-Jan_files/btn_liprofile_blue_80x15.png" alt="View Pieter-Jan Van de Maele's profile on LinkedIn" width="80" height="15" border="0">
        
    </a>
<p></p>
</center>  </div>
</div>
<div id="block-views-related-content-block" class="block block-views">

    <h2>Related Content</h2>
  
  <div class="content">
    <div class="view view-related-content view-id-related_content view-display-id-block view-dom-id-b3e40c46e75dbd3b64b9e3b080e50a34">
        
  
  
      <div class="view-content">
      <div class="item-list">    <ul>          <li class="views-row views-row-1 views-row-odd views-row-first">  
  <div class="views-field views-field-title">        <span class="field-content"><a href="http://www.pieter-jan.com/node/3">Hacking the Hobbyking HK401B gyroscope module</a></span>  </div></li>
          <li class="views-row views-row-2 views-row-even">  
  <div class="views-field views-field-title">        <span class="field-content"><a href="http://www.pieter-jan.com/node/6">Four (4) independent PWM-signals generated with one PIC</a></span>  </div></li>
          <li class="views-row views-row-3 views-row-odd views-row-last">  
  <div class="views-field views-field-title">        <span class="field-content"><a href="http://www.pieter-jan.com/node/7">Getting the angular position from gyroscope data</a></span>  </div></li>
      </ul></div>    </div>
  
  
  
  
  
  
</div>  </div>
</div>
<div id="block-views-workblock-block-1" class="block block-views">

    <h2>Articles</h2>
  
  <div class="content">
    <div class="view view-workblock view-id-workblock view-display-id-block_1 view-dom-id-280f7379abe84b647c3d5058ec3cd370">
        
  
  
      <div class="view-content">
      <div class="item-list">    <ul>          <li class="views-row views-row-1 views-row-odd views-row-first">  
  <div class="views-field views-field-title">        <span class="field-content"><a href="http://www.pieter-jan.com/node/15">Low Level Programming of the Raspberry Pi in C</a></span>  </div>  
  <div class="views-field views-field-created">    <span class="views-label views-label-created">Post date: </span>    <span class="field-content">Friday, May 24, 2013 - 05:10</span>  </div></li>
          <li class="views-row views-row-2 views-row-even">  
  <div class="views-field views-field-title">        <span class="field-content"><a href="http://www.pieter-jan.com/node/14">DIY Raspberry Pi Breakout Cable</a></span>  </div>  
  <div class="views-field views-field-created">    <span class="views-label views-label-created">Post date: </span>    <span class="field-content">Monday, May 20, 2013 - 09:54</span>  </div></li>
          <li class="views-row views-row-3 views-row-odd">  
  <div class="views-field views-field-title">        <span class="field-content"><a href="http://www.pieter-jan.com/node/11" class="active">Reading a IMU Without Kalman: The Complementary Filter</a></span>  </div>  
  <div class="views-field views-field-created">    <span class="views-label views-label-created">Post date: </span>    <span class="field-content">Friday, April 26, 2013 - 08:38</span>  </div></li>
          <li class="views-row views-row-4 views-row-even">  
  <div class="views-field views-field-title">        <span class="field-content"><a href="http://www.pieter-jan.com/node/12">Replacing the Raspberry Pi's SD Card Socket</a></span>  </div>  
  <div class="views-field views-field-created">    <span class="views-label views-label-created">Post date: </span>    <span class="field-content">Thursday, October 11, 2012 - 11:06</span>  </div></li>
          <li class="views-row views-row-5 views-row-odd">  
  <div class="views-field views-field-title">        <span class="field-content"><a href="http://www.pieter-jan.com/node/10">Reading Nintendo 64 controller with PIC microcontroller</a></span>  </div>  
  <div class="views-field views-field-created">    <span class="views-label views-label-created">Post date: </span>    <span class="field-content">Wednesday, September 12, 2012 - 08:37</span>  </div></li>
          <li class="views-row views-row-6 views-row-even">  
  <div class="views-field views-field-title">        <span class="field-content"><a href="http://www.pieter-jan.com/node/7">Getting the angular position from gyroscope data</a></span>  </div>  
  <div class="views-field views-field-created">    <span class="views-label views-label-created">Post date: </span>    <span class="field-content">Thursday, September 6, 2012 - 02:41</span>  </div></li>
          <li class="views-row views-row-7 views-row-odd">  
  <div class="views-field views-field-title">        <span class="field-content"><a href="http://www.pieter-jan.com/node/4">Setting up the Raspberry Pi for programming from Mac OS X</a></span>  </div>  
  <div class="views-field views-field-created">    <span class="views-label views-label-created">Post date: </span>    <span class="field-content">Monday, September 3, 2012 - 13:49</span>  </div></li>
          <li class="views-row views-row-8 views-row-even">  
  <div class="views-field views-field-title">        <span class="field-content"><a href="http://www.pieter-jan.com/node/13">Universal/Jamming Gripper Prototype</a></span>  </div>  
  <div class="views-field views-field-created">    <span class="views-label views-label-created">Post date: </span>    <span class="field-content">Monday, September 3, 2012 - 12:25</span>  </div></li>
          <li class="views-row views-row-9 views-row-odd">  
  <div class="views-field views-field-title">        <span class="field-content"><a href="http://www.pieter-jan.com/node/3">Hacking the Hobbyking HK401B gyroscope module</a></span>  </div>  
  <div class="views-field views-field-created">    <span class="views-label views-label-created">Post date: </span>    <span class="field-content">Saturday, July 21, 2012 - 10:12</span>  </div></li>
          <li class="views-row views-row-10 views-row-even">  
  <div class="views-field views-field-title">        <span class="field-content"><a href="http://www.pieter-jan.com/node/6">Four (4) independent PWM-signals generated with one PIC</a></span>  </div>  
  <div class="views-field views-field-created">    <span class="views-label views-label-created">Post date: </span>    <span class="field-content">Tuesday, July 17, 2012 - 11:55</span>  </div></li>
          <li class="views-row views-row-11 views-row-odd">  
  <div class="views-field views-field-title">        <span class="field-content"><a href="http://www.pieter-jan.com/node/17">NARF Keypoint Test</a></span>  </div>  
  <div class="views-field views-field-created">    <span class="views-label views-label-created">Post date: </span>    <span class="field-content">Saturday, May 26, 2012 - 10:23</span>  </div></li>
          <li class="views-row views-row-12 views-row-even">  
  <div class="views-field views-field-title">        <span class="field-content"><a href="http://www.pieter-jan.com/node/5">Custom ROI (Region Of Interest) with OpenCV</a></span>  </div>  
  <div class="views-field views-field-created">    <span class="views-label views-label-created">Post date: </span>    <span class="field-content">Friday, April 6, 2012 - 15:44</span>  </div></li>
          <li class="views-row views-row-13 views-row-odd views-row-last">  
  <div class="views-field views-field-title">        <span class="field-content"><a href="http://www.pieter-jan.com/node/2">Bachelor Assignment</a></span>  </div>  
  <div class="views-field views-field-created">    <span class="views-label views-label-created">Post date: </span>    <span class="field-content">Monday, May 30, 2011 - 08:33</span>  </div></li>
      </ul></div>    </div>
  
  
  
  
  
  
</div>  </div>
</div>
<div id="block-block-3" class="block block-block">

    
  <div class="content">
    <center><script type="text/javascript"><!--
google_ad_client = "ca-pub-4243283927665000";
/* Ads3 */
google_ad_slot = "6489770370";
google_ad_width = 160;
google_ad_height = 600;
//-->
</script>
<script type="text/javascript" src="Reading%20a%20IMU%20Without%20Kalman:%20The%20Complementary%20Filter%20%7C%20Pieter-Jan_files/show_ads.js">
</script><ins id="aswift_1_expand" style="display:inline-table;border:none;height:600px;margin:0;padding:0;position:relative;visibility:visible;width:160px;background-color:transparent"><ins id="aswift_1_anchor" style="display:block;border:none;height:600px;margin:0;padding:0;position:relative;visibility:visible;width:160px;background-color:transparent"><iframe marginwidth="0" marginheight="0" vspace="0" hspace="0" allowtransparency="true" allowfullscreen="true" onload="var i=this.id,s=window.google_iframe_oncopy,H=s&amp;&amp;s.handlers,h=H&amp;&amp;H[i],w=this.contentWindow,d;try{d=w.document}catch(e){}if(h&amp;&amp;d&amp;&amp;(!d.body||!d.body.firstChild)){if(h.call){setTimeout(h,0)}else if(h.match){try{h=s.upd(h,i)}catch(e){}w.location.replace(h)}}" id="aswift_1" name="aswift_1" style="left:0;position:absolute;top:0;" scrolling="no" width="160" frameborder="0" height="600"></iframe></ins></ins></center>  </div>
</div>
  </div>
      </div></div> <!-- /.section, /#sidebar-second -->
    
  </div></div> <!-- /#main, /#main-wrapper -->

  
  <div id="footer-wrapper"><div class="section">

    
          <div id="footer" class="clearfix">
          <div class="region region-footer">
    <div id="block-block-1" class="block block-block">

    
  <div class="content">
    <center><img src="Reading%20a%20IMU%20Without%20Kalman:%20The%20Complementary%20Filter%20%7C%20Pieter-Jan_files/LOGOCOMPACT_Smaller.png"></center>
<br>
<center>© 2013 <b>Pieter-Jan Van de Maele</b> All Rights Reserved</center>  </div>
</div>
  </div>
      </div> <!-- /#footer -->
    
  </div></div> <!-- /.section, /#footer-wrapper -->

</div></div> <!-- /#page, /#page-wrapper -->
  

</body></html>