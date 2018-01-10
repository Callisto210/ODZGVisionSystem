var VideoStream= function () {
   if ($('#video_bitrate').val()!==""){this.video_bitrate=$('#video_bitrate').val();}
    if ($('#video_stream').val()!==""){this.video_stream=$('#video_stream').val();}
    if ($('#vcodec').val()!==null){this.vcodec=$('#vcodec').val();}
    if ($('#fps').val()!==""){this.fps=$('#fps').val();}
    if($('#width').val()!==""){ this.width=$('#width').val();}
    if ($('#height').val()!==""){this.height=$('#height').val();}
    if ($('#pip_stream').val()!==""){this.pip_stream=$('#pip_stream').val();}
    cleanVideo();
}

function cleanVideo(){
$('#height').val("");
$('#width').val("");
$('#fps').val("");
$('#vcodec').val(null);
$('#video_stream').val("");
$('#video_bitrate').val("");
$('#pip_stream').val("");
}