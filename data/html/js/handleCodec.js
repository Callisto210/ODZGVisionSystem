handleCodec =$(document).ready(function () {

    // $('#vcodec').on('change', function() {
    //     requiredChange()
    // });
    //
    //
    // $('#acodec').on('change', function() {
    //     requiredChange()
    // })
    $('#video_check').on('change', function() {
        requiredChange();
showVideo();
        console.log("chcec")
    });


    $('#audio_check').on('change', function() {
        requiredChange();
    showAudio()    })
})

function requiredChange(){
    clear();
    if( $('#video_check').prop( "checked" )){
        if(all_video.length==0)$("#vcodec").prop('required',true);
        $("#audio_check").prop('required',false);


    }
    if($('#audio_check').prop( "checked" )){
        if(all_audio.length==0)$("#acodec").prop('required',true);
        $("#video_check").prop('required',false);

    }
    if (!($('#audio_check').prop( "checked" )||$('#video_check').prop( "checked" ))){
        if(all_audio.length==0 && all_video.length==0) {
            $("#video_check").prop('required', true)
            $("#audio_check").prop('required', true)
        }

    }
}

function clear() {
    if( !$('#audio_check').prop( "checked" )){
        $("#acodec").val('');
        $("#audio_bitrate").val('');
        $("#audio_stream").val('');


    }
    if( !$('#video_check').prop( "checked" )){
        $("#video_bitrate").val('');
        $("#vcodec").val('');
        $("#video_stream").val('');
        $("#height").val('');
        $("#width").val('');
        if(!$('#pip_check').prop("checked"))
        $("#pip_stream").val("");

    }
    $("#video_check").prop('required',true)
    $("#audio_check").prop('required',true)
    $("#acodec").prop('required',false);
    $("#vcodec").prop('required',false);
    $("#pip_check").prop('required',false);

}

function showVideo() {
if($('#video_check').prop( "checked" )){

    $('#video_div').show()

}else {
    $('#video_div').hide()

}
}
function showAudio() {
if($('#audio_check').prop( "checked" )){

    $('#audio_div').show()
}else {
    $('#audio_div').hide()

}
}

