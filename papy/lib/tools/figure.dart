import 'dart:ui' as ui;   // Image at painting.dart, width is int
import 'dart:io';
import 'dart:async';
import 'dart:typed_data';
import 'dart:math';
import 'package:drift/drift.dart';
import 'package:flutter/material.dart';  // Image at Image.dart width is double
import 'package:get/get.dart';
import 'package:permission_handler/permission_handler.dart';
import 'package:json_annotation/json_annotation.dart';
import 'package:papy/refer/const.dart';

part 'figure.g.dart';

//toNull(_) => null;

@JsonSerializable()
class Preview {
  String data;
  Preview(this.data);

  factory Preview.fromJson(Map<String, dynamic> json) =>
      _$PreviewFromJson(json);
  Map<String, dynamic> toJson() => _$PreviewToJson(this);
}

@JsonSerializable()
class Figure {
  String path;
  double posx = 0.0;
  double posy = 0.0;
  double scale = 1.0;
  double radian = 0.0;

  Figure(this.path, this.posx, this.posy, this.scale, this.radian);

  factory Figure.fromJson(Map<String, dynamic> json) =>
      _$FigureFromJson(json);
  Map<String, dynamic> toJson() => _$FigureToJson(this);

  //@JsonKey(ignore: true)
  //@JsonKey(toJson: toNull, includeIfNull: false)
  ui.Image? image;

  //@JsonKey(ignore: true)
  //@JsonKey(toJson: toNull, includeIfNull: false)
  bool selected = false;
  bool trimming = false;
}

class _Trim {
  _Trim({required this.figure}) { _setDefaultRect(); }
  Figure figure;
  Rect edge = const Rect.fromLTWH(0, 0, 0, 0);
  Rect tilt = const Rect.fromLTWH(0, 0, 0, 0);

  _setDefaultRect() {
    edge = _getViewRect(figure);
    tilt = _getCircumscribedRect(edge, figure.radian);
  }

  // trim
  void draw(Canvas canvas, Size size, bool selectMode) async {
    canvas.save();
    edge = _getInScreen(_getViewRect(figure), size);
    if (figure.radian != 0.0) {
      tilt = _getCircumscribedRect(edge, figure.radian);
      Offset offset = tilt.topLeft;
      tilt = _getInScreen(tilt, size);
      if (offset != tilt.topLeft) {
        edge = edge.shift(tilt.topLeft - offset);
      }
    } else { tilt = edge; }

    if (figure.image != null) {
      canvas.save();
      canvas.clipRect(tilt);   //rect);
      Paint paint = Paint();

      canvas.translate((edge.left + edge.width / 2),
          (edge.top + edge.height / 2));
      canvas.rotate(figure.radian);
      canvas.translate(-(edge.left + edge.width / 2),
          -(edge.top + edge.height / 2));

      canvas.translate(edge.left, edge.top);
      canvas.scale(figure.scale);
      canvas.translate(-edge.left, -edge.top);

      canvas.drawImage(figure.image!, edge.topLeft, paint);
      // reference point of tilt is 'center' of image,
      // resize is 'top-left' of image

      canvas.restore();
      //paint = Paint()..color = Colors.blue
      //  ..style = PaintingStyle.stroke
      //  ..strokeWidth = 2;
      //canvas.drawRect(edge, paint);
      if (figure.trimming) {
        paint = Paint()
          ..color = Colors.red
          ..style = PaintingStyle.stroke
          ..strokeWidth = 2;
        canvas.drawRect(tilt, paint);
      }

      if (selectMode) {
        final icon = figure.selected
            ? Icons.check_circle_outline : Icons.circle_outlined;
        TextPainter textPainter = TextPainter(textDirection: TextDirection.rtl);
        textPainter.text = TextSpan(text: String.fromCharCode(icon.codePoint),
            style: TextStyle(fontSize: 40.0, fontFamily: icon.fontFamily));
        textPainter.layout();
        textPainter.paint(canvas, tilt.center);
      }
    }
    canvas.restore();
  }

  Rect _getViewRect(Figure figure) {
    Rect rect = Rect.fromLTWH(
        figure.posx, figure.posy,
        figure.image!.width.toDouble(), figure.image!.height.toDouble()
    );
    if (figure.scale.toPrecision(1) != 1.0) {
      double width = figure.image!.width.toDouble() * figure.scale;
      double height = figure.image!.height.toDouble() * figure.scale;
      double x = figure.posx
          - (width - figure.image!.width.toDouble()) / 2.0;
      double y = figure.posy
          - (height - figure.image!.height.toDouble()) / 2.0;
      return Rect.fromLTWH(x, y, width, height);
    }
    return rect;
  }

  Rect _getCircumscribedRect(Rect rect, double radian) {
    assert(rect.width != 0.0);
    assert(rect.height != 0.0);
    double h = sqrt(pow(rect.width / 2, 2) + pow(rect.height / 2, 2));
    double ra = asin(rect.height / 2 / h);
    double dx1 = h * cos(ra + radian % pi).abs();
    double dy1 = h * sin(ra + radian % pi).abs();
    double dx2 = h * cos(ra - radian % pi).abs();
    double dy2 = h * sin(ra - radian % pi).abs();
    double width = max(dx1, dx2) * 2;
    double height = max(dy1, dy2) * 2;
    return Rect.fromLTWH(
        rect.left - (width - rect.width) / 2,
        rect.top - (height - rect.height) / 2,
        width, height);
  }

  Rect _getInScreen(Rect rect, Size size) {
    if (rect.left < 0.0) {
      rect = rect.shift(Offset(-rect.left, 0));
    }
    if (rect.top < 0.0) {
      rect = rect.shift(Offset(0, -rect.top));
    }
    if (rect.right > size.width) {
      rect = rect.shift(Offset(size.width - rect.right, 0));
    }
    return rect;
  }
}

