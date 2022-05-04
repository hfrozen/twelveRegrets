import 'dart:ui' as ui;
import 'dart:async';
import 'dart:convert';
import 'dart:typed_data';
import 'dart:math';
import 'package:flutter/material.dart';
import 'package:flutter/gestures.dart';
import 'package:flutter/widgets.dart' hide Image;
import 'package:vector_math/vector_math.dart';
import 'package:json_annotation/json_annotation.dart';
import 'package:get/get.dart';
import 'package:papy/tools/figure.dart';
import 'package:papy/refer/const.dart';
import 'package:papy/base/how.dart';

part 'painter.g.dart';

class Painter extends StatefulWidget {
  final PainterController painterController;
  final EditMode editMode;

  Painter(this.painterController, this.editMode)
      : super(key: ValueKey<PainterController>(painterController));

  @override
  _PainterState createState() => _PainterState();
}

class _PainterState extends State<Painter> {
  bool _finished = false;
  Offset _canvasOffset = const Offset(0, 0);  // will be delete
  Offset _currentOffset = const Offset(0, 0);
  Offset? _longStartOffset = null;            // will be delete
  double _currentScale = 1.0;
  double _currentAngle = 1.0;
  int _currentThickness =
      ((Environs.maxThickness - Environs.minThickness) / 2 +
          Environs.minThickness).toInt();

  @override
  void initState() {
    super.initState();
    _finished = false;
    widget.painterController._widgetFinish = _finish;
  }

  Size _finish() {
    setState(() {
      _finished = true;
    });
    return context.size ?? const Size(0, 0);
  }

  @override
  Widget build(BuildContext context) {
    Widget child = CustomPaint(
      willChange: true,
      painter: _PainterPainter(widget.painterController._pathTrack,
          repaint: widget.painterController),
    );
    child = ClipRect(child: child,);
    if (!_finished) {
      child = RawGestureDetector(
        gestures: <Type, GestureRecognizerFactory>{
          LongPressGestureRecognizer:
          GestureRecognizerFactoryWithHandlers<LongPressGestureRecognizer>(
            () => LongPressGestureRecognizer(
              debugOwner: this,
              duration: const Duration(seconds: 1),
            ),
            (LongPressGestureRecognizer instance) {
              instance.onLongPress = () => _onLongPress();
            },
          ),
        },
      //);
        child: GestureDetector(
          child: child,
          //behavior: HitTestBehavior.translucent,
          onScaleStart: _onScaleStart,
          onScaleUpdate: _onScaleUpdate,
          onScaleEnd: _onScaleEnd,
          onTapDown: _onTapDown,
        ),
      );
    }
    return Container(
      child: child,
      width: double.infinity,
      height: double.infinity,
    );
  }

  void _onScaleStart(ScaleStartDetails start) {
    _longStartOffset = null;
    if (widget.editMode == EditMode.draw
        || widget.editMode == EditMode.picture) {
      Offset offset = (context.findRenderObject() as RenderBox)
          .globalToLocal(start.focalPoint);
      if (widget.editMode == EditMode.draw) {
        offset += _canvasOffset;
        widget.painterController._pathTrack.add(offset);
        //widget.painterController._pathTrack.addWith(offset, _currentThickness.toDouble());
        widget.painterController._notifyListeners();
        //print('DEBUG>painter>add path $offset $_currentThickness');
      } else {  // EditMode.picture
        int target = widget.painterController
              .isPressOnFigure(offset + _canvasOffset);
        if (target >= 0) {
          widget.painterController.setTargetFigure(target);
          _currentOffset =
            Offset(offset.dx.toPrecision(0), offset.dy.toPrecision(0));
          widget.painterController.figureSelectMode = false;
          widget.painterController._figureTrail.startTrim(_currentOffset);
          widget.painterController._onSelectItemListener!.call();
          print('DBG>painter>_onScaleStart');
        }
      }
    }
  }

  double _getThickness(Offset delta) {
    double velocity = sqrt(pow(delta.dx, 2) + pow(delta.dy, 2));
    if (velocity < Environs.minVelocity) {
      velocity = Environs.minVelocity;
    } else if (velocity > Environs.maxVelocity) {
      velocity = Environs.maxVelocity;
    }
    double thickness = Environs.maxThickness -
        (Environs.maxThickness - Environs.minThickness) *
            (velocity - Environs.minVelocity) /
            (Environs.maxVelocity - Environs.minVelocity);
    if (thickness < Environs.minThickness) {
      thickness = Environs.minThickness;
    } else if (thickness > Environs.maxThickness) {
      thickness = Environs.maxThickness;
    }
    return thickness;
  }

