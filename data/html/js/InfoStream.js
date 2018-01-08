InfoStream = function (InfoData) {
    console.log('ajax');
    $.ajax({
        url: 'http://' + window.location.hostname + ':8090' + '/info',
        dataType: 'json',
        type: 'post',
        contentType: "application/json",
        data: JSON.stringify(InfoData),
        success: function( data, textStatus, jQxhr ){
            console.log(data)
            $( "#accordion" ).html( "" );
            for (i = 0; i < data["audio"].length; i++) {
                var resultHtmls = $("<div class=\"panel panel-default\"><div class=\"panel-heading\"><h5 class=\"panel-title\"><a data-toggle=\"collapse\" href=\"#audio"+i+"\">Audio "+ i +"</a></h5></div>");
                var resultHtml=  $("<div id=\"audio"+i+"\"class=\"panel-collapse collapse\">");
                //var row = $("<div class=\"row\">");
                resultHtml.append("<div class=\"row\"><div class=\"col-sm-2\">" +  "AUDIO</div> <div class=\"col-sm-9\">"+data["audio"][i]["audio"] +"</div></div>")
                resultHtml.append("<div class=\"row\"><div class=\"col-sm-2\">" +  "STREAMID</div> <div class=\"col-sm-9\">"+data["audio"][i]["streamid"] +"</div></div>")
                resultHtml.append("<div class=\"row\"><div class=\"col-sm-2\">" +  "BITRATE</div> <div class=\"col-sm-9\">"+data["audio"][i]["bitrate"] +"</div></div>")
                resultHtml.append("<div class=\"row\"><div class=\"col-sm-2\">" +  "DEPTH</div> <div class=\"col-sm-9\">"+data["audio"][i]["depth"] +"</div></div>")
                resultHtml.append("<div class=\"row\"><div class=\"col-sm-2\">" +  "SAMPLE_RATE</div> <div class=\"col-sm-9\">"+data["audio"][i]["sample_rate"] +"</div></div>")
                resultHtml.append("<div class=\"row\"><div class=\"col-sm-2\">" +  "CHANNELS</div> <div class=\"col-sm-9\">"+data["audio"][i]["channels"] +"</div></div>")
                resultHtml.append("</div>")
                resultHtmls.append(resultHtml)
                resultHtmls.append("</div>")
                $( "#accordion" ).append( resultHtmls );
            }
            for (i = 0; i < data["video"].length; i++) {
                var resultHtmls = $("<div class=\"panel panel-default\"><div class=\"panel-heading\"><h5 class=\"panel-title\"><a data-toggle=\"collapse\" href=\"#video"+i+"\">Video "+ i +"</a></h5></div>");
                var resultHtml=  $("<div id=\"video"+i+"\"class=\"panel-collapse collapse\">");
                //var row = $("<div class=\"row\">");
                resultHtml.append("<div class=\"row\"><div class=\"col-sm-2\">" +  "VIDEO</div> <div class=\"col-sm-9\">"+data["video"][i]["video"] +"</div></div>")
                resultHtml.append("<div class=\"row\"><div class=\"col-sm-2\">" +  "STREAMID</div> <div class=\"col-sm-9\">"+data["video"][i]["streamid"] +"</div></div>")
                resultHtml.append("<div class=\"row\"><div class=\"col-sm-2\">" +  "BITRATE</div> <div class=\"col-sm-9\">"+data["video"][i]["bitrate"] +"</div></div>")
                resultHtml.append("<div class=\"row\"><div class=\"col-sm-2\">" +  "DEPTH</div> <div class=\"col-sm-9\">"+data["video"][i]["depth"] +"</div></div>")
                resultHtml.append("<div class=\"row\"><div class=\"col-sm-2\">" +  "HEIGHT</div> <div class=\"col-sm-9\">"+data["video"][i]["height"] +"</div></div>")
                resultHtml.append("<div class=\"row\"><div class=\"col-sm-2\">" +  "WIDTH</div> <div class=\"col-sm-9\">"+data["video"][i]["witdh"] +"</div></div>")
                resultHtml.append("<div class=\"row\"><div class=\"col-sm-2\">" +  "FPS</div> <div class=\"col-sm-9\">"+data["video"][i]["fps"] +"</div></div>")
                resultHtml.append("</div>")
                resultHtmls.append(resultHtml)
                resultHtmls.append("</div>")
                $( "#accordion" ).append( resultHtmls );
            }

            console.log(resultHtml);


        },
        error: function( jqXhr, textStatus, errorThrown ){
            $( "#accordion" ).html( "" );
            $( "#accordion" ).append( "<div> <p>" + errorThrown +"</p></div>" );
        }
    })

}