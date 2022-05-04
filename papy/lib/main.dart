import 'package:flutter/material.dart';
import 'package:flutter/services.dart';
import 'package:flutter_localizations/flutter_localizations.dart';
import 'package:get/get.dart';
import 'package:provider/provider.dart';
import 'package:google_fonts/google_fonts.dart';

import 'package:papy/base/letter.dart';
import 'package:papy/refer/translates.dart';
import 'package:papy/refer/const.dart';
import 'package:papy/pages/home.dart';

void main() {
  runApp(const MyApp());
}

class MyApp extends StatelessWidget {
  const MyApp({Key? key}) : super(key: key);

  @override
  Widget build(BuildContext context) {
    SystemChrome.setPreferredOrientations([
      DeviceOrientation.portraitUp,
      DeviceOrientation.portraitDown,
    ]);

    return Provider(
      create: (_) => LetterDatabase(),
      child: GetMaterialApp(
        supportedLocales: const [
          Locale('ko'),
          Locale('en', 'US'),
          Locale('es', '429'),
          Locale('fr'),
          Locale('de'),
          Locale('it'),
          Locale('ja'),
          Locale('zh'),
        ],
        localizationsDelegates: const [
          Localization.delegate,
          GlobalMaterialLocalizations.delegate,
          GlobalCupertinoLocalizations.delegate,
          GlobalWidgetsLocalizations.delegate,
        ],
        title: "Papyrus",
        theme: ThemeData(
          primarySwatch: Environs.defaultColor,
          visualDensity: VisualDensity.adaptivePlatformDensity,
          textTheme: GoogleFonts.poppinsTextTheme(Theme.of(context).textTheme),
        ),
        home: HomePage(),
      ),
    );
  }
}
