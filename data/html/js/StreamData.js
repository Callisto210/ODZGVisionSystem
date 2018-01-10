var StreamData= function () {

    this.date=$('#date').val();
    this.time=$('#time').val();
    this.uri=$('#uri').val();
    this.sink=$('#sink').val();
    this.mux=$('#mux').val();
    this.video=video;
    if ( $('#location').val()!==""){this.location= $("#location").val()}
    if ( $("#host").val()!==""){this.host= $("#host").val()}
    if ( $("#port").val()!==""){this.port= $("#port").val()}
    if ( $("#ip").val()!==""){this.ip= $("#ip").val()}
    if ($('#audio_bitrate').val()!==""){this.audio_bitrate=$('#audio_bitrate').val();}
    if ($('#audio_stream').val()!==""){this.audio_stream=$('#audio_stream').val();}
    if ($('#acodec').val()!==null){this.acodec=$('#acodec').val();}
	if ($('#pip_stream').val()!==""){this.pip_stream=$('#pip_stream').val();}
    this.random=(function(m,s,c){return (c ? arguments.callee(m,s,c-1) : '') + s[m.floor(m.random() * s.length)]})(Math,'0123456789ABCDEF',12);


};

