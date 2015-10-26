/* Basic information */
var bar_sysst = new Array("<j mlword:MSGID_SYS_STAT>");
var url_sysst = new Array("sysst");
var bar_usb = new Array("<j mlword:MSGID_USB_STAT>");
var url_usb = new Array("usb");
var bar_wlsst = new Array("<j mlword:MSGID_WLS_STAT>");
var url_wlsst = new Array("wlsst");
var bar_basic = new Array("<j mlword:MSGID_GENERAL_CONF>");
var url_basic = new Array("basic");
var bar_tcpip = new Array("<j mlword:MSGID_TCPIP_CONF>");
var url_tcpip = new Array("tcpip");
var bar_ntp  = new Array("<j mlword:MSGID_NTP_CONF>");
var url_ntp  = new Array("ntp");
var bar_dlna  = new Array("<j mlword:MSGID_DLNA_CONF>");
var url_dlna  = new Array("dlna");
var bar_devc  = new Array("<j mlword:MSGID_DEVC_SERVER_CONF>");
var url_devc  = new Array("devc");
var bar_wless  = new Array("<j mlword:MSGID_WIRELESS_CONF>");
var url_wless  = new Array("wless");
var bar_renew = new Array("<j mlword:MSGID_SERVER_RESTART>");
var url_renew = new Array("renew");
var bar_fact  = new Array("<j mlword:MSGID_FACTORY_DEFAULT_2>");
var url_fact  = new Array("fact");
var bar_firm  = new Array("<j mlword:MSGID_FIRM_UP_CONF>");
var url_firm  = new Array("firm");
var bar_help = new Array("<j mlword:MSGID_STATUS_INFO>", "<j mlword:MSGID_NETWORK_CONF>", "<j mlword:MSGID_MAINTENANCE>");
var url_help = new Array("help_stat", "help_conf", "help_mainte");
var bar__ = new Array("&nbsp;");
var url__ = new Array("");

var title = new Array(
/*	id	 ,	menu lavel,							url,						          page title,							sub title,		sub url,	*/
 "_menu",	"<j mlword:MSGID_STATUS_INFO>",		"",							          "",									"bar__",		"url__",
 "sysst",	"<j mlword:MSGID_SYS>",				"./public/stat/sysst_indx.htm",	      "<j mlword:MSGID_SYS_STAT>",			"bar_sysst",	"url_sysst",
 "usb",		"<j mlword:MSGID_USB>",				"./public/stat/usb_indx.htm",	      "<j mlword:MSGID_USB_STAT>",			"bar_usb",		"url_usb",
 "wlsst",	"<j mlword:MSGID_WLS>",				"./public/stat/wlsst_indx.htm",	      "<j mlword:MSGID_WLS_STAT>",			"bar_wlsst",	"url_wlsst",
 "___",		"",									"",							          "",									"bar__",		"url__",
 "_menu",	"<j mlword:MSGID_NETWORK_CONF>",	"",							          "",									"bar__",		"url__",
 "basic",	"<j mlword:MSGID_GENERAL>",			"./private/conf/basic_indx.htm",	  "<j mlword:MSGID_GENERAL_CONF>",		"bar_basic",	"url_basic",
 "tcpip",	"<j mlword:MSGID_TCPIP>",			"./private/conf/tcpip_indx.htm",	  "<j mlword:MSGID_TCPIP_CONF>",		"bar_tcpip",	"url_tcpip",
 "wless",	"<j mlword:MSGID_WIRELESS_LAN>",	"./private/conf/wless_indx.htm",	  "<j mlword:MSGID_WIRELESS_CONF>", 	"bar_wless",	"url_wless",
 "ntp",		"<j mlword:MSGID_NTP>",				"./private/conf/ntp_indx.htm",		  "<j mlword:MSGID_NTP_CONF>",			"bar_ntp",		"url_ntp",
 "dlna",	"<j mlword:MSGID_DLNA>",			"./private/conf/dlna_indx.htm",		  "<j mlword:MSGID_DLNA_CONF>",			"bar_dlna",		"url_dlna",
 "devc",	"<j mlword:MSGID_DEVC_SERVER>",		"./private/conf/devc_server_indx.htm","<j mlword:MSGID_DEVC_SERVER_CONF>",	"bar_devc",		"url_devc",
 "___",		"",									"",							          "",									"bar__",		"url__",
 "_menu",	"<j mlword:MSGID_MAINTENANCE>",		"",							          "",									"bar__",		"url__",
 "firm",	"<j mlword:MSGID_FIRM_UP>",			"./private/mainte/firm_up_indx.htm",  "<j mlword:MSGID_FIRM_UP_CONF>",		"bar_firm",		"url_firm",
 "renew",	"<j mlword:MSGID_RESTART>",			"./private/mainte/renew_indx.htm",	  "<j mlword:MSGID_SERVER_RESTART>",	"bar_renew",	"url_renew",
 "fact",	"<j mlword:MSGID_FACTORY_DEFAULT>",	"./private/mainte/fact_indx.htm",	  "<j mlword:MSGID_FACTORY_DEFAULT_2>",	"bar_fact",		"url_fact",
 "homep",	"<j mlword:MSGID_HOMEPAGE>",		"http://www.belkin.com/",	          "<j mlword:MSGID_HOMEPAGE>",			"bar__",		"bar__",
 "___",		"",									"",							          "",									"bar__",		"url__",
 "_mend",	"",									"",							          "",									"bar__",		"bar__",
 "help",	"",									"",							          "<j mlword:MSGID_HELP>",				"bar_help",		"url_help",
 "error",	"",									"",							          "Error",								"bar__",		"bar__",
 "_end",	"",									"",							          "---",								"bar__",		"bar__"
);

