import 'dart:async';
import 'dart:io';
import 'dart:math';
import 'package:flutter/material.dart';
import 'package:flutter/foundation.dart';
import 'package:camera/camera.dart';
import 'package:video_player/video_player.dart';
import 'package:font_awesome_flutter/font_awesome_flutter.dart';

T? _ambiguate<T>(T? value) => value;

List<CameraDescription> cameras = <CameraDescription>[];

void logError(String code, String? message) {
  if (message != null) {
    print('ERROR:$code\nError Message:$message');
  } else {
    print('ERROR:$code');
  }
}

IconData getCameraLensIcon(CameraLensDirection direction) {
  switch (direction) {
    case CameraLensDirection.back:
      return Icons.camera_rear;
    case CameraLensDirection.front:
      return Icons.camera_front;
    case CameraLensDirection.external:
      return Icons.camera;
    default:
      throw ArgumentError('Unknown lens direction');
  }
}

Future<bool> igniteCamera() async {
  try {
    print('DBG>igniteCamera begin');
    WidgetsFlutterBinding.ensureInitialized();
    cameras = await availableCameras();
    print('DBG>igniteCamera end');
    return Future<bool>.value(true);
  } on CameraException catch (e) {
    logError(e.code, e.description);
    return Future<bool>.value(false);
  }
}

class Photog extends StatefulWidget {
  @override
  _Photog createState() => _Photog();
}

