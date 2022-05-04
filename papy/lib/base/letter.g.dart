// GENERATED CODE - DO NOT MODIFY BY HAND

part of 'letter.dart';

// **************************************************************************
// MoorGenerator
// **************************************************************************

// ignore_for_file: unnecessary_brace_in_string_interps, unnecessary_this
class Letter extends DataClass implements Insertable<Letter> {
  final int id;
  final int? drawId;
  final int view;
  final String story;
  final String plainStory;
  final DateTime date;
  final String how;
  Letter(
      {required this.id,
      this.drawId,
      required this.view,
      required this.story,
      required this.plainStory,
      required this.date,
      required this.how});
  factory Letter.fromData(Map<String, dynamic> data, {String? prefix}) {
    final effectivePrefix = prefix ?? '';
    return Letter(
      id: const IntType()
          .mapFromDatabaseResponse(data['${effectivePrefix}id'])!,
      drawId: const IntType()
          .mapFromDatabaseResponse(data['${effectivePrefix}draw_id']),
      view: const IntType()
          .mapFromDatabaseResponse(data['${effectivePrefix}view'])!,
      story: const StringType()
          .mapFromDatabaseResponse(data['${effectivePrefix}story'])!,
      plainStory: const StringType()
          .mapFromDatabaseResponse(data['${effectivePrefix}plain_story'])!,
      date: const DateTimeType()
          .mapFromDatabaseResponse(data['${effectivePrefix}date'])!,
      how: const StringType()
          .mapFromDatabaseResponse(data['${effectivePrefix}how'])!,
    );
  }
  @override
  Map<String, Expression> toColumns(bool nullToAbsent) {
    final map = <String, Expression>{};
    map['id'] = Variable<int>(id);
    if (!nullToAbsent || drawId != null) {
      map['draw_id'] = Variable<int?>(drawId);
    }
    map['view'] = Variable<int>(view);
    map['story'] = Variable<String>(story);
    map['plain_story'] = Variable<String>(plainStory);
    map['date'] = Variable<DateTime>(date);
    map['how'] = Variable<String>(how);
    return map;
  }

  LettersCompanion toCompanion(bool nullToAbsent) {
    return LettersCompanion(
      id: Value(id),
      drawId:
          drawId == null && nullToAbsent ? const Value.absent() : Value(drawId),
      view: Value(view),
      story: Value(story),
      plainStory: Value(plainStory),
      date: Value(date),
      how: Value(how),
    );
  }

  factory Letter.fromJson(Map<String, dynamic> json,
      {ValueSerializer? serializer}) {
    serializer ??= driftRuntimeOptions.defaultSerializer;
    return Letter(
      id: serializer.fromJson<int>(json['id']),
      drawId: serializer.fromJson<int?>(json['drawId']),
      view: serializer.fromJson<int>(json['view']),
      story: serializer.fromJson<String>(json['story']),
      plainStory: serializer.fromJson<String>(json['plainStory']),
      date: serializer.fromJson<DateTime>(json['date']),
      how: serializer.fromJson<String>(json['how']),
    );
  }
  @override
  Map<String, dynamic> toJson({ValueSerializer? serializer}) {
    serializer ??= driftRuntimeOptions.defaultSerializer;
    return <String, dynamic>{
      'id': serializer.toJson<int>(id),
      'drawId': serializer.toJson<int?>(drawId),
      'view': serializer.toJson<int>(view),
      'story': serializer.toJson<String>(story),
      'plainStory': serializer.toJson<String>(plainStory),
      'date': serializer.toJson<DateTime>(date),
      'how': serializer.toJson<String>(how),
    };
  }

  Letter copyWith(
          {int? id,
          int? drawId,
          int? view,
          String? story,
          String? plainStory,
          DateTime? date,
          String? how}) =>
      Letter(
        id: id ?? this.id,
        drawId: drawId ?? this.drawId,
        view: view ?? this.view,
        story: story ?? this.story,
        plainStory: plainStory ?? this.plainStory,
        date: date ?? this.date,
        how: how ?? this.how,
      );
  @override
  String toString() {
    return (StringBuffer('Letter(')
          ..write('id: $id, ')
          ..write('drawId: $drawId, ')
          ..write('view: $view, ')
          ..write('story: $story, ')
          ..write('plainStory: $plainStory, ')
          ..write('date: $date, ')
          ..write('how: $how')
          ..write(')'))
        .toString();
  }

