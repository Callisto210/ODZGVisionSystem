handleCodec =$(document).ready(function () {
    $('#video_check').on('change', function() {
        requiredChange();
        showVideo();
        console.log("chcec")
    });


    $('#audio_check').on('change', function() {
        requiredChange();
        showAudio()    })
});
$(document).on('change','#pip_check', function() {
    showpip();
    console.log("pip")
});

function requiredChange(){
    clear();
    if( $('#video_check').prop( "checked" )){
        if(all_video.length===0){
            $("#vcodec").prop('required',true);
            $('#video_stream').prop('required',true);
        }
        $("#audio_check").prop('required',false);
        $('#audio_stream').prop('required',false);
        $('#acodec').prop('required',false);


    }
    if($('#audio_check').prop( "checked" )){
        if(all_audio.length===0){
            $("#acodec").prop('required',true);
            $('#video_stream').prop('required',true);
        }
        $("#video_check").prop('required',false);
        $('#video_stream').prop('required',false);
        $('#vcodec').prop('required',false);
    }
    if (!($('#audio_check').prop( "checked" )||$('#video_check').prop( "checked" ))){
        if(all_audio.length===0 && all_video.length===0) {
            $("#video_check").prop('required', true);
            $("#audio_check").prop('required', true)
        }

    }
}

function clear() {
    if( !$('#audio_check').prop( "checked" )){
        $("#acodec").val('');
        $("#audio_bitrate").val('');
        $("#audio_stream").prop('selectedIndex',0);


    }
    if( !$('#video_check').prop( "checked" )){
        $("#video_bitrate").val('');
        $("#vcodec").val('');
        $("#video_stream").prop('selectedIndex',0);
        $("#height").val('');
        $("#width").val('');
        if(!$('#pip_check').prop("checked")) {
            $('#pip_contained').find('>  div').each(function () {
                $(this).find("select").prop("selectedIndex",0);
                $(this).find("input[name='pip_height']").val('');
                $(this).find("input[name='pip_width']").val('');
                $(this).find("input[name='pip_x']").val('');
                $(this).find("input[name='pip_y']").val('');
            })
        }

    }
    $("#video_check").prop('required',true);
    $("#audio_check").prop('required',true);
    $("#acodec").prop('required',false);
    $("#vcodec").prop('required',false);
    $("#pip_check").prop('required',false);
    $('#video_stream').prop('required',false);
    $('#audio_stream').prop('required',false);

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
        $('#pip_contained').find('> div').each(function () {
            $(this).find("select").prop("selectedIndex",0);
            $(this).find("select").prop('required',true);
            $(this).find("input").each(function (i, val) {

                val.required = true
            });
            video_change()
        })

    }else {
        $('#pip_all').hide();
        $('#pip_contained').find('> div').each(function () {
            $(this).find("select option").each(function (i,val) {

                $(val).prop('disabled', false)
            });
            $(this).find("select").prop("selectedIndex",0);
            $(this).find("select").prop('required',false);
            $(this).find("input[name='pip_height']").prop('required',false);
            $(this).find("input[name='pip_width']").prop('required',false);
            $(this).find("input[name='pip_x']").prop('required',false);
            $(this).find("input[name='pip_y']").prop('required',false);
        });
        $('#pip_contained').find('> div:not(:first)').each(function () {
            $(this).remove()
        });
        pip_i = 0;

        $("#video_stream").find("option").each(function () { $(this).prop('disabled',false) });

    }
}