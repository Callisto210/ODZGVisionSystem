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
$('#pip_check').change( function() {
    showpip()
    console.log("pip")
});

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
        $("#audio_stream").prop('selectedIndex',1);


    }
    if( !$('#video_check').prop( "checked" )){
        $("#video_bitrate").val('');
        $("#vcodec").val('');
        $("#video_stream").prop('selectedIndex',1);
        $("#height").val('');
        $("#width").val('');
        if(!$('#pip_check').prop("checked")) {
            $("#pip_stream").prop("selectedIndex",1)
            $("#pip_y").val("");
            $("#pip_x").val("");
            $("#pip_width").val("");
            $("#pip_height").val("");
        }

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
function showpip() {
    if($('#pip_check').prop( "checked" )){

        $('#pip_all').show();
        $("#pip_stream").prop('required',true);
        $("#pip_height").prop('required',true);
        $("#pip_width").prop('required',true);
        $("#pip_x").prop('required',true);
        $("#pip_y").prop('required',true);
        $("#pip_stream").prop('selectedIndex',0);

    }else {
        $('#pip_all').hide();
        $("#pip_stream").prop('required',false);
        $("#pip_height").prop('required',false);
        $("#pip_width").prop('required',false);
        $("#pip_x").prop('required',false);
        $("#pip_y").prop('required',false);
        $("#pip_stream").prop('selectedIndex',1);
        $("#video_stream  option").each(function () { $(this).prop('disabled',false) });

    }
}