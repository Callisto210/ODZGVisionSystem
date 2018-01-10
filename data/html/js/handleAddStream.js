all_audio = []
all_video = []

handleCodec =$(document).ready(function () {
    $('#add_stream').on('click', function(){($('#modal_add_stream').modal())
})});


handleAudio =$(document).ready(function () {
    $('#add_stream1').on('click', function(){
        var data = new audio_data();
        if(!jQuery.isEmptyObject(data) && data.hasOwnProperty("acodec")){
            all_audio.push(data)
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
        var data = new video_data();
        if (!jQuery.isEmptyObject(data) && data.hasOwnProperty("vcodec")){

            all_video.push(data)
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
    $("#pip_stream").val("");
    $("#pip_y").val("");
    $("#pip_x").val("");
    $("#pip_width").val("");
    $("#pip_height").val("");

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
    if($('#pip_check').prop("checked")){
        this.pip_stream=$("#pip_stream").val();
        this.pip_height=$("#pip_height").val();
        this.pip_width=$("#pip_width").val();
        this.pip_x=$("#pip_x").val();
        this.pip_y=$("#pip_y").val();
    }
}