class _Photog extends State<Photog>
    with WidgetsBindingObserver, TickerProviderStateMixin {
  XFile? _imageFile;
  XFile? _videoFile;
  int _currentLens = 0;
  CameraController? _cameraController;
  VideoPlayerController? _videoController;
  VoidCallback? _videoPlayerListener;
  bool _enableAudio = true;
  double _minAvailableExposureOffset = 0.0;
  double _maxAvailableExposureOffset = 0.0;
  double _currentExposureOffset = 0.0;
  late AnimationController _flashAniController;
  late Animation<double> _flashAnimation;
  late AnimationController _exposureAniController;
  late Animation<double> _exposureAnimation;
  late AnimationController _focusAniController;
  late Animation<double> _focusAnimation;
  double _minAvailableZoom = 1.0;
  double _maxAvailableZoom = 1.0;
  double _currentScale = 1.0;
  double _baseScale = 1.0;
  int _pointers = 0;
  bool _initial = false;
  bool _change = false;
  late double _deviceRatio;
  late double _scale;
  late double _orgWidth;
  late double _orgHeight;
  late double _chgWidth;

  @override
  void initState() {
    super.initState();
    //T? _ambiguate<T>(T? value) => value;
    _ambiguate(WidgetsBinding.instance)?.addObserver(this);
    //(WidgetsBinding.instance)?.addObserver(this);

    _flashAniController = AnimationController(
        duration: const Duration(milliseconds: 300), vsync: this);
    _flashAnimation = CurvedAnimation(
        parent: _flashAniController, curve: Curves.easeInCubic);
    _exposureAniController = AnimationController(
        duration: const Duration(milliseconds: 300), vsync: this);
    _exposureAnimation = CurvedAnimation(
        parent: _exposureAniController, curve: Curves.easeInCubic);
    _focusAniController = AnimationController(
        duration: const Duration(milliseconds: 300), vsync: this);
    _focusAnimation = CurvedAnimation(
        parent: _focusAniController, curve: Curves.easeInCubic);

    _selectCameraConnect(cameras[_currentLens]);
  }

  @override
  void dispose() {
    _ambiguate(WidgetsBinding.instance)?.removeObserver(this);
    _flashAniController.dispose();
    _exposureAniController.dispose();
    _cameraController!.dispose();
    super.dispose();
    print('DBG>camera>dispose()');
  }

  @override
  void didChangeAppLifecycleState(AppLifecycleState state) {
    final CameraController? controller = _cameraController;
    if (controller == null || !controller.value.isInitialized) {
      return;
    }
    if (state == AppLifecycleState.inactive) {
      controller.dispose();
    } else if (state == AppLifecycleState.resumed) {
      _selectCameraConnect(controller.description);
    }
  }

  //final GlobalKey<ScaffoldState> _scaffoldKey = GlobalKey<ScaffoldState>();

  @override
  Widget build(BuildContext context) {
    return Scaffold(
      //key: _scaffoldKey,
      appBar: AppBar(title: const Text('Camera')),
      body: Stack(
        children: [
          _change ? _lensPreview2() : _cameraPreview(),
          _buttons(),
        ],
      ),
    );
  }

  Widget _cameraPreview() {
    final CameraController? controller = _cameraController;
    if (controller == null || !controller.value.isInitialized) {
      return const Text('wait ...',   // Need translate!!!!!
        style: TextStyle(
          color: Colors.white, fontSize: 24.0, fontWeight: FontWeight.w900,
        ),
      );
    } else {
      return Listener(
        onPointerDown: (_) => ++ _pointers,
        onPointerUp: (_) => -- _pointers,
        child: CameraPreview(
          _cameraController!,
          child: LayoutBuilder(
            builder: (BuildContext context, BoxConstraints constraints) {
              return GestureDetector(
                behavior: HitTestBehavior.opaque,
                onScaleStart: _handleScaleStart,
                onScaleUpdate: _handleScaleUpdate,
                onVerticalDragStart: _handleVerticalDragStart,
                onVerticalDragUpdate: _handleVerticalDragUpdate,
                onVerticalDragEnd: _handleVerticalDragEnd,
                onTapDown: (TapDownDetails details) =>
                    onViewFinderTap(details, constraints),
              );
            },
          ),
        ),
      );
    }
  }

  Widget _buttons() {
    return Column(
      mainAxisAlignment: MainAxisAlignment.end,
      children: [
        Row(
          mainAxisAlignment: MainAxisAlignment.center,
          children: [
            IconButton(
              icon: const Icon(Icons.camera_alt_outlined),
              color: Colors.blue,
              iconSize: 40,
              onPressed: () => {},
              //controller != null &&
              //    controller.value.isInitialized &&
              //    !controller.value.isRecordingVideo
              //    ? onTakePictureButtonPressed : null,
            ),
            IconButton(
              icon: const Icon(Icons.change_circle_outlined),
              color: Colors.white,
              iconSize: 24,
              onPressed: () => {
                setState(() {
                  _change = !_change;
                })
              },
            )
          ]
        ),
      ]
    );
  }

  Widget _lensPreview2() {
    final CameraController? controller = _cameraController;
    if (controller == null || !controller.value.isInitialized) {
      return const Text('wait ...',   // Need translate!!!!!
        style: TextStyle(
          color: Colors.white, fontSize: 24.0, fontWeight: FontWeight.w900,
        ),
      );
    } else {
      if (!_initial) {
        _deviceRatio = (MediaQuery.of(context).size.height - 96)
            / MediaQuery.of(context).size.width;
        _scale = controller.value.aspectRatio / _deviceRatio;
        _initial = true;
      }
      print('DBG>camera>_ratio=$_deviceRatio ${controller.value.aspectRatio} _scale=$_scale');
      return AspectRatio(
        aspectRatio: 1 / _deviceRatio,
        child: Transform(
          alignment: Alignment.center,
          transform: Matrix4.diagonal3Values(1 / _scale, 1, 1),
          child: _cameraPreview(),
        ),
      );
    }
  }

  void _handleVerticalDragStart(DragStartDetails details) {
    print('DBG>camera>_handleVerticalDragStart()');
  }

  void _handleVerticalDragUpdate(DragUpdateDetails details) {
    print('DBG>camera>_handleVerticalDragUpdate()');
  }

  void _handleVerticalDragEnd(DragEndDetails details) {
    if (++ _currentLens >= cameras.length)  _currentLens = 0;
    _selectCameraConnect(cameras[_currentLens]);
    print('DBG>camera>_handleVerticalDragEnd() ${cameras.length}');
  }

  void _handleScaleStart(ScaleStartDetails details) {
    _baseScale = _currentScale;
  }

  Future<void> _handleScaleUpdate(ScaleUpdateDetails details) async {
    if (_cameraController == null || _pointers != 2) return;

    _currentScale = (_baseScale * details.scale)
        .clamp(_minAvailableZoom, _maxAvailableZoom);
    await _cameraController!.setZoomLevel(_currentScale);
  }

  Widget _thumbnail() {
    final VideoPlayerController? controller = _videoController;
    return Expanded(
      child: Align(
        alignment: Alignment.centerRight,
        child: Row(
          mainAxisSize: MainAxisSize.min,
          children: [
            if (controller == null && _imageFile == null)
              Container()
            else
              SizedBox(
                child: (controller == null)
                  ? (kIsWeb ? Image.network(_imageFile!.path)
                            : Image.file(File(_imageFile!.path)))
                  : Container(
                    child: AspectRatio(
                      aspectRatio: controller.value.size != null
                          ? controller.value.aspectRatio : 1.0,
                      child: VideoPlayer(controller),
                    ),
                    decoration: BoxDecoration(
                      border: Border.all(color: Colors.pink),
                    ),
                ),
                width: 64.0,
                height: 64.0,
              )
          ],
        ),
      ),
    );
  }

  Widget _modeControl() {
    return Column(
      children: [
        Row(
          mainAxisAlignment: MainAxisAlignment.spaceEvenly,
          mainAxisSize: MainAxisSize.max,
          children: <Widget>[
            IconButton(
              icon: const Icon(Icons.flash_on),
              color: Colors.blue,
              onPressed: _cameraController != null
                  ? onFlashModeButtonPressed : null,
            ),
            ...!kIsWeb
            ? <Widget>[
              IconButton(
                icon: const Icon(Icons.exposure),
                color: Colors.blue,
                onPressed: _cameraController != null
                    ? onExposureModeButtonPressed : null,
              ),
              IconButton(
                icon: const Icon(Icons.filter_center_focus),
                color: Colors.blue,
                onPressed: _cameraController != null
                    ? onFocusModeButtonPressed : null,
              ),
            ]
            : <Widget>[],
            IconButton(
              icon: Icon(_enableAudio ? Icons.volume_up : Icons.volume_mute),
              color: Colors.blue,
              onPressed: _cameraController != null
                  ? onAudioModeButtonPressed : null,
            ),
            IconButton(
              icon: Icon(
                  _cameraController?.value.isCaptureOrientationLocked ?? false
                  ? Icons.screen_lock_rotation : Icons.screen_rotation),
              color: Colors.blue,
              onPressed: _cameraController != null
                  ? onCaptureOrientationLockButtonPressed : null,
            ),
          ]
        ),
        _flashModeControl(),
        _exposureModeControl(),
        _focusModeControl(),
      ],
    );
  }

  Widget _flashModeControl() {
    return SizeTransition(
      sizeFactor: _flashAnimation,
      child: ClipRect(
        child: Row(
          mainAxisAlignment: MainAxisAlignment.spaceEvenly,
          mainAxisSize: MainAxisSize.max,
          children: [
            IconButton(
              icon: const Icon(Icons.flash_off),
              color: _cameraController?.value.flashMode == FlashMode.off
                  ? Colors.orange : Colors.blue,
              onPressed: _cameraController != null
                  ? () => onSetFlashModeButtonPressed(FlashMode.off)
                  : null,
            ),
            IconButton(
              icon: const Icon(Icons.flash_auto),
              color: _cameraController?.value.flashMode == FlashMode.auto
                  ? Colors.orange : Colors.blue,
              onPressed: _cameraController != null
                  ? () => onSetFlashModeButtonPressed(FlashMode.auto)
                  : null,
            ),
            IconButton(
              icon: const Icon(Icons.flash_on),
              color: _cameraController?.value.flashMode == FlashMode.always
                  ? Colors.orange : Colors.blue,
              onPressed: _cameraController != null
                  ? () => onSetFlashModeButtonPressed(FlashMode.always)
                  : null,
            ),
            IconButton(
              icon: const Icon(Icons.highlight),
              color: _cameraController?.value.flashMode == FlashMode.torch
                  ? Colors.orange : Colors.blue,
              onPressed: _cameraController != null
                  ? () => onSetFlashModeButtonPressed(FlashMode.torch)
                  : null,
            ),
          ],
        ),
      ),
    );
  }

  Widget _exposureModeControl() {
    final ButtonStyle styleAuto = TextButton.styleFrom(
      primary: _cameraController?.value.exposureMode == ExposureMode.auto
          ? Colors.orange : Colors.blue,
    );
    final ButtonStyle styleLocked = TextButton.styleFrom(
      primary: _cameraController?.value.exposureMode == ExposureMode.locked
          ? Colors.orange : Colors.blue,
    );
    return SizeTransition(
      sizeFactor: _exposureAnimation,
      child: ClipRect(
        child: Container(
          color: Colors.grey.shade50,
          child: Column(
            children: [
              const Center(child: Text('Exposure Mode')),
              Row(
                mainAxisAlignment: MainAxisAlignment.spaceEvenly,
                mainAxisSize: MainAxisSize.max,
                children: [
                  TextButton(
                    child: const Text('AUTO'),   // Need translate!!!!!
                    style: styleAuto,
                    onPressed: _cameraController != null
                        ? () => onSetExposureModeButtonPressed(ExposureMode.auto)
                        : null,
                    onLongPress: () {
                      if (_cameraController != null) {
                        _cameraController!.setExposurePoint(null);
                        showInSnackBar('Resetting exposure point');
                      }
                    },
                  ),
                  TextButton(
                    child: const Text('LOCKED'),   // Need translate!!!!!
                    style: styleLocked,
                    onPressed: _cameraController != null
                        ? () =>
                          onSetExposureModeButtonPressed(ExposureMode.locked)
                        : null
                  ),
                  TextButton(
                    child: const Text('RESET OFFSET'),   // Need translate!!!!!
                    style: styleLocked,
                    onPressed: _cameraController != null
                        ? () => _cameraController!.setExposureOffset(0.0)
                        : null,
                  ),
                ],
              ),
              const Center(
                child: Text('Exposure Offset'),
              ),
              Row(
                mainAxisAlignment: MainAxisAlignment.spaceEvenly,
                mainAxisSize: MainAxisSize.max,
                children: [
                  Text(_minAvailableExposureOffset.toString()),
                  Slider(
                    value: _currentExposureOffset,
                    min: _minAvailableExposureOffset,
                    max: _maxAvailableExposureOffset,
                    label: _currentExposureOffset.toString(),
                    onChanged: _minAvailableExposureOffset ==
                        _maxAvailableExposureOffset
                        ? null : setExposureOffset,
                  ),
                  Text(_maxAvailableExposureOffset.toString()),
                ],
              ),
            ],
          ),
        ),
      ),
    );
  }

  Widget _focusModeControl() {
    final ButtonStyle styleAuto = TextButton.styleFrom(
      primary: _cameraController?.value.focusMode == FocusMode.auto
          ? Colors.orange : Colors.blue,
    );
    final ButtonStyle styleLocked = TextButton.styleFrom(
      primary: _cameraController?.value.focusMode == FocusMode.locked
          ? Colors.orange : Colors.blue,
    );

    return SizeTransition(
      sizeFactor: _focusAnimation,
      child: ClipRect(
        child: Container(
          color: Colors.grey.shade50,
          child: Column(
            children: [
              const Center(child: Text('Focus Mode')),
              Row(
                mainAxisAlignment: MainAxisAlignment.spaceEvenly,
                mainAxisSize: MainAxisSize.max,
                children: [
                  TextButton(
                    child: const Text('AUTO'),   // Need translate!!!!!
                    style: styleAuto,
                    onPressed: _cameraController != null
                        ? () => onSetFocusModeButtonPressed(FocusMode.auto)
                        : null,
                    onLongPress: () {
                      if (_cameraController != null) {
                        _cameraController!.setFocusPoint(null);
                      }
                      showInSnackBar('Resetting focus point');
                    },
                  ),
                  TextButton(
                    child: const Text('LOCKED'),   // Need translate!!!!!
                    style: styleLocked,
                    onPressed: _cameraController != null
                        ? () => onSetFocusModeButtonPressed(FocusMode.locked)
                        : null,
                  ),
                ],
              ),
            ],
          ),
        ),
      ),
    );
  }

  Widget _captureControl() {
    final CameraController? controller = _cameraController;
    return Row(
      mainAxisAlignment: MainAxisAlignment.spaceEvenly,
      mainAxisSize: MainAxisSize.max,
      children: [
        IconButton(
          icon: const Icon(Icons.camera_alt),
          color: Colors.blue,
          onPressed: controller != null &&
              controller.value.isInitialized &&
              !controller.value.isRecordingVideo
              ? onTakePictureButtonPressed : null,
        ),
        IconButton(
          icon: const Icon(Icons.videocam),
          color: Colors.blue,
          onPressed: controller != null &&
              controller.value.isInitialized &&
              !controller.value.isRecordingVideo
              ? onVideoRecordButtonPressed : null,
        ),
        IconButton(
          icon: controller != null &&
              controller.value.isRecordingPaused
              ? const Icon(Icons.play_arrow) : const Icon(Icons.pause),
          color: Colors.blue,
          onPressed: controller != null &&
              controller.value.isInitialized &&
              controller.value.isRecordingVideo
              ? controller.value.isRecordingPaused
                ? onResumeButtonPressed : onPauseButtonPressed
              : null,
        ),
        IconButton(
          icon: const Icon(Icons.stop),
          color: Colors.red,
          onPressed: controller != null &&
              controller.value.isInitialized &&
              controller.value.isRecordingVideo
              ? onStopButtonPressed : null,
        ),
        IconButton(
          icon: const Icon(Icons.pause_presentation),
          color: controller != null &&
              controller.value.isPreviewPaused
              ? Colors.red : Colors.blue,
          onPressed: controller != null
              ? onPausePreviewButtonPressed : null,
        ),
      ],
    );
  }

  Widget _cameraToggle() {
    final List<Widget> toggles = [];

    final Null Function(CameraDescription? description) onChanged =
        (CameraDescription? description) {
      if (description == null) { return; }
      _selectCameraConnect(description);
    };

    if (cameras.isEmpty) {
      return const Text('No camera found');   // Need translate!!!!!
    } else {
      for (final CameraDescription descript in cameras) {
        print('DBG>camera>_cameraToggle() ${descript.name}');
        toggles.add(
          SizedBox(
            width: 90.0,
            child: RadioListTile<CameraDescription>(
              title: Icon(getCameraLensIcon(descript.lensDirection)),
              groupValue: _cameraController?.description,
              value: descript,
              onChanged: _cameraController != null &&
                _cameraController!.value.isRecordingVideo
                ? null : onChanged,
            ),
          ),
        );
      }
    }
    return Row(children: toggles);
  }

  String timestamp() => DateTime.now().millisecondsSinceEpoch.toString();

  void showInSnackBar(String message) {
    ScaffoldMessenger.of(context)
        .showSnackBar(SnackBar(content: Text(message)));
    //_scaffoldKey.currentState?.showSnackBar(SnackBar(content: Text(message)));
  }

  void onViewFinderTap(TapDownDetails details, BoxConstraints constraints) {
    if (_cameraController == null) {
      print('DBG>camera>onTap but null');
      return;
    }

    print('DBG>camera>onTap...');
    final CameraController controller = _cameraController!;
    final Offset offset = Offset(
      details.localPosition.dx / constraints.maxWidth,
      details.localPosition.dy / constraints.maxHeight,
    );
    controller.setExposurePoint(offset);
    controller.setFocusPoint(offset);
  }

  Future<void> _selectCameraConnect(CameraDescription description) async {
    if (_cameraController != null) await _cameraController!.dispose();

    final CameraController controller = CameraController(
      description,
      //kIsWeb ? ResolutionPreset.max : ResolutionPreset.medium,
      ResolutionPreset.ultraHigh,
      enableAudio: _enableAudio,
      imageFormatGroup: ImageFormatGroup.jpeg,
    );

    _cameraController = controller;

    controller.addListener(() {
      if (mounted) {
        setState(() {});
      }
      if (controller.value.hasError) {
        showInSnackBar('Camera error ${controller.value.errorDescription}');
      }
    });

    try {
      await controller.initialize();
      await Future.wait(<Future<Object?>>[
        ...!kIsWeb
          ? <Future<Object?>>[
            controller.getMinExposureOffset().then(
                (double value) => _minAvailableExposureOffset = value),
            controller
                .getMaxExposureOffset()
                .then((double value) => _maxAvailableExposureOffset = value)
          ]
            : <Future<Object?>>[],
        controller.getMaxZoomLevel().then((double value) =>
            _maxAvailableZoom = value),
        controller.getMinZoomLevel().then((double value) =>
            _minAvailableZoom = value),
      ]);
    } on CameraException catch (e) {
      _showCameraException(e);
    }

    if (mounted) {
      setState(() {});
    }
  }

  void onTakePictureButtonPressed() {
    takePicture().then((XFile? file) {
      if (mounted) {
        setState(() {
          _imageFile = file;
          _videoController?.dispose();
          _videoController = null;
        });
        if (file != null) {
          showInSnackBar('Picture saved to ${file.path}');
          print('DBG>camera>Picture saved to ${file.path}');
        }
      }
    });
  }

  void onFlashModeButtonPressed() {
    if (_flashAniController.value == 1) {
      _flashAniController.reverse();
    } else {
      _flashAniController.forward();
      _exposureAniController.reverse();
      _focusAniController.reverse();
    }
  }

  void onExposureModeButtonPressed() {
    if (_exposureAniController.value == 1) {
      _exposureAniController.reverse();
    } else {
      _exposureAniController.forward();
      _flashAniController.reverse();
      _focusAniController.reverse();
    }
  }

  void onFocusModeButtonPressed() {
    if (_focusAniController.value == 1) {
      _focusAniController.reverse();
    } else {
      _focusAniController.forward();
      _flashAniController.reverse();
      _exposureAniController.reverse();
    }
  }

  void onAudioModeButtonPressed() {
    _enableAudio = !_enableAudio;
    if (_cameraController != null) {
      _selectCameraConnect(_cameraController!.description);
    }
  }

  Future<void> onCaptureOrientationLockButtonPressed() async {
    try {
      if (_cameraController != null) {
        final CameraController controller = _cameraController!;
        if (controller.value.isCaptureOrientationLocked) {
          await controller.unlockCaptureOrientation();
          showInSnackBar('Capture orientation unlocked');
        } else {
          await controller.lockCaptureOrientation();
          showInSnackBar('Capture orientation locked to'
              ' ${controller.value.lockedCaptureOrientation
              .toString().split('.').last}');
        }
      }
    } on CameraException catch (e) {
      _showCameraException(e);
    }
  }

  void onSetFlashModeButtonPressed(FlashMode mode) {
    setFlashMode(mode).then((_) {
      if (mounted) {
        setState(() {});
      }
      showInSnackBar('Flash mode set to ${mode.toString().split('.').last}');
    });
  }

  void onSetExposureModeButtonPressed(ExposureMode mode) {
    setExposureMode(mode).then((_) {
      if (mounted) {
        setState(() {});
      }
      showInSnackBar('Exposure mode set to ${mode.toString().split('.').last}');
    });
  }

  void onSetFocusModeButtonPressed(FocusMode mode) {
    setFocusMode(mode).then((_) {
      if (mounted) {
        setState(() {});
      }
      showInSnackBar('Focus mode set to ${mode.toString().split('.').last}');
    });
  }

  void onVideoRecordButtonPressed() {
    startVideoRecording().then((_) {
      if (mounted) {
        setState(() {});
      }
    });
  }

  void onStopButtonPressed() {
    stopVideoRecording().then((XFile? file) {
      if (mounted) {
        setState(() {});
      }
      if (file != null) {
        showInSnackBar('Video recorded to ${file.path}');
        _videoFile = file;
        _startVideoPlayer();
      }
    });
  }

  Future<void> onPausePreviewButtonPressed() async {
    final CameraController? controller = _cameraController;
    if (controller == null || !controller.value.isInitialized) {
      showInSnackBar('Error: select a camera first.');
      return;
    }
    if (controller.value.isPreviewPaused) {
      await controller.resumePreview();
    } else {
      await controller.pausePreview();
    }
    if (mounted) {
      setState(() {});
    }
  }

  void onPauseButtonPressed() {
    pauseVideoRecording().then((_) {
      if (mounted) {
        setState(() {});
      }
      showInSnackBar('Video recording paused');
    });
  }

  void onResumeButtonPressed() {
    resumeVideoRecording().then((_) {
      if (mounted) {
        setState(() {});
      }
      showInSnackBar('Video recording resumed');
    });
  }

  Future<void> startVideoRecording() async {
    final CameraController? controller = _cameraController;
    if (controller == null || !controller.value.isInitialized) {
      showInSnackBar('Error: select a camera first.');
      return;
    }
    if (controller.value.isRecordingVideo) {
      return;
    }
    try {
      await controller.startVideoRecording();
    } on CameraException catch (e) {
      _showCameraException(e);
      return;
    }
  }

  Future<XFile?> stopVideoRecording() async {
    final CameraController? controller = _cameraController;
    if (controller == null || !controller.value.isRecordingVideo) {
      return null;
    }
    try {
      return controller.stopVideoRecording();
    } on CameraException catch (e) {
      _showCameraException(e);
      return null;
    }
  }

  Future<void> pauseVideoRecording() async {
    final CameraController? controller = _cameraController;
    if (controller == null || !controller.value.isRecordingVideo) {
      return;
    }
    try {
      await controller.pauseVideoRecording();
    } on CameraException catch (e) {
      _showCameraException(e);
      rethrow;
    }
  }

  Future<void> resumeVideoRecording() async {
    final CameraController? controller = _cameraController;
    if (controller == null || !controller.value.isRecordingVideo) {
      return;
    }
    try {
      await controller.resumeVideoRecording();
    } on CameraException catch (e) {
      _showCameraException(e);
      rethrow;
    }
  }

  Future<void> setFlashMode(FlashMode mode) async {
    if (_cameraController == null)  return;
    try {
      await _cameraController!.setFlashMode(mode);
    } on CameraException catch (e) {
      _showCameraException(e);
      rethrow;
    }
  }

  Future<void> setExposureMode(ExposureMode mode) async {
    if (_cameraController == null)  return;
    try {
      await _cameraController!.setExposureMode(mode);
    } on CameraException catch (e) {
      _showCameraException(e);
      rethrow;
    }
  }

  Future<void> setExposureOffset(double offset) async {
    if (_cameraController == null)  return;
    setState(() {
      _currentExposureOffset = offset;
    });
    try {
      offset = await _cameraController!.setExposureOffset(offset);
    } on CameraException catch (e) {
      _showCameraException(e);
      rethrow;
    }
  }

  Future<void> setFocusMode(FocusMode mode) async {
    if (_cameraController == null)  return;
    try {
      await _cameraController!.setFocusMode(mode);
    } on CameraException catch (e) {
      _showCameraException(e);
      rethrow;
    }
  }

  Future<void> _startVideoPlayer() async {
    if (_videoFile == null) return;
    final VideoPlayerController controller = kIsWeb
        ? VideoPlayerController.network(_videoFile!.path)
        : VideoPlayerController.file(File(_videoFile!.path));
    _videoPlayerListener = () {
      if (_videoController != null && _videoController!.value.size != null) {
        if (mounted) {
          setState(() {});
        }
        _videoController!.removeListener(_videoPlayerListener!);
      }
    };
    controller.addListener(_videoPlayerListener!);
    await controller.setLooping(true);
    await controller.initialize();
    await _videoController?.dispose();
    if (mounted) {
      setState(() {
        _imageFile = null;
        _videoController = controller;
      });
    }
    await controller.play();
  }

  Future<XFile?> takePicture() async {
    final CameraController? controller = _cameraController;
    if (controller == null || !controller.value.isInitialized) {
      showInSnackBar('Error: select a camera first.');
      return null;
    }
    if (controller.value.isTakingPicture) {
      return null;
    }
    try {
      final XFile file = await controller.takePicture();
      return file;
    } on CameraException catch (e) {
      _showCameraException(e);
      return null;
    }
  }

  void _showCameraException(CameraException e) {
    logError(e.code, e.description);
    showInSnackBar('Error: ${e.code}\n${e.description}');
  }
}