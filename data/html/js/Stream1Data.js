var Stream1Data= function () {
    this.source=$('#source').val();
    this.path=$('#file').val();
    this.acodec=$('#audioCodec').val();
    this.vcodec=$('#videoCodec').val();
    this.fps=parseInt($('#fps').val(), 10);
    this.bitrate=parseInt($('#bitRate').val(), 10);
    this.width=parseInt($('#width').val(), 10);
    this.height=parseInt($('#height').val(), 10);
};