  void _onScaleUpdate(ScaleUpdateDetails update) {
    if (widget.editMode == EditMode.draw
        || widget.editMode == EditMode.picture) {
      Offset offset = (context.findRenderObject() as RenderBox)
          .globalToLocal(update.focalPoint);
      if (widget.editMode == EditMode.draw) {
        offset += _canvasOffset;
        widget.painterController._pathTrack
            .update(offset, widget.painterController.compressionLevel);
        //int t = _getThickness(update.focalPointDelta).toInt();
        //if (t != _currentThickness) {
        //  //widget.painterController._pathTrack._filterBlankChronicle();
        //  widget.painterController._pathTrack.end();
        //  widget.painterController._pathTrack.addWith(offset, t.toDouble());
        //  _currentThickness = t;
        //  print('DEBUG>painter>new path $offset $_currentThickness');
        //}
        //widget.painterController.updatePath(offset,
        //    widget.painterController.compressionLevel, t);
        widget.painterController._notifyListeners();
        //print('DEBUG>painter>_onScaleUpdate() delta:${update.focalPointDelta} $t');
      } else { // EditMode.picture
        if (widget.painterController.getTargetFigure() >= 0) {
          Offset intOffset =
              Offset(offset.dx.toPrecision(0), offset.dy.toPrecision(0));
          if (_currentOffset != intOffset) { _currentOffset = intOffset; }
          double value = update.scale.toPrecision(1);
          if (_currentScale != value) { _currentScale = value; }
          value = degrees(update.rotation).toPrecision(0);
          if (_currentAngle != value) { _currentAngle = value; }
          if (widget.painterController._figureTrail.trimming(_currentOffset,
              _currentScale, radians(_currentAngle), context.size!)) {
            widget.painterController._notifyListeners();
            //widget.painterController._updatePaint();
          }
        }
      }
    }
  }

  void _onScaleEnd(ScaleEndDetails end) {
    if (widget.editMode == EditMode.draw
        || widget.editMode == EditMode.picture) {
      if (widget.editMode == EditMode.draw) {
        widget.painterController._pathTrack._filterBlankChronicle();
        widget.painterController._pathTrack.end();
        widget.painterController._notifyListeners();
        widget.painterController._onInterimUpdateListener!.call();
        //widget.painterController._onMenuRealignListener!.call();
      } else {  // EditMode.picture
        if (widget.painterController.getTargetFigure() >= 0) {
          widget.painterController._figureTrail.endTrim();
          widget.painterController._notifyListeners();
        }
      }
    }
  }

  void _onLongPress() {
    if (widget.editMode == EditMode.picture && _longStartOffset != null) {
      int current = widget.painterController
          .isPressOnFigure(_longStartOffset! + _canvasOffset);
      if (current >= 0) {
        widget.painterController.toggleFigureSelect(current);
        widget.painterController._notifyListeners();
        widget.painterController._onSelectItemListener!.call(); // for delete
      }
    }
  }


  void _onTapDown(TapDownDetails down) {
    if (widget.editMode == EditMode.picture) {
      Offset offset = (context.findRenderObject() as RenderBox)
          .globalToLocal(down.globalPosition);
      int current = widget.painterController
          .isPressOnFigure(offset + _canvasOffset);
      if (!widget.painterController.figureSelectMode) {
        _longStartOffset = offset;
        widget.painterController.setTargetFigure(current);
        widget.painterController._notifyListeners();
      }
      else {
        if (current >= 0) {
          widget.painterController.toggleFigureSelect(current);
          widget.painterController._notifyListeners();
          widget.painterController._onSelectItemListener!.call(); // for delete
        }
      }
    }
  }
}

class _PainterPainter extends CustomPainter {
  final _PathTrack _pathTrack;

  _PainterPainter(this._pathTrack,
      {required Listenable repaint}) : super(repaint: repaint);

  @override
  void paint(Canvas canvas, Size size) {
    _pathTrack.draw(canvas, size);
  }

  @override
  bool shouldRepaint(_PainterPainter oldDelegate) {
    return true;
  }
}

@JsonSerializable()   //nullable: false)
class Point {
  double x;
  double y;
  Point(this.x, this.y);

  factory Point.fromJson(Map<String, dynamic> json) => _$PointFromJson(json);
  Map<String, dynamic> toJson() => _$PointToJson(this);
}