var colnum = 6;
function get_titlenum(n){
	for(i=0;i<title.length;i+=colnum){
		if(title[i]==n){
			return (i);
		}
		if(title[i]=="_end")return (i);
	}
	return (title.length-colnum);
}
function get_menuid(i) {
	if ((i*colnum)>title.length) return title[title.length-colnum];
	return title[i*colnum];
}
function get_menutitle(i) {
	if ((i*colnum)>title.length) return title[title.length-colnum];
	return title[i*colnum+1];
}
function get_menuuri(i) {
	if ((i*colnum)>title.length) return title[title.length-colnum];
	return title[i*colnum+2];
}
function get_title(n) {
	document.write(title[get_titlenum(n)+3]);
}
function get_pagetitle(n) {
	return (title[get_titlenum(n)+4]);
}
function get_pageurl(n) {
	return (title[get_titlenum(n)+5]);
}
function get_subid(id, url) {
	u = eval(get_pageurl(id));
	for(i = 0; i < u.length; i++) {
		if (u[i]+"_indx.htm" == url) {
			return i;
		}
	}
	return 0;
}
function mouseover(id, i) {
	var color = new Array("blue" , "#181E5C");/*.dactv a:hover */
	tab = document.getElementById("tab" +id);
	tab.style.backgroundColor = color[i];
}
function create_mainbar(n, a) {
	b = eval(get_pagetitle(n));
	u = eval(get_pageurl(n));
	menuuri=title[get_titlenum(n)+2];

	with(document) {
		write("<table width=\"100%\" border=0 cellpadding=0 cellspacing=0 class=\"dactv\">");
		write("<tr><td class=\"w1h7\"></td></tr>");
		write("<tr><td class=\"mainbar_left\">&nbsp;</td></tr>");
		write("<tr><td class=\"w1h6\"></td></tr>");
		write("</table>");
		write("</td>");
		for (i = 0; i < b.length; i++) {
			write("<td width=120>");
			if (i!=a && u[i] != ""){
				write('<span onMouseOver="mouseover('+i+', 0);" onMouseOut="mouseover('+i+', 1);">');
			}
			write("<table width=\"100%\" border=0 cellpadding=0 cellspacing=0 class=\"");
			if (i==a){write("actv");}else{write("dactv");}
			write("\">");
			write("<tr><td class=\"w1h7\"></td></tr>");
			write("<tr><td class=\"mainbar_");
			if (i==a){write("actv");}else{write("dactv");}
			if (i!=a && u[i] != ""){
				write('" id="tab'+i);
			}
			write("\" nowrap>");
			if (i != a && u[i] != "") {
				write("<a href=\"./"+u[i]+"_indx.htm\" target=\"_self\"");
				if (top.hide) { /* help : false */
					write(" onClick=\"top.hide.UpdateFromSlave()\"");
				}
				write(">");
			}
			if (b[i] == "&nbsp;") {
				for(j=0;j<14;j++) { b[i]+="&nbsp;"; }
			}
			write("&nbsp;&nbsp;"+b[i]+"&nbsp;&nbsp;");
			if (i != a && u[i] != "") {write("</a>");}
			write("</td></tr>");
			write("<tr><td class=\"w1h6\"></td></tr>");
			write("</table>");
			if (i!=a && u[i] != ""){
				write("</span>");
			}
			write("</td>");
		}
		for (i = b.length; i < 5; i++) {
			write("<td width=120>");
			write("<table width=\"100%\" border=0 cellpadding=0 cellspacing=0 class=\"dactv\">");
			write("<tr><td class=\"w1h7\"></td></tr>");
			write("<tr><td class=\"mainbar_dactv\">&nbsp;</td></tr>");
			write("<tr><td class=\"w1h6\"></td></tr>");
			write("</table>");
			write("</td>");
		}
		write("<td class=\"border_r\">");
		write("<table width=\"100%\" border=0 cellpadding=0 cellspacing=0 class=\"dactv\">");
		write("<tr><td class=\"w200h7\"></td></tr>");
		write("<tr><td class=\"mainbar_dactvr\">&nbsp;</td></tr>");
		write("<tr><td class=\"w200h6\"></td></tr>");
		write("</table>");
	}
}
function HelpWindow(id , sub)
{
	var locpath = location.pathname;
	var argument = locpath.split("/");
	var dir = argument[argument.length-2];
	window.open("../../help/help_"+dir+"_indx.htm?id="+id+"&sub="+sub,"");
}
function create_helpselect(n, a)
{
	var top = 0;
	var flag = 0;
	var end = 0;

	for(i = 0; i < title.length; i+=colnum) {
		if (title[i] == "_menu") top = i+colnum;
		if (title[i] == n) flag = i;
		if (title[i] == "___" && flag) {
			end = i;
			break;
		}
	}

	with(document) {
		for (i = top; i < end; i+=colnum) {
			write("[");
			write("<a href=./");
			write(get_menuid(i/colnum));
			write("_help.htm>");
			write(get_menutitle(i/colnum));
			write("]");
			write("</a>&nbsp;&nbsp;");
		}
	}
}
function urlchg(elem)
{
	window.location.href = "./"+elem.value+"_help.htm";
	window.parent.h_id = elem.value;
}
function disptips(str)
{
	obj = window.parent.document.getElementById("tips");
	obj.innerHTML = str;
}
function awk85_br()
{
	var uName = navigator.userAgent.toUpperCase();
	if (uName.indexOf("APPLEWEBKIT") >= 0) {
		tmp = eval(uName.split('APPLEWEBKIT/')[1].split('.')[0]);
		if (tmp < 100) {
			document.write("<br>");
		}
	}
}
function awk522_jump()
{
	var uName = navigator.userAgent.toUpperCase();
	if (uName.indexOf("APPLEWEBKIT") >= 0) {
		if (location.href.split("#").length == 2) {
			fragment = location.href.split("#")[1];
			location.href = "#top";
			location.href = "#" + fragment;
		}
	}
}
function getBrowserName()
{
	var aName  = navigator.appName.toUpperCase();
	var uName = navigator.userAgent.toUpperCase();
	if (uName.indexOf("APPLEWEBKIT") >= 0)  return "Safari";
	if (uName.indexOf("OPERA") >= 0)  return "Opera";
	if (aName.indexOf("NETSCAPE") >= 0)  return "Netscape";
	if (aName.indexOf("MICROSOFT") >= 0) return "MSIE";
	return "";
}
