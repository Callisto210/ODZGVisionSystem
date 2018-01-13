InputInfo = function (str) {
    console.log('ajax');
    datas = {"uri" : str}
    $.ajax({
        url: 'http://' + window.location.hostname + ':8090' + '/info',
        dataType: 'json',
        type: 'post',
        contentType: "application/json",
        data: JSON.stringify(datas),
        success: function( data, textStatus, jQxhr ){
            console.log(data)
            $( "#audio_stream" ).html( "" );
            for (i = 0; i < data["audio"].length; i++) {
                resultHtmls =(("<option value=\""+data["audio"][i]["streamid"]+"\">" +data["audio"][i]["streamid"]+"</option>"))
                $( "#audio_stream" ).append( resultHtmls );
            }
            for (i = 0; i < data["video"].length; i++) {
                resultHtmls =(("<option value=\""+data["video"][i]["streamid"]+"\">" +data["video"][i]["streamid"]+"</option>"))
                $( "#video_stream" ).append( resultHtmls );
            }



        },
        error: function( jqXhr, textStatus, errorThrown ){
            $( "#audio_stream" ).html( "" );
            $( "#video_stream" ).html( "" );

        }
    })

}

$( "#uri" ).change(function() {
    if($("#uri").val().trim().length !=0){
        InputInfo($("#uri").val())
    }
});
