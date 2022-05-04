import 'dart:io';
import 'dart:async';
import 'dart:typed_data';
import 'package:flutter/material.dart';
import 'package:get/get.dart';
import 'package:photo_manager/photo_manager.dart';
import 'package:video_player/video_player.dart';
import 'package:papy/refer/const.dart';
import 'package:papy/refer/translates.dart';

class PictureList extends StatefulWidget {
  const PictureList({Key? key, required this.onSelectImage}) : super(key: key);
  final Function onSelectImage;

  @override
  _PictureListState createState() => _PictureListState();
}

class _PictureListState extends State<PictureList> {
  List<AssetEntity> assets = [];

  @override
  void initState() {
    _fetchAssets();
    super.initState();
  }

  _fetchAssets() async {
    final albums = await PhotoManager.getAssetPathList(onlyAll: true);
    final recentAlbum = albums.first;
    final recentAssets =
            await recentAlbum.getAssetListRange(start: 0, end: 1000000);
    setState(() => assets = recentAssets);
  }

  @override
  Widget build(BuildContext context) {
    return Scaffold(
      appBar: AppBar(
        title: Text(Localization.of(context)!.tr('selectPicture')!)
      ),
      body: GridView.builder(
        gridDelegate: const SliverGridDelegateWithFixedCrossAxisCount(
          crossAxisCount: 3
        ),
        itemCount: assets.length,
        itemBuilder: (_, index) {
          return FutureBuilder<Uint8List?>(
            future: assets[index].thumbnailData,
            builder: (_, shapshot) {
              var bytes = shapshot.data;
              if (bytes == null)  return const CircularProgressIndicator();
              return InkWell(
                //onTap: () => _onTap(index, bytes),
                onTap: () => assets[index].type == AssetType.image
                  ? _selectImage(index)
                  : Get.to(() => _VideoView(videoFile: assets[index].file)),
                onDoubleTap: () => {},
                child: Stack(
                  children: [
                    Positioned.fill(
                      child: Image.memory(bytes, fit: BoxFit.cover,),
                    ),
                    if (assets[index].type == AssetType.video)
                      Center(
                        child: Container(
                          color: Environs.defaultColor,
                          child: const Icon(
                            Icons.play_arrow, color: Colors.white,
                          ),
                        ),
                      ),
                  ],
                ),
              );
            },
          );
        },
      ),
    );
  }

  _selectImage(int index) {
    final Future file = assets[index].file;
    file.then((imageFile) {
      print('DBG>picture_list>path=$imageFile path=${imageFile.path}');
      if (imageFile.path != null) {
        widget.onSelectImage(imageFile.path);
      }
      Get.back();
    }).catchError((e) { print('picture select error:$e');});
    //Get.to(() => _ImageView(imageFile: assets[index].file));
  }
}

class _ImageView extends StatelessWidget {
  const _ImageView({Key? key, required this.imageFile}) : super(key: key);

  final Future<File?> imageFile;

  @override
  Widget build(BuildContext context) {
    return Scaffold(
      appBar: AppBar(
          title: Text(Localization.of(context)!.tr('selectedPicture')!)
      ),
      body: Container(
        color: Colors.transparent,
        alignment: Alignment.center,
        child: FutureBuilder<File?>(
          future: imageFile,
          builder: (_, snapshot) {
            final file = snapshot.data;
            if (file == null) return Container();
            print('DBG>select file ${file.path}');
            return Image.file(file);
          },
        )
      ),
    );
  }
}

class ImageView extends StatelessWidget {
  const ImageView({Key? key, required this.file}) : super(key: key);

  final File file;

  @override
  Widget build(BuildContext context) {
    return Scaffold(
      appBar: AppBar(
          title: Text(Localization.of(context)!.tr('selectedPicture')!)
      ),
      body: Center(
        child: Column(
          mainAxisAlignment: MainAxisAlignment.center,
          children: [
            Image.file(
              file,
              width: 400,
              height: 400,
              fit: BoxFit.fill,
            ),
          ]
        ),
      ),
    );
  }
}

class _VideoView extends StatefulWidget {
  const _VideoView({Key? key, required this.videoFile}) : super(key: key);

  final Future<File?> videoFile;

  @override
  _VideoViewState createState() => _VideoViewState();
}

class _VideoViewState extends State<_VideoView> {
  VideoPlayerController? _controller;
  bool initialized = false;

  @override
  void initState() {
    _initVideo();
    super.initState();
  }

  @override
  void dispose() {
    if (_controller != null) _controller!.dispose();
    super.dispose();
  }

  _initVideo() async {
    final video = await widget.videoFile;
    _controller = VideoPlayerController.file(video!)
      ..setLooping(true)
      ..initialize().then((_) => setState(() => initialized = true));
  }

  @override
  Widget build(BuildContext context) {
    return Scaffold(
      body: initialized
      ? Scaffold(
        body: Center(
          child: AspectRatio(
            aspectRatio: _controller!.value.aspectRatio,
            child: VideoPlayer(_controller!),
          ),
        ),
        floatingActionButton: FloatingActionButton(
          onPressed: () {
            setState(() {
              if (_controller!.value.isPlaying) {
                _controller!.pause();
              } else {
                _controller!.play();
              }
            });
          },
          child: Icon(
            _controller!.value.isPlaying ? Icons.pause : Icons.play_arrow
          ),
        ),
      )
      : const Center(
        child: CircularProgressIndicator(),
      ),
    );
  }
}