  @override
  int get hashCode =>
      Object.hash(id, drawId, view, story, plainStory, date, how);
  @override
  bool operator ==(Object other) =>
      identical(this, other) ||
      (other is Letter &&
          other.id == this.id &&
          other.drawId == this.drawId &&
          other.view == this.view &&
          other.story == this.story &&
          other.plainStory == this.plainStory &&
          other.date == this.date &&
          other.how == this.how);
}

class LettersCompanion extends UpdateCompanion<Letter> {
  final Value<int> id;
  final Value<int?> drawId;
  final Value<int> view;
  final Value<String> story;
  final Value<String> plainStory;
  final Value<DateTime> date;
  final Value<String> how;
  const LettersCompanion({
    this.id = const Value.absent(),
    this.drawId = const Value.absent(),
    this.view = const Value.absent(),
    this.story = const Value.absent(),
    this.plainStory = const Value.absent(),
    this.date = const Value.absent(),
    this.how = const Value.absent(),
  });
  LettersCompanion.insert({
    this.id = const Value.absent(),
    this.drawId = const Value.absent(),
    required int view,
    required String story,
    required String plainStory,
    required DateTime date,
    required String how,
  })  : view = Value(view),
        story = Value(story),
        plainStory = Value(plainStory),
        date = Value(date),
        how = Value(how);
  static Insertable<Letter> custom({
    Expression<int>? id,
    Expression<int?>? drawId,
    Expression<int>? view,
    Expression<String>? story,
    Expression<String>? plainStory,
    Expression<DateTime>? date,
    Expression<String>? how,
  }) {
    return RawValuesInsertable({
      if (id != null) 'id': id,
      if (drawId != null) 'draw_id': drawId,
      if (view != null) 'view': view,
      if (story != null) 'story': story,
      if (plainStory != null) 'plain_story': plainStory,
      if (date != null) 'date': date,
      if (how != null) 'how': how,
    });
  }

  LettersCompanion copyWith(
      {Value<int>? id,
      Value<int?>? drawId,
      Value<int>? view,
      Value<String>? story,
      Value<String>? plainStory,
      Value<DateTime>? date,
      Value<String>? how}) {
    return LettersCompanion(
      id: id ?? this.id,
      drawId: drawId ?? this.drawId,
      view: view ?? this.view,
      story: story ?? this.story,
      plainStory: plainStory ?? this.plainStory,
      date: date ?? this.date,
      how: how ?? this.how,
    );
  }

  @override
  Map<String, Expression> toColumns(bool nullToAbsent) {
    final map = <String, Expression>{};
    if (id.present) {
      map['id'] = Variable<int>(id.value);
    }
    if (drawId.present) {
      map['draw_id'] = Variable<int?>(drawId.value);
    }
    if (view.present) {
      map['view'] = Variable<int>(view.value);
    }
    if (story.present) {
      map['story'] = Variable<String>(story.value);
    }
    if (plainStory.present) {
      map['plain_story'] = Variable<String>(plainStory.value);
    }
    if (date.present) {
      map['date'] = Variable<DateTime>(date.value);
    }
    if (how.present) {
      map['how'] = Variable<String>(how.value);
    }
    return map;
  }

  @override
  String toString() {
    return (StringBuffer('LettersCompanion(')
          ..write('id: $id, ')
          ..write('drawId: $drawId, ')
          ..write('view: $view, ')
          ..write('story: $story, ')
          ..write('plainStory: $plainStory, ')
          ..write('date: $date, ')
          ..write('how: $how')
          ..write(')'))
        .toString();
  }
}

