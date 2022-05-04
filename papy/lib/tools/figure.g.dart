// GENERATED CODE - DO NOT MODIFY BY HAND

part of 'figure.dart';

// **************************************************************************
// JsonSerializableGenerator
// **************************************************************************

Preview _$PreviewFromJson(Map<String, dynamic> json) => Preview(
      json['data'] as String,
    );

Map<String, dynamic> _$PreviewToJson(Preview instance) => <String, dynamic>{
      'data': instance.data,
    };

Figure _$FigureFromJson(Map<String, dynamic> json) => Figure(
      json['path'] as String,
      (json['posx'] as num).toDouble(),
      (json['posy'] as num).toDouble(),
      (json['scale'] as num).toDouble(),
      (json['radian'] as num).toDouble(),
    );

Map<String, dynamic> _$FigureToJson(Figure instance) => <String, dynamic>{
      'path': instance.path,
      'posx': instance.posx,
      'posy': instance.posy,
      'scale': instance.scale,
      'radian': instance.radian,
    };
