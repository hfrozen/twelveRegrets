// GENERATED CODE - DO NOT MODIFY BY HAND

part of 'how.dart';

// **************************************************************************
// JsonSerializableGenerator
// **************************************************************************

Hows _$HowsFromJson(Map<String, dynamic> json) => Hows(
      const ColorSerializer().fromJson(json['foregroundColor'] as int),
      const ColorSerializer().fromJson(json['backgroundColor'] as int),
      (json['thickness'] as num).toDouble(),
    );

Map<String, dynamic> _$HowsToJson(Hows instance) => <String, dynamic>{
      'foregroundColor':
          const ColorSerializer().toJson(instance.foregroundColor),
      'backgroundColor':
          const ColorSerializer().toJson(instance.backgroundColor),
      'thickness': instance.thickness,
    };