class $LettersTable extends Letters with TableInfo<$LettersTable, Letter> {
  @override
  final GeneratedDatabase attachedDatabase;
  final String? _alias;
  $LettersTable(this.attachedDatabase, [this._alias]);
  final VerificationMeta _idMeta = const VerificationMeta('id');
  @override
  late final GeneratedColumn<int?> id = GeneratedColumn<int?>(
      'id', aliasedName, false,
      type: const IntType(),
      requiredDuringInsert: false,
      defaultConstraints: 'PRIMARY KEY AUTOINCREMENT');
  final VerificationMeta _drawIdMeta = const VerificationMeta('drawId');
  @override
  late final GeneratedColumn<int?> drawId = GeneratedColumn<int?>(
      'draw_id', aliasedName, true,
      type: const IntType(), requiredDuringInsert: false);
  final VerificationMeta _viewMeta = const VerificationMeta('view');
  @override
  late final GeneratedColumn<int?> view = GeneratedColumn<int?>(
      'view', aliasedName, false,
      type: const IntType(), requiredDuringInsert: true);
  final VerificationMeta _storyMeta = const VerificationMeta('story');
  @override
  late final GeneratedColumn<String?> story = GeneratedColumn<String?>(
      'story', aliasedName, false,
      type: const StringType(), requiredDuringInsert: true);
  final VerificationMeta _plainStoryMeta = const VerificationMeta('plainStory');
  @override
  late final GeneratedColumn<String?> plainStory = GeneratedColumn<String?>(
      'plain_story', aliasedName, false,
      type: const StringType(), requiredDuringInsert: true);
  final VerificationMeta _dateMeta = const VerificationMeta('date');
  @override
  late final GeneratedColumn<DateTime?> date = GeneratedColumn<DateTime?>(
      'date', aliasedName, false,
      type: const IntType(), requiredDuringInsert: true);
  final VerificationMeta _howMeta = const VerificationMeta('how');
  @override
  late final GeneratedColumn<String?> how = GeneratedColumn<String?>(
      'how', aliasedName, false,
      type: const StringType(), requiredDuringInsert: true);
  @override
  List<GeneratedColumn> get $columns =>
      [id, drawId, view, story, plainStory, date, how];
  @override
  String get aliasedName => _alias ?? 'letters';
  @override
  String get actualTableName => 'letters';
  @override
  VerificationContext validateIntegrity(Insertable<Letter> instance,
      {bool isInserting = false}) {
    final context = VerificationContext();
    final data = instance.toColumns(true);
    if (data.containsKey('id')) {
      context.handle(_idMeta, id.isAcceptableOrUnknown(data['id']!, _idMeta));
    }
    if (data.containsKey('draw_id')) {
      context.handle(_drawIdMeta,
          drawId.isAcceptableOrUnknown(data['draw_id']!, _drawIdMeta));
    }
    if (data.containsKey('view')) {
      context.handle(
          _viewMeta, view.isAcceptableOrUnknown(data['view']!, _viewMeta));
    } else if (isInserting) {
      context.missing(_viewMeta);
    }
    if (data.containsKey('story')) {
      context.handle(
          _storyMeta, story.isAcceptableOrUnknown(data['story']!, _storyMeta));
    } else if (isInserting) {
      context.missing(_storyMeta);
    }
    if (data.containsKey('plain_story')) {
      context.handle(
          _plainStoryMeta,
          plainStory.isAcceptableOrUnknown(
              data['plain_story']!, _plainStoryMeta));
    } else if (isInserting) {
      context.missing(_plainStoryMeta);
    }
    if (data.containsKey('date')) {
      context.handle(
          _dateMeta, date.isAcceptableOrUnknown(data['date']!, _dateMeta));
    } else if (isInserting) {
      context.missing(_dateMeta);
    }
    if (data.containsKey('how')) {
      context.handle(
          _howMeta, how.isAcceptableOrUnknown(data['how']!, _howMeta));
    } else if (isInserting) {
      context.missing(_howMeta);
    }
    return context;
  }

  @override
  Set<GeneratedColumn> get $primaryKey => {id};
  @override
  Letter map(Map<String, dynamic> data, {String? tablePrefix}) {
    return Letter.fromData(data,
        prefix: tablePrefix != null ? '$tablePrefix.' : null);
  }

  @override
  $LettersTable createAlias(String alias) {
    return $LettersTable(attachedDatabase, alias);
  }
}

