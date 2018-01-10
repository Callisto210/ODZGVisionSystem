        var video = [];



handleCodec =$(document).ready(function () {
    $('#add_stream').on('click', function(){addVideoToArray()
})})


function addVideoToArray(){
var videoStream = new VideoStream();
if(!jQuery.isEmptyObject(videoStream)){
video.push(videoStream);

}
console.log(video)

}