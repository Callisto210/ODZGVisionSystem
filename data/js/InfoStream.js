InfoStream = function (InfoData) {
    console.log('ajax');

    console.log(InfoData);

    $.post('http://' + window.location.hostname + ':8090' + '/info',
        JSON.stringify(InfoData),
        function (data, status) {
            $( "#accordion" ).html( "" );
            for (i = 0; i < data["audio"].length; i++) {
                var resultHtmls = $("<div class=\"panel panel-default\"><div class=\"panel-heading\"><h5 class=\"panel-title\"><a data-toggle=\"collapse\" href=\"#audio"+i+"\">Audio "+ i +"</a></h5></div>");
                var resultHtml=  $("<div id=\"audio"+i+"\"class=\"panel-collapse collapse\">");
                //var row = $("<div class=\"row\">");
                resultHtml.append("<div class=\"row\"><div class=\"col-sm-2\">" +  "AUDIO</div> <div class=\"col-sm-9\">"+"<p>"+data["audio"][i]["audio"] +"</p></div></div>")
                resultHtml.append("<div class=\"row\"><div class=\"col-sm-2\">" +  "STREAMID</div> <div class=\"col-sm-9\">"+"<p>"+data["audio"][i]["streamid"] +"</p></div></div>")
                resultHtml.append("<div class=\"row\"><div class=\"col-sm-2\">" +  "BITRATE</div> <div class=\"col-sm-9\">"+"<p>"+data["audio"][i]["bitrate"] +"</p></div></div>")
                resultHtml.append("<div class=\"row\"><div class=\"col-sm-2\">" +  "DEPTH</div> <div class=\"col-sm-9\">"+"<p>"+data["audio"][i]["depth"] +"</p></div></div>")
                resultHtml.append("<div class=\"row\"><div class=\"col-sm-2\">" +  "SAMPLE_RATE</div> <div class=\"col-sm-9\">"+"<p>"+data["audio"][i]["sample_rate"] +"</p></div></div>")
                resultHtml.append("</div>")
                resultHtmls.append(resultHtml)
                resultHtmls.append("</div>")
                $( "#accordion" ).append( resultHtmls );
            }
            for (i = 0; i < data["video"].length; i++) {
                var resultHtmls = $("<div class=\"panel panel-default\"><div class=\"panel-heading\"><h5 class=\"panel-title\"><a data-toggle=\"collapse\" href=\"#video"+i+"\">Video "+ i +"</a></h5></div>");
                var resultHtml=  $("<div id=\"video"+i+"\"class=\"panel-collapse collapse\">");
                //var row = $("<div class=\"row\">");
                resultHtml.append("<div class=\"row\"><div class=\"col-sm-2\">" +  "VIDEO</div> <div class=\"col-sm-9\">"+"<p>"+data["video"][i]["video"] +"</p></div></div>")
                resultHtml.append("<div class=\"row\"><div class=\"col-sm-2\">" +  "STREAMID</div> <div class=\"col-sm-9\">"+"<p>"+data["video"][i]["streamid"] +"</p></div></div>")
                resultHtml.append("<div class=\"row\"><div class=\"col-sm-2\">" +  "BITRATE</div> <div class=\"col-sm-9\">"+"<p>"+data["video"][i]["bitrate"] +"</p></div></div>")
                resultHtml.append("<div class=\"row\"><div class=\"col-sm-2\">" +  "DEPTH</div> <div class=\"col-sm-9\">"+"<p>"+data["video"][i]["depth"] +"</p></div></div>")
                resultHtml.append("<div class=\"row\"><div class=\"col-sm-2\">" +  "HEIGHT</div> <div class=\"col-sm-9\">"+"<p>"+data["video"][i]["height"] +"</p></div></div>")
                resultHtml.append("<div class=\"row\"><div class=\"col-sm-2\">" +  "WIDTH</div> <div class=\"col-sm-9\">"+"<p>"+data["video"][i]["witdh"] +"</p></div></div>")
                resultHtml.append("</div>")
                resultHtmls.append(resultHtml)
                resultHtmls.append("</div>")
                $( "#accordion" ).append( resultHtmls );
            }

            console.log(resultHtml);


        },"json");
}