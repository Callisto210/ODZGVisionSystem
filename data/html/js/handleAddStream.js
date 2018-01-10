all_audio = []
all_video = []

handleCodec =$(document).ready(function () {
    $('#add_stream').on('click', function(){($('#modal_add_stream').modal())
})});


handleAudio =$(document).ready(function () {
    $('#add_stream1').on('click', function(){
        if(!jQuery.isEmptyObject(new audio_data())){
            all_audio.push(new audio_data())
            claeraudio()
            requiredChange()
            showAudio()
        }
    })});

handleShowAudio =$(document).ready(function () {
    $('#show_stream1').on('click', function(){

    })});


handleVideo =$(document).ready(function () {
    $('#add_stream2').on('click', function(){
        if (!jQuery.isEmptyObject(new video_data())){
            all_video.push(new video_data())
            clearvideo()
            requiredChange()
            showVideo()
        }
    })});

claeraudio = function () {

        $("#acodec").val('');
        $("#audio_bitrate").val('');
        $("#audio_stream").val('');
}
clearvideo = function () {
    $("#video_bitrate").val('');
    $("#vcodec").val('');
    $("#video_stream").val('');
    $("#height").val('');
    $("#width").val('');
    if(!$('#pip_check').prop("checked"))
        $("#pip_stream").val("");
}


audio_data = function () {
    if ($('#audio_bitrate').val()!==""){this.audio_bitrate=$('#audio_bitrate').val();}
    if ($('#audio_stream').val()!==""){this.audio_stream=$('#audio_stream').val();}
    if ($('#acodec').val()!==null){this.acodec=$('#acodec').val();}
}

video_data = function () {
    if ($('#video_bitrate').val()!==""){this.video_bitrate=$('#video_bitrate').val();}
    if ($('#video_stream').val()!==""){this.video_stream=$('#video_stream').val();}
    if ($('#vcodec').val()!==null){this.vcodec=$('#vcodec').val();}
    if ($('#fps').val()!==""){this.fps=$('#fps').val();}
    if($('#width').val()!==""){ this.width=$('#width').val();}
    if ($('#height').val()!==""){this.height=$('#height').val();}
}




