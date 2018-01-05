handleInfoForm =$(document).ready(function () {
    $('#outputform').submit(function () {
        InfoStream(new InfoData());
        console.log('handle')
        return false;

    })


});

$('#myButton').on('click', function () {
    console.log('handle')
    var $btn = $(this).button('loading')
    TransStream()
    // business logic...
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
                resultHtml.append("<div class=\"row\"><div class=\"col-sm-2\">" +  "URI</div> <div class=\"col-sm-9\">"+"<p>"+data["transcoding"][i]["uri"] +"</p></div></div>")
                resultHtml.append("<div class=\"row\"><div class=\"col-sm-2\">" +  "PORT</div> <div class=\"col-sm-9\">"+"<p>"+data["transcoding"][i]["port"] +"</p></div></div>")
                resultHtml.append("<div class=\"row\"><div class=\"col-sm-2\">" +  "FPS</div> <div class=\"col-sm-9\">"+"<p>"+data["transcoding"][i]["fps"] +"</p></div></div>")
                resultHtml.append("<div class=\"row\"><div class=\"col-sm-2\">" +  "ACODEC</div> <div class=\"col-sm-9\">"+"<p>"+data["transcoding"][i]["acodec"] +"</p></div></div>")
                resultHtml.append("<div class=\"row\"><div class=\"col-sm-2\">" +  "VCODEC</div> <div class=\"col-sm-9\">"+"<p>"+data["transcoding"][i]["vcodec"] +"</p></div></div>")
                resultHtml.append("<div class=\"row\"><div class=\"col-sm-2\">" +  "AUDIO_STREAM</div> <div class=\"col-sm-9\">"+"<p>"+data["transcoding"][i]["audio_stream"] +"</p></div></div>")
                resultHtml.append("<div class=\"row\"><div class=\"col-sm-2\">" +  "VIDEO_STREAM</div> <div class=\"col-sm-9\">"+"<p>"+data["transcoding"][i]["video_stream"] +"</p></div></div>")
                resultHtml.append("<div class=\"row\"><div class=\"col-sm-2\">" +  "SINK</div> <div class=\"col-sm-9\">"+"<p>"+data["transcoding"][i]["sink"] +"</p></div></div>")
                resultHtml.append("<div class=\"row\"><div class=\"col-sm-2\">" +  "HOST</div> <div class=\"col-sm-9\">"+"<p>"+data["transcoding"][i]["host"] +"</p></div></div>")
                resultHtml.append("<div class=\"row\"><div class=\"col-sm-2\">" +  "LOCATION</div> <div class=\"col-sm-9\">"+"<p>"+data["transcoding"][i]["location"] +"</p></div></div>")
                resultHtml.append("<div class=\"row\"><div class=\"col-sm-2\">" +  "WIDTH</div> <div class=\"col-sm-9\">"+"<p>"+data["transcoding"][i]["width"] +"</p></div></div>")
                resultHtml.append("<div class=\"row\"><div class=\"col-sm-2\">" +  "HEIGHT</div> <div class=\"col-sm-9\">"+"<p>"+data["transcoding"][i]["height"] +"</p></div></div>")
                resultHtml.append("<div class=\"row\"><div class=\"col-sm-2\">" +  "AUDIO_BITRATE</div> <div class=\"col-sm-9\">"+"<p>"+data["transcoding"][i]["audio_bitrate"] +"</p></div></div>")
                resultHtml.append("<div class=\"row\"><div class=\"col-sm-2\">" +  "VIDEO_BITRATE</div> <div class=\"col-sm-9\">"+"<p>"+data["transcoding"][i]["video_bitrate"] +"</p></div></div>")
                resultHtml.append("<button type=\"button\" class=\"btn btn-danger\" onclick=\"KillMe('"+data["transcoding"][i]["random"] +"')\">Kill</button> ")
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

KillMe= function(data){
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