@JsonSerializable()   //nullable: false)
class PathTrackEntry {
  double x, y;

  List<Point> lineToList = [];

  @ColorSerializer()
  Color color;
  int paintBlendMode;
  double paintThickness;

  PathTrackEntry(this.x, this.y, this.color,
      this.paintBlendMode, this.paintThickness);

  factory PathTrackEntry.fromJson(Map<String, dynamic> json) =>
      _$PathTrackEntryFromJson(json);
  Map<String, dynamic> toJson() => _$PathTrackEntryToJson(this);

  Paint extractPaint() {
    Paint paint = Paint();
    //paint.color = Color.fromARGB(paintA, paintR, paintG, paintB);
    paint.color = color;
    paint.blendMode = BlendMode.values[paintBlendMode];
    paint.strokeWidth = paintThickness;
    paint.style = PaintingStyle.stroke;
    return paint;
  }

  Path extractPath() {
    Path path = Path();
    path.moveTo(x, y);
    for (var point in lineToList) {
      path.lineTo(point.x, point.y);
    }
    return path;
  }

  MapEntry<Path, Paint> convertToPathTrackFormat() {
    return MapEntry(extractPath(), extractPaint());
  }
}

class _PathTrack {
  late List<PathTrackEntry> _paths;
  late Paint _foregroundPaint;
  late Paint _backgroundPaint;
  late bool _inDrag;
  Point max = Point(0, 0);
  late Function _figureDrawFunction;

  void _filterBlankChronicle() {
    List<PathTrackEntry> result = <PathTrackEntry>[];
    for (var path in _paths) {
      if (path.lineToList.isNotEmpty) {
        Point initialPoint = path.lineToList.first;
        for (var point in path.lineToList) {
          if (point.x != initialPoint.x || point.y != initialPoint.y) {
            result.add(path);
            break;
          }
        }
      } else {
        continue;
      }
    }
    _paths = result;
  }

  // 이 부분이 제대로 되는지 검사할 것
  _PathTrack(List<PathTrackEntry>? paths) {
    if (paths != null) {
      _paths = paths;
    } else {
      _paths = <PathTrackEntry>[];
    }
    _inDrag = false;
    _backgroundPaint = Paint();
  }

  int get length => _paths.length;

  void setFigureDrawFunction(Function figureDrawFunction) {
    _figureDrawFunction = figureDrawFunction;
  }

  void setBackgroundColor(Color backgroundColor) {
    _backgroundPaint.color = backgroundColor;
  }

  void undo() {
    if (!_inDrag && _paths.isNotEmpty) {
      _paths.removeLast();
    }
  }

  void clear() {
    if (!_inDrag) {
      _paths.clear();
    }
  }

  void add(Offset startPoint) {
    if (!_inDrag) {
      _inDrag = true;
      Path path = Path();
      path.moveTo(startPoint.dx, startPoint.dy);
      PathTrackEntry pathTrackEntry = PathTrackEntry(
          startPoint.dx, startPoint.dy, _foregroundPaint.color,
          _foregroundPaint.blendMode.index, _foregroundPaint.strokeWidth);
      _paths.add(pathTrackEntry);
    }
  }

  void addWith(Offset startPoint, double thickness) {
    if (!_inDrag) {
      _inDrag = true;
      Path path = Path();
      path.moveTo(startPoint.dx, startPoint.dy);
      PathTrackEntry pathTrackEntry = PathTrackEntry(
          startPoint.dx, startPoint.dy, _foregroundPaint.color,
          _foregroundPaint.blendMode.index, thickness);
      _paths.add(pathTrackEntry);
    }
  }

  void update(Offset nextPoint, int compressionLevel) {
    if (_inDrag) {
      if (_paths.last.lineToList.isNotEmpty) {
        Offset previousOffset = Offset(
            _paths.last.lineToList.last.x, _paths.last.lineToList.last.y);
        if ((previousOffset - nextPoint).distance >
            _paths.last.paintThickness / compressionLevel) {
          Path path = _paths.last.extractPath();
          path.lineTo(nextPoint.dx, nextPoint.dy);
          _paths.last.lineToList.add(Point(nextPoint.dx, nextPoint.dy));
        }
      } else {
        Path path = _paths.last.extractPath();
        path.lineTo(nextPoint.dx, nextPoint.dy);
        _paths.last.lineToList.add(Point(nextPoint.dx, nextPoint.dy));
      }
    }
  }