class FigureTrail {
  late List<Figure> _figures;
  late Function _onImageUpListener;
  bool _selectMode = false;
  int _target = -1;
  Offset _startPosition = const Offset(0, 0);
  double _startScale = 0.0;
  double _startRadian = 0.0;
  _Trim? _trim;

  FigureTrail(List<Figure>? figures) {
    if (figures != null) {
      _figures = figures;
      //print('DBG>figure>init length=${_figures.length}');
      for (var figure in _figures) {
        uiImageFromFile(figure.path)   //, const Size(400, 800))
            .then((image) {
              figure.image = image;
              _onImageUpListener.call();
              //print('DBG>figure>init pos=${figure.posx},${figure.posy} scale=${figure.scale} radian=${figure.radian}');
        });
      }
    } else {
      _figures = <Figure>[];
    }
  }

  void setImageUpListener(Function onImageUpListener) {
    _onImageUpListener = onImageUpListener;
  }

  List<Figure> get figures => _figures;
  void set figures(value) => _figures = value;

  int get target => _target;
  set target(value) => _target = value;

  int get length => _figures.length;
  Figure? _getFigure(int index) {
    if (index > length) return null;
    return _figures[index];
  }
  void _setFigure(int index, Figure figure) {
    if (index > length) return;
    _figures[index] = figure;
  }

  bool get selectMode => _selectMode;
  void setSelectMode(bool value) {
    for (var figure in _figures) {
      figure.selected = false;
    }
    _selectMode = false;
  }
  void toggleSelect(int index) {
    if (index < 0 || index >= _figures.length) return;
    print('DBG>figure>toggleSelect $index');
    _figures[index].selected ^= true;
    if (_figures[index].selected) {
      _selectMode = true;
    } else {
      _selectMode = false;
      for (var figure in _figures) {
        _selectMode |= figure.selected;
      }
    }
  }

  void remove() {
    if (!selectMode)  return;
    _figures.removeWhere((element) => element.selected == true);
  }

  void undo() { _figures.removeLast(); }
  void clear() { _figures.clear(); }
  void add(Figure figure) {
    _figures.add(figure);
    print('DBG>FigureTrail>add() length = $length');
  }

  // FigureTrail
  void draw(Canvas canvas, Size size, bool front) {
    if (_figures.isNotEmpty) {
      int index = 0;
      for (var figure in _figures) {
        if (figure.image != null && (
            (front && index == target) || (!front && index != target))) {
          _Trim(figure: figure).draw(canvas, size, selectMode);
        }
        ++ index;
      }
    }
  }

  void startTrim(Offset offset) {
    Figure? figure;
    if (target < 0 || (figure = _getFigure(target)) == null) return;
    figure!.trimming = true;
    _trim = _Trim(figure: figure);

    _startPosition = offset;
    _startScale = _trim!.figure.scale;
    _startRadian = _trim!.figure.radian;
  }

  bool trimming(Offset offset, double scale, double radian, Size size) {
    if (_trim == null)  return false;

    bool update = false;
    // drag
    Offset org = Offset(_trim!.tilt.left, _trim!.tilt.top);
    Offset pos = org + offset - _startPosition;
    if (pos.dx > 0.0 && pos.dx + _trim!.tilt.width <= size.width
        && pos.dy > 0.0) {
      if (org != pos) {
        Offset current = Offset(_trim!.figure.posx, _trim!.figure.posy);
        current += (pos - org);
        _trim!.figure.posx = current.dx;
        _trim!.figure.posy = current.dy;
        update = true;
      }
      _startPosition = offset;
    }
    //scale
    double real = (_startScale * scale)
        .clamp(Environs.minScale, Environs.maxScale);
    if (_trim!.figure.scale != real) {
      _trim!.figure.scale = real;
      update = true;
    }
    // rotate
    real = _startRadian + radian;
    if (_trim!.figure.radian != real) {
      _trim!.figure.radian = real;
      update = true;
    }
    return update;
  }

  void endTrim() {
    if (_trim == null) return;
    _trim!.figure.trimming = false;
    _setFigure(target, _trim!.figure);
  }

  int isPressOn(Offset offset) {
    int index = 0;
    for (var figure in _figures) {
      _Trim trim = _Trim(figure: figure);
      if (trim.tilt.contains(offset)) {
        print('DBG>figure>pressOn $index at ${_figures.length}');
        return index;
      }
      ++ index;
    }
    return -1;
  }

  Future<ui.Image> uiImageFromFile(String path) async {
    await Permission.storage.request();

    File file = File(path);
    Uint8List list = await file.readAsBytes();
    final ui.ImmutableBuffer buffer =
    await ui.ImmutableBuffer.fromUint8List(list);
    final ui.ImageDescriptor descriptor =
    await ui.ImageDescriptor.encoded(buffer);
    //double targetWidth = descriptor.width.toDouble();
    //double targetHeight = descriptor.height.toDouble();
    //print('DBG>figure>org size width=$targetWidth height=$targetHeight');
    //if (targetWidth > size.width.toInt()
    //    || targetHeight > size.height.toInt()) {
    //  double rate = min(size.width / targetWidth, size.height / targetHeight);
    //  targetWidth *= rate;
    //  targetHeight *= rate;
    //  print('DBG>figure>view size width=${size.width} height=${size.height}');
    //  print('DBG>figure>org size width=$targetWidth height=$targetHeight rate=$rate');
   // }
    buffer.dispose();
    ui.Codec codec =
      await descriptor.instantiateCodec();
        //targetWidth: targetWidth.toInt(),
        //targetHeight: targetHeight.toInt());
    ui.FrameInfo frame = await codec.getNextFrame();
    return frame.image;
  }
}
