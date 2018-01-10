var AudioStream= function () {
   if ($('#audio_bitrate').val()!==""){this.audio_bitrate=$('#audio_bitrate').val();}
    if ($('#audio_stream').val()!==""){this.audio_stream=$('#audio_stream').val();}
    if ($('#acodec').val()!==null){this.acodec=$('#acodec').val();}
    cleanAudio();
}

function cleanAudio(){
$('#acodec').val(null);
$('#audio_stream').val("");
$('#audio_bitrate').val("");
}