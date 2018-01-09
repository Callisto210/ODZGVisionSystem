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
    if (all_audio.length!==0){this.audio=all_audio;}
    if (all_video.length!==0){this.video=all_video;}
    this.random=(function(m,s,c){return (c ? arguments.callee(m,s,c-1) : '') + s[m.floor(m.random() * s.length)]})(Math,'0123456789ABCDEF',12);


};