class Drawing extends DataClass implements Insertable<Drawing> {
  final int id;
  final String? paths;
  final String? figures;
  final String? preview;
  Drawing({required this.id, this.paths, this.figures, this.preview});
  factory Drawing.fromData(Map<String, dynamic> data, {String? prefix}) {
    final effectivePrefix = prefix ?? '';
    return Drawing(
      id: const IntType()
          .mapFromDatabaseResponse(data['${effectivePrefix}id'])!,
      paths: const StringType()
          .mapFromDatabaseResponse(data['${effectivePrefix}paths']),
      figures: const StringType()
          .mapFromDatabaseResponse(data['${effectivePrefix}figures']),
      preview: const StringType()
          .mapFromDatabaseResponse(data['${effectivePrefix}preview']),
    );
  }
  @override
  Map<String, Expression> toColumns(bool nullToAbsent) {
    final map = <String, Expression>{};
    map['id'] = Variable<int>(id);
    if (!nullToAbsent || paths != null) {
      map['paths'] = Variable<String?>(paths);
    }
    if (!nullToAbsent || figures != null) {
      map['figures'] = Variable<String?>(figures);
    }
    if (!nullToAbsent || preview != null) {
      map['preview'] = Variable<String?>(preview);
    }
    return map;
  }

  DrawingsCompanion toCompanion(bool nullToAbsent) {
    return DrawingsCompanion(
      id: Value(id),
      paths:
          paths == null && nullToAbsent ? const Value.absent() : Value(paths),
      figures: figures == null && nullToAbsent
          ? const Value.absent()
          : Value(figures),
      preview: preview == null && nullToAbsent
          ? const Value.absent()
          : Value(preview),
    );
  }

  factory Drawing.fromJson(Map<String, dynamic> json,
      {ValueSerializer? serializer}) {
    serializer ??= driftRuntimeOptions.defaultSerializer;
    return Drawing(
      id: serializer.fromJson<int>(json['id']),
      paths: serializer.fromJson<String?>(json['paths']),
      figures: serializer.fromJson<String?>(json['figures']),
      preview: serializer.fromJson<String?>(json['preview']),
    );
  }
  @override
  Map<String, dynamic> toJson({ValueSerializer? serializer}) {
    serializer ??= driftRuntimeOptions.defaultSerializer;
    return <String, dynamic>{
      'id': serializer.toJson<int>(id),
      'paths': serializer.toJson<String?>(paths),
      'figures': serializer.toJson<String?>(figures),
      'preview': serializer.toJson<String?>(preview),
    };
  }

  Drawing copyWith(
          {int? id, String? paths, String? figures, String? preview}) =>
      Drawing(
        id: id ?? this.id,
        paths: paths ?? this.paths,
        figures: figures ?? this.figures,
        preview: preview ?? this.preview,
      );
  @override
  String toString() {
    return (StringBuffer('Drawing(')
          ..write('id: $id, ')
          ..write('paths: $paths, ')
          ..write('figures: $figures, ')
          ..write('preview: $preview')
          ..write(')'))
        .toString();
  }

  @override
  int get hashCode => Object.hash(id, paths, figures, preview);
  @override
  bool operator ==(Object other) =>
      identical(this, other) ||
      (other is Drawing &&
          other.id == this.id &&
          other.paths == this.paths &&
          other.figures == this.figures &&
          other.preview == this.preview);
}

class DrawingsCompanion extends UpdateCompanion<Drawing> {
  final Value<int> id;
  final Value<String?> paths;
  final Value<String?> figures;
  final Value<String?> preview;
  const DrawingsCompanion({
    this.id = const Value.absent(),
    this.paths = const Value.absent(),
    this.figures = const Value.absent(),
    this.preview = const Value.absent(),
  });
  DrawingsCompanion.insert({
    this.id = const Value.absent(),
    this.paths = const Value.absent(),
    this.figures = const Value.absent(),
    this.preview = const Value.absent(),
  });
  static Insertable<Drawing> custom({
    Expression<int>? id,
    Expression<String?>? paths,
    Expression<String?>? figures,
    Expression<String?>? preview,
  }) {
    return RawValuesInsertable({
      if (id != null) 'id': id,
      if (paths != null) 'paths': paths,
      if (figures != null) 'figures': figures,
      if (preview != null) 'preview': preview,
    });
  }

  DrawingsCompanion copyWith(
      {Value<int>? id,
      Value<String?>? paths,
      Value<String?>? figures,
      Value<String?>? preview}) {
    return DrawingsCompanion(
      id: id ?? this.id,
      paths: paths ?? this.paths,
      figures: figures ?? this.figures,
      preview: preview ?? this.preview,
    );
  }

