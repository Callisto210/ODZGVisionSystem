handleInfoForm =$(document).ready(function () {
    $('#outputform').submit(function () {
        InfoStream(new InfoData());
        console.log('information')
        return false;

    })


});

$('#myButton').on('click', function () {
    console.log('refresh')
    var $btn = $(this).button('loading')
    TransStream()
    $btn.button('reset')

});
$('#transcoding').on('show.bs.modal', function () {
    TransStream()
});
TransStream = function () {
    console.log('ajax');
    $.ajax({
        url: 'http://' + window.location.hostname + ':8090' + '/now_transcoding',
        dataType: 'json',
        type: 'get',
        contentType: "application/json",
        data: JSON.stringify(InfoData),
        success: function( data, textStatus, jQxhr ){
            console.log(data)
            $( "#accordion1" ).html( "" );
            if(data["transcoding"].length == 0){
                $( "#accordion1" ).html( "" );
                $( "#accordion1" ).append( "<div> <p> No transcoding in progress</p></div>" );
            }
            for (i = 0; i < data["transcoding"].length; i++) {
                var resultHtmls = $("<div class=\"panel panel-default\"><div class=\"panel-heading\"><h5 class=\"panel-title\"><a data-toggle=\"collapse\" href=\"#trans"+i+"\">Transcoding: "+ data["transcoding"][i]["random"] +"</a></h5></div>");
                var resultHtml=  $("<div id=\"trans"+i+"\"class=\"panel-collapse collapse\">");
                if(data["transcoding"][i].hasOwnProperty("state")) {
                    resultHtml.append("<div class=\"row\"><div class=\"col-sm-3\">" + "STATE</div> <div class=\"col-sm-9\">" +  data["transcoding"][i]["state"] + "</div></div>")
                }
                if(data["transcoding"][i].hasOwnProperty("uri")) {
                    resultHtml.append("<div class=\"row\"><div class=\"col-sm-3\">" + "URI</div> <div class=\"col-sm-9\">" +  data["transcoding"][i]["uri"] + "</div></div>")
                }
                if(data["transcoding"][i].hasOwnProperty("port")) {
                    resultHtml.append("<div class=\"row\"><div class=\"col-sm-3\">" + "PORT</div> <div class=\"col-sm-9\">" +  data["transcoding"][i]["port"] + "</div></div>")
                }
                if(data["transcoding"][i].hasOwnProperty("acodec")) {
                    resultHtml.append("<div class=\"row\"><div class=\"col-sm-3\">" + "ACODEC</div> <div class=\"col-sm-9\">" +  data["transcoding"][i]["acodec"] + "</div></div>")
                }
                if(data["transcoding"][i].hasOwnProperty("vcodec")) {
                    resultHtml.append("<div class=\"row\"><div class=\"col-sm-3\">" + "VCODEC</div> <div class=\"col-sm-9\">" + data["transcoding"][i]["vcodec"] + "</div></div>")
                }
                if(data["transcoding"][i].hasOwnProperty("host")) {
                    resultHtml.append("<div class=\"row\"><div class=\"col-sm-3\">" + "HOST</div> <div class=\"col-sm-9\">" + data["transcoding"][i]["host"] + "</div></div>")
                }
                if(data["transcoding"][i].hasOwnProperty("location")) {
                    resultHtml.append("<div class=\"row\"><div class=\"col-sm-3\">" + "LOCATION</div> <div class=\"col-sm-9\">" +  data["transcoding"][i]["location"] + "</div></div>")
                }
                if(data["transcoding"][i].hasOwnProperty("width")) {
                    resultHtml.append("<div class=\"row\"><div class=\"col-sm-3\">" + "WIDTH</div> <div class=\"col-sm-9\">" + data["transcoding"][i]["width"] + "</div></div>")
                }
                if(data["transcoding"][i].hasOwnProperty("height")) {
                    resultHtml.append("<div class=\"row\"><div class=\"col-sm-3\">" + "HEIGHT</div> <div class=\"col-sm-9\">" +  data["transcoding"][i]["height"] + "</div></div>")
                }
                if(data["transcoding"][i].hasOwnProperty("audio_bitrate")) {
                    resultHtml.append("<div class=\"row\"><div class=\"col-sm-3\">" + "AUDIO_BITRATE</div> <div class=\"col-sm-9\">" +  data["transcoding"][i]["audio_bitrate"] + "</div></div>")
                }
                if(data["transcoding"][i].hasOwnProperty("video_bitrate")) {
                    resultHtml.append("<div class=\"row\"><div class=\"col-sm-3\">" + "VIDEO_BITRATE</div> <div class=\"col-sm-9\">" +  data["transcoding"][i]["video_bitrate"] + "</div></div>")
                }
                resultHtml.append("<button type=\"button\" class=\"btn btn-danger\" onclick=\"CancelMe('"+data["transcoding"][i]["random"] +"')\">Cancel</button> ")
                resultHtml.append("</div>")
                resultHtmls.append(resultHtml)
                resultHtmls.append("</div>")
                $( "#accordion1" ).append( resultHtmls );
            }

            console.log(resultHtml);


        },
        error: function( jqXhr, textStatus, errorThrown ){
            $( "#accordion1" ).html( "" );
            $( "#accordion1" ).append( "<div> <p>" + errorThrown +"</p></div>" );
        }
    })

};

CancelMe= function(data){
    var s = {random: data}
    console.log(s)
    $.ajax({
        url: 'http://' + window.location.hostname + ':8090' + '/kill',
        dataType: 'json',
        type: 'post',
        contentType: "application/json",
        data: JSON.stringify(s),
        success: function( data, textStatus, jQxhr ){
            TransStream()
        },
        error: function( jqXhr, textStatus, errorThrown ){
            alert(textStatus)
        }
    })
};