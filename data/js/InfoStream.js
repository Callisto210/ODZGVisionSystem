InfoStream = function (InfoData) {
    console.log('ajax');

    console.log(InfoData);

    $.post('http://' + window.location.hostname + ':8090/info',
        JSON.stringify(InfoData),
        function (data, status) {
            //var resultHtml = $("<div class=\"container-fluid\">");
            var resultHtml = $("<div class=\"row\">")
            //var result = JSON.stringify(data)
            for (i = 0; i < data["audio"].length; i++) {

                resultHtml.append("<div class=\"col-sm-2\"" +"\">" +  " AUDIO </div>")
                resultHtml.append("<div class=\"col-sm-9\"" +"\">" + "<p>" +data["audio"][i]["audio"] + "</p></div>")
                resultHtml.append("<div class=\"col-sm-2\"" +"\">" +  " STREAMID </div>")
                resultHtml.append("<div class=\"col-sm-9\"" +"\">" + "<p>" +data["audio"][i]["streamid"] + "</p></div>")
                resultHtml.append("<div class=\"col-sm-2\"" +"\">" +  " BITRATE </div>")
                resultHtml.append("<div class=\"col-sm-9\"" +"\">" + "<p>" +data["audio"][i]["bitrate"] + "</p></div>")
                resultHtml.append("<div class=\"col-sm-2\"" +"\">" +  " DEPTH </div>")
                resultHtml.append("<div class=\"col-sm-9\"" +"\">" + "<p>" +data["audio"][i]["depth"] + "</p></div>")
                resultHtml.append("<div class=\"col-sm-2\"" +"\">" +  " SAMPLE_RATE </div>")
                resultHtml.append("<div class=\"col-sm-9\"" +"\">" + "<p>" +data["audio"][i]["sample_rate"] + "</p></div>")
            }
            resultHtml.append("</div>");
            resultHtml.append("<div class=\"row\">")
            for (i = 0; i < data["video"].length; i++) {

                resultHtml.append("<div class=\"col-sm-2\"" +"\">" +  " VIDEO </div>")
                resultHtml.append("<div class=\"col-sm-9\"" +"\">" + "<p>" +data["video"][i]["video"] + "</p></div>")
                resultHtml.append("<div class=\"col-sm-2\"" +"\">" +  " STREAMID </div>")
                resultHtml.append("<div class=\"col-sm-9\"" +"\">" + "<p>" +data["video"][i]["streamid"] + "</p></div>")
                resultHtml.append("<div class=\"col-sm-2\"" +"\">" +  " BITRATE </div>")
                resultHtml.append("<div class=\"col-sm-9\"" +"\">" + "<p>" +data["video"][i]["bitrate"] + "</p></div>")
                resultHtml.append("<div class=\"col-sm-2\"" +"\">" +  " DEPTH </div>")
                resultHtml.append("<div class=\"col-sm-9\"" +"\">" + "<p>" +data["video"][i]["depth"] + "</p></div>")
                resultHtml.append("<div class=\"col-sm-2\"" +"\">" +  " HEIGHT </div>")
                resultHtml.append("<div class=\"col-sm-9\"" +"\">" + "<p>" +data["video"][i]["height"] + "</p></div>")
                resultHtml.append("<div class=\"col-sm-2\"" +"\">" +  " WIDTH </div>")
                resultHtml.append("<div class=\"col-sm-9\"" +"\">" + "<p>" +data["video"][i]["witdh"] + "</p></div>")
            }
            resultHtml.append("</div>");
            //resultHtml.append("</div>");
            $("#message").html(resultHtml);
            
        },"json");
}