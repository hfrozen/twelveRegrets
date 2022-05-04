import 'package:flutter/material.dart';
import 'package:flutter/services.dart';
import 'package:get/get.dart';
import 'package:papy/refer/translates.dart';
import 'package:papy/pages/summaries.dart';

class HomePage extends StatefulWidget {
  const HomePage({Key? key}) : super(key: key);

  @override
  _HomePageState createState() => _HomePageState();
}

class _HomePageState extends State<HomePage> {
  final bool _debug = false;
  var _iconView = 0.0;
  bool _forcedEmpty = false;

  @override
  void initState() {
    super.initState();
    Future.delayed(const Duration(milliseconds: 100)).then((_) => {
      setState(() {
        _iconView = 1.0;
      })
    });
  }

  @override
  Widget build(BuildContext context) {
    SystemChrome.setApplicationSwitcherDescription(
        ApplicationSwitcherDescription(
          label: 'Papyrus',
          primaryColor: Theme.of(context).primaryColor.value,
        )
    );

    return Scaffold(
      backgroundColor: Colors.white,
      appBar: _debug
      ? AppBar(
        title: Text(Localization.of(context)!.tr('notepad')!),
        actions: [
          IconButton(
            icon: const Icon(Icons.search),
            onPressed: () {  },
          ),
        ],
      )
      : null,
      body: Stack(
        children: [
          Center(
            child: Column(
              mainAxisAlignment: MainAxisAlignment.center,
              crossAxisAlignment: CrossAxisAlignment.center,
              children: [
                const Image(
                  image: AssetImage('assets/papyrus.png'),
                  fit: BoxFit.fill,
                  width: 225,
                  height: 225,
                ),
                Visibility(
                  visible: _debug,
                  child: TextButton(
                    child: _forcedEmpty
                        ? const Text('from new') : const Text('from list'),
                    onPressed: () {
                      setState(() {
                        _forcedEmpty ^= true;
                      });
                    },
                  ),
                ),
              ]
            ),
          ),
          if (!_debug)
          AnimatedOpacity(
            duration: const Duration(milliseconds: 500),
            opacity: _iconView,
            onEnd: () => {
              Get.offAll(() => const SummariesPage(forcedEmpty: false))
            },
            child: Column(
              children: [
                AppBar(
                  title: Text(Localization.of(context)!.tr('notepad')!),
                  actions: [
                    IconButton(
                      icon: const Icon(Icons.search),
                      onPressed: () {},
                    )
                  ],
                ),
                Expanded(
                  child: Container(
                    height: 100,
                    color: Colors.white,
                  ),
                ),
              ],
            ),
          )
        ]
      ),
      floatingActionButton: _debug
        ? FloatingActionButton(
        onPressed: () => {
          //Get.offAll(() => SummariesPage())
          Get.to(() => SummariesPage(forcedEmpty: _forcedEmpty))
        },
        child: const Icon(Icons.chevron_right),
        tooltip: _forcedEmpty ? 'list' : 'new',
      )
      : null,
    );
  }
}