  void end() {
    _inDrag = false;
  }

  Offset getMaxCoordinates() {
    Point max = Point(0, 0);
    for (var path in _paths) {
      if (path.lineToList.isNotEmpty) {
        //Point initialPoint = path.lineToList.first;
        for (var point in path.lineToList) {
          if (point.x > max.x)  max.x = point.x;
          if (point.y > max.y)  max.y = point.y;
        }
      } else {
        continue;
      }
    }
    return Offset(max.x, max.y);
  }

  void draw(Canvas canvas, Size size) {   // PathTrack
    canvas.saveLayer(Offset.zero & size, Paint());
    canvas.drawRect(Rect.fromLTWH(0, 0, size.width, size.height),
        _backgroundPaint);
    canvas.saveLayer(Offset.zero & size, Paint());
    // safe backgroundPaint from blendmode.clear !!!
    _figureDrawFunction.call(canvas, size, false);
    for (PathTrackEntry path in _paths) {
      MapEntry<Path, Paint> old = path.convertToPathTrackFormat();
      canvas.drawPath(old.key, old.value);
    }
    _figureDrawFunction.call(canvas, size, true);
    canvas.restore();
    canvas.restore();
  }
}

typedef PictureCallback = PictureDetails Function();

class PictureDetails {
  final ui.Picture picture;
  final int width, height;

  const PictureDetails(this.picture, this.width, this.height);

  Future<ui.Image> toImage() {
    return picture.toImage(width, height);
  }
  Future<Uint8List> toPNG() async {
    final image = await toImage();
    return (await image.toByteData(format: ui.ImageByteFormat.png))!
        .buffer
        .asUint8List();
  }
}

class PainterController extends ChangeNotifier {
  Color _foregroundColor = const Color.fromARGB(255, 0, 0, 0);
  Color _backgroundColor = const Color.fromARGB(255, 0, 255, 255);
  double _thickness = 1.0;
  int compressionLevel = 4;
  bool _eraseMode = false;

  PictureDetails? _cached;
  late _PathTrack _pathTrack;
  FigureTrail _figureTrail = FigureTrail(null);
  ValueGetter<Size>? _widgetFinish;
  Function? _onInterimUpdateListener;
  Function? _onMenuRealignListener;
  Function? _onSelectItemListener;

  PainterController(String? pathTrackString, String? figureTrailString, {
    required this.compressionLevel
    }) {
    _setPathTrack(pathTrackString);
    _setFigureTrail(figureTrailString);
    _pathTrack.setFigureDrawFunction(_figureTrail.draw);
  }

  bool get figureSelectMode => _figureTrail.selectMode;
  void set figureSelectMode(value) => _figureTrail.setSelectMode(value);
  void toggleFigureSelect(index) {
    _figureTrail.toggleSelect(index);
    _onSelectItemListener!.call();
  }

  String get getPathTrack => _serializedPathTrack();

  void _setPathTrack(String? pathTrackString) {
    _pathTrack = _PathTrack(_unSerializePathTrack(pathTrackString));
  }

  String _serializedPathTrack() {
    return jsonEncode(_pathTrack._paths);
  }

  List<PathTrackEntry>? _unSerializePathTrack(String? pathTrackString) {
    if (pathTrackString != null) {
      return (jsonDecode(pathTrackString) as List).map((entry) =>
          PathTrackEntry.fromJson(entry)).toList();
    } else { return null; }
  }

  String get getFigureTrail => _serializedFigureTrail();

  void _setFigureTrail(String? figureTrailString) {
    _figureTrail = FigureTrail(_unSerializeFigureTrail(figureTrailString));
  }

  String _serializedFigureTrail() {
    return jsonEncode(_figureTrail.figures);
  }

  List<Figure>? _unSerializeFigureTrail(figureTrailString) {
    if (figureTrailString != null) {
      return (jsonDecode(figureTrailString) as List).map((entry) =>
          Figure.fromJson(entry)).toList();
    } else { return null; }
  }

  void setOnInterimUpdateListener(Function onInterimUpdateListener) {
    _onInterimUpdateListener = onInterimUpdateListener;
  }

  void setOnMenuRealignListener(Function onMenuRealignListener) {
    _onMenuRealignListener = onMenuRealignListener;
  }

  void setOnImageUpListener(Function onImageUpListener) {
    _figureTrail.setImageUpListener(onImageUpListener);
  }

