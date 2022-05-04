import 'package:flutter/widgets.dart';
import 'package:json_annotation/json_annotation.dart';
import 'dart:convert';
import 'letter.dart';

part 'how.g.dart';

class ColorSerializer implements JsonConverter<Color, int> {
  const ColorSerializer();

  @override
  Color fromJson(int json) => Color(json);

  @override
  int toJson(Color color) => color.value;
}

@JsonSerializable(nullable: false)
class Hows {
  @ColorSerializer()
  Color foregroundColor;
  @ColorSerializer()
  Color backgroundColor;
  double thickness;

  Hows(this.foregroundColor, this.backgroundColor, this.thickness);

  factory Hows.fromJson(Map<String, dynamic> json) =>
      _$HowsFromJson(json);
  Map<String, dynamic> toJson() => _$HowsToJson(this);
}

class HowsConvertor {
  static Hows fromLetter(Letter letter) {
    return Hows.fromJson(jsonDecode(letter.how));
  }
}
