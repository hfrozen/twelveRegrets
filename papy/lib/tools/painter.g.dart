// GENERATED CODE - DO NOT MODIFY BY HAND

part of 'painter.dart';

// **************************************************************************
// JsonSerializableGenerator
// **************************************************************************

Point _$PointFromJson(Map<String, dynamic> json) => Point(
      (json['x'] as num).toDouble(),
      (json['y'] as num).toDouble(),
    );

Map<String, dynamic> _$PointToJson(Point instance) => <String, dynamic>{
      'x': instance.x,
      'y': instance.y,
    };

PathTrackEntry _$PathTrackEntryFromJson(Map<String, dynamic> json) =>
    PathTrackEntry(
      (json['x'] as num).toDouble(),
      (json['y'] as num).toDouble(),
      const ColorSerializer().fromJson(json['color'] as int),
      json['paintBlendMode'] as int,
      (json['paintThickness'] as num).toDouble(),
    )..lineToList = (json['lineToList'] as List<dynamic>)
        .map((e) => Point.fromJson(e as Map<String, dynamic>))
        .toList();

Map<String, dynamic> _$PathTrackEntryToJson(PathTrackEntry instance) =>
    <String, dynamic>{
      'x': instance.x,
      'y': instance.y,
      'lineToList': instance.lineToList,
      'color': const ColorSerializer().toJson(instance.color),
      'paintBlendMode': instance.paintBlendMode,
      'paintThickness': instance.paintThickness,
    };
