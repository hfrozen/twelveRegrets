import 'package:flutter/material.dart';

class Environs {
  List<Color> backgroundColors = [
    Colors.redAccent.withAlpha(paintColorAlpha),
    Colors.orange.withAlpha(paintColorAlpha),
    Colors.yellowAccent.withAlpha(paintColorAlpha),
    Colors.lightGreenAccent.withAlpha(paintColorAlpha),
    Colors.lightBlueAccent.withAlpha(paintColorAlpha),
    Colors.indigoAccent.withAlpha(paintColorAlpha),
    Colors.purpleAccent.withAlpha(paintColorAlpha),
    Colors.white.withAlpha(paintColorAlpha),
  ];

  List<Color> foregroundColors = [
    Colors.redAccent,
    Colors.orange,
    Colors.yellowAccent,
    Colors.lightGreenAccent,
    Colors.lightBlueAccent,
    Colors.indigoAccent,
    Colors.purpleAccent,
    Colors.black,
  ];

  static const paintColorAlpha = 127;
  static var defaultColor = Colors.blue;
  //static const Color foregroundColor = Colors.black;
  static const Color defaultBackgroundColor = Colors.grey;
  static const double maxThickness = 14.0;
  static const double minThickness = 2.0;
  static const double defaultThickness = 2.0;
  static const double maxVelocity = 30.0;
  static const double minVelocity = 1.5;
  static const int pathCompressionLevel = 8;
  static const double maxScale = 2.0;
  static const double minScale = 0.1;
  static const int columnViews = 4;
  static const int rowViews = 2;
  static const double defaultPaintHeight = 1920;
}

const eToolbarHeight = 46.0;
enum EditMode { text, draw, picture, none }
enum EditSubMenu { share, delete, close }