  void setSelectItemListener(Function onSelectItemListener) {
    _onSelectItemListener = onSelectItemListener;
  }

  bool hasPathTrack() {
    return _pathTrack.length != 0;
  }

  bool get drawMode => !_eraseMode;
  bool get eraseMode => _eraseMode;
  set eraseMode(bool enabled) {
    _eraseMode = enabled;
    _updatePaint();
  }

  Color get foregroundColor => _foregroundColor;
  set foregroundColor(Color color) {
    _foregroundColor = color;
    _updatePaint();
  }

  Color get backgroundColor => _backgroundColor;
  set backgroundColor(Color color) {    // paint
    _backgroundColor = color;
    _updatePaint();
  }

  double get thickness => _thickness;
  set thickness(double t) {
    _thickness = t;
    _updatePaint();
  }

  void undoPathTrack() {
    if (!isFinished()) {
      _pathTrack.undo();
      notifyListeners();
    }
  }

  void clearPathTrack() {
    if (!isFinished()) {
      _pathTrack.clear();
      notifyListeners();
    }
  }

  int getPathTrackLength() => _pathTrack.length;
  Offset getMaxCoordinatesInPathTrack() => _pathTrack.getMaxCoordinates();

  void addPath(Offset startPoint, double t) {
    Path path = Path();
    path.moveTo(startPoint.dx, startPoint.dy);
    PathTrackEntry pathTrackEntry = PathTrackEntry(
        startPoint.dx, startPoint.dy, _pathTrack._foregroundPaint.color,
        _pathTrack._foregroundPaint.blendMode.index, t);
    _pathTrack._paths.add(pathTrackEntry);
  }

  int getTargetFigure() {
    if (_figureTrail == null) return -1;
    return _figureTrail.target;
  }

  void setTargetFigure(int target) {
    //if (_figureTrail != null)
      _figureTrail.target = target;
  }

  int getFigureTrailLength() {
    if (_figureTrail == null) return 0;
    return _figureTrail.length;
  }

  int isPressOnFigure(Offset offset) {
    if (_figureTrail == null) return -1;
    return _figureTrail.isPressOn(offset);
  }

  void removeFigureTrail() {
    if (_figureTrail != null) {
      _figureTrail.remove();
      notifyListeners();
    }
  }
  void undoFigureTrail() {
    if (_figureTrail != null) {
      _figureTrail.undo();
      notifyListeners();
    }
  }

  void clearFigureTrail() {
    if (_figureTrail != null) {
      _figureTrail.clear();
      notifyListeners();
    }
  }

  void addFigureTrail(String path, ui.Image image, Offset offset, double scale) {
    Figure figure = Figure(path, offset.dx, offset.dy, scale, 0.0);
    figure.image = image;
    _figureTrail.add(figure);
    notifyListeners();
  }

  void addFigureTrailFile(String path, Size size) {
    Future image = _figureTrail.uiImageFromFile(path);
    image.then((value) {
      double scale = 1.0;
      Offset offset = const Offset(0, 0);
      size *= 0.9;
      if (value.width >= size.width.toInt()
          || value.height >= size.height.toInt()) {
        scale = min(size.width / value.width, size.height / value.height);
        offset = Offset(-(value.width - value.width * scale) / 2,
            -(value.height - value.height * scale) / 2);
      }
      print('DBG>painter>add figure scale=$scale');
      addFigureTrail(path, value, offset, scale);
    });
  }

  void _notifyListeners() {
    notifyListeners();
  }

  void _updatePaint() {
    Paint paint = Paint();
    if (_eraseMode) {
      paint.color = const Color.fromARGB(0, 255, 0, 0);
      paint.blendMode = BlendMode.clear;
    } else {
      paint.color = foregroundColor;
    }
    paint.style = PaintingStyle.stroke;
    paint.strokeWidth = _thickness;
    _pathTrack._foregroundPaint = paint;
    _pathTrack.setBackgroundColor(_backgroundColor);
    notifyListeners();
  }

  PictureDetails? finish() {
    if (!isFinished()) {
      _cached = _render(_widgetFinish!());
    }
    return _cached;
  }

  PictureDetails _render(Size size) {
    ui.PictureRecorder recorder = ui.PictureRecorder();
    Canvas canvas = Canvas(recorder);
    _pathTrack.draw(canvas, size);
    return PictureDetails(
        recorder.endRecording(), size.width.floor(), size.height.floor());
  }

  bool isFinished() {
    return _cached != null; // _cached is null return false
  }
}
