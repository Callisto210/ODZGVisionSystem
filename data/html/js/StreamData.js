var StreamData= function () {

    this.date=$('#date').val();
    this.time=$('#time').val();
    this.uri=$('#uri').val();
    this.sink=$('#sink').val();
    this.mux=$('#mux').val();
    if ( $('#location').val()!==""){this.location= $("#location").val()}
    if ( $("#host").val()!==""){this.host= $("#host").val()}
    if ( $("#port").val()!==""){this.port= $("#port").val()}
    if ( $("#ip").val()!==""){this.ip= $("#ip").val()}
    if ($('#fps').val()!==""){this.fps=$('#fps').val();}
    if($('#width').val()!==""){ this.width=$('#width').val();}
    if ($('#height').val()!==""){this.height=$('#height').val();}
    if ($('#audio_bitrate').val()!==""){this.audio_bitrate=$('#audio_bitrate').val();}
    if ($('#audio_stream').val()!==""){this.audio_stream=$('#audio_stream').val();}
    if ($('#acodec').val()!==null){this.acodec=$('#acodec').val();}
    if ($('#video_bitrate').val()!==""){this.video_bitrate=$('#video_bitrate').val();}
    if ($('#video_stream').val()!==""){this.video_stream=$('#video_stream').val();}
    if ($('#vcodec').val()!==null){this.vcodec=$('#vcodec').val();}
    this.random=(function(m,s,c){return (c ? arguments.callee(m,s,c-1) : '') + s[m.floor(m.random() * s.length)]})(Math,'0123456789ABCDEF',12);


};