  @override
  Map<String, Expression> toColumns(bool nullToAbsent) {
    final map = <String, Expression>{};
    if (id.present) {
      map['id'] = Variable<int>(id.value);
    }
    if (paths.present) {
      map['paths'] = Variable<String?>(paths.value);
    }
    if (figures.present) {
      map['figures'] = Variable<String?>(figures.value);
    }
    if (preview.present) {
      map['preview'] = Variable<String?>(preview.value);
    }
    return map;
  }

  @override
  String toString() {
    return (StringBuffer('DrawingsCompanion(')
          ..write('id: $id, ')
          ..write('paths: $paths, ')
          ..write('figures: $figures, ')
          ..write('preview: $preview')
          ..write(')'))
        .toString();
  }
}

class $DrawingsTable extends Drawings with TableInfo<$DrawingsTable, Drawing> {
  @override
  final GeneratedDatabase attachedDatabase;
  final String? _alias;
  $DrawingsTable(this.attachedDatabase, [this._alias]);
  final VerificationMeta _idMeta = const VerificationMeta('id');
  @override
  late final GeneratedColumn<int?> id = GeneratedColumn<int?>(
      'id', aliasedName, false,
      type: const IntType(),
      requiredDuringInsert: false,
      defaultConstraints: 'PRIMARY KEY AUTOINCREMENT');
  final VerificationMeta _pathsMeta = const VerificationMeta('paths');
  @override
  late final GeneratedColumn<String?> paths = GeneratedColumn<String?>(
      'paths', aliasedName, true,
      type: const StringType(), requiredDuringInsert: false);
  final VerificationMeta _figuresMeta = const VerificationMeta('figures');
  @override
  late final GeneratedColumn<String?> figures = GeneratedColumn<String?>(
      'figures', aliasedName, true,
      type: const StringType(), requiredDuringInsert: false);
  final VerificationMeta _previewMeta = const VerificationMeta('preview');
  @override
  late final GeneratedColumn<String?> preview = GeneratedColumn<String?>(
      'preview', aliasedName, true,
      type: const StringType(), requiredDuringInsert: false);
  @override
  List<GeneratedColumn> get $columns => [id, paths, figures, preview];
  @override
  String get aliasedName => _alias ?? 'drawings';
  @override
  String get actualTableName => 'drawings';
  @override
  VerificationContext validateIntegrity(Insertable<Drawing> instance,
      {bool isInserting = false}) {
    final context = VerificationContext();
    final data = instance.toColumns(true);
    if (data.containsKey('id')) {
      context.handle(_idMeta, id.isAcceptableOrUnknown(data['id']!, _idMeta));
    }
    if (data.containsKey('paths')) {
      context.handle(
          _pathsMeta, paths.isAcceptableOrUnknown(data['paths']!, _pathsMeta));
    }
    if (data.containsKey('figures')) {
      context.handle(_figuresMeta,
          figures.isAcceptableOrUnknown(data['figures']!, _figuresMeta));
    }
    if (data.containsKey('preview')) {
      context.handle(_previewMeta,
          preview.isAcceptableOrUnknown(data['preview']!, _previewMeta));
    }
    return context;
  }

  @override
  Set<GeneratedColumn> get $primaryKey => {id};
  @override
  Drawing map(Map<String, dynamic> data, {String? tablePrefix}) {
    return Drawing.fromData(data,
        prefix: tablePrefix != null ? '$tablePrefix.' : null);
  }

  @override
  $DrawingsTable createAlias(String alias) {
    return $DrawingsTable(attachedDatabase, alias);
  }
}

abstract class _$LetterDatabase extends GeneratedDatabase {
  _$LetterDatabase(QueryExecutor e) : super(SqlTypeSystem.defaultInstance, e);
  late final $LettersTable letters = $LettersTable(this);
  late final $DrawingsTable drawings = $DrawingsTable(this);
  @override
  Iterable<TableInfo> get allTables => allSchemaEntities.whereType<TableInfo>();
  @override
  List<DatabaseSchemaEntity> get allSchemaEntities => [letters, drawings